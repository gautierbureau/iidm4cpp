# iidm-bridge-cpp

> A C++ bridge library providing object-oriented getter/setter access to
> PowSyBl IIDM network components, delegating all heavy lifting to the Java
> implementation via either a **GraalVM native image** (standalone mode) or
> **direct JNI** (embedded mode), while exposing a single, unified API to its
> consumers (e.g. Dynawo).

---

## Table of Contents

1. [Motivation & Context](#1-motivation--context)
2. [Design Goals and Non-Goals](#2-design-goals-and-non-goals)
3. [High-Level Architecture](#3-high-level-architecture)
4. [C++ Public API Design](#4-c-public-api-design)
   - 4.1 [Network Components](#41-network-components)
   - 4.2 [Handle Model](#42-handle-model)
   - 4.3 [Network & NetworkFactory](#43-network--networkfactory)
   - 4.4 [Value Types and Enumerations](#44-value-types-and-enumerations)
5. [Backend Abstraction Layer](#5-backend-abstraction-layer)
   - 5.1 [BackendProvider Interface](#51-backendprovider-interface)
   - 5.2 [GraalVM Native Image Backend](#52-graalvm-native-image-backend)
   - 5.3 [JNI Embedded Backend](#53-jni-embedded-backend)
   - 5.4 [Extensions](#54-extensions)
   - 5.5 [Backend Selection Strategy](#55-backend-selection-strategy)
6. [Java-Side Design](#6-java-side-design)
   - 6.1 [GraalVM Entry Points](#61-graalvm-entry-points)
   - 6.2 [JNI Registry for Embedded Mode](#62-jni-registry-for-embedded-mode)
7. [Lifecycle and Memory Management](#7-lifecycle-and-memory-management)
8. [Thread Safety](#8-thread-safety)
9. [Error Handling](#9-error-handling)
10. [Directory Layout](#10-directory-layout)
11. [Build System (CMake)](#11-build-system-cmake)
12. [Integration with Dynawo](#12-integration-with-dynawo)
13. [Testing Strategy](#13-testing-strategy)
14. [Comparison with libiidm4cpp and pypowsybl](#14-comparison-with-libiidm4cpp-and-pypowsybl)
15. [Roadmap](#15-roadmap)

---

## 1. Motivation & Context

### The problem with libiidm4cpp

[`powsybl-iidm4cpp`](https://github.com/powsybl/powsybl-iidm4cpp) is a
full reimplementation of the PowSyBl IIDM data model in C++. Maintaining
feature parity with the reference Java implementation
([`powsybl-core/iidm`](https://github.com/powsybl/powsybl-core/tree/main/iidm))
is a continuous, expensive effort:

- Every new IIDM feature or extension must be ported twice.
- Bugs fixed in Java silently persist in C++.
- Serialization formats (XIIDM, CGMES) must also be duplicated.
- Dynawo consumes only a **small subset** of the full model: it reads
  topology, calls a handful of getters to inspect initial conditions, and
  calls setters during simulation to update operating points. There is no
  need to reproduce the full object graph.

### The opportunity

GraalVM Native Image can compile a Java library into a **standalone shared
library** (`.so` / `.dylib` / `.dll`) that exposes a plain C API callable
from any native language. This is exactly the mechanism used by
[pypowsybl](https://github.com/powsybl/pypowsybl) to give Python access to
PowSyBl without reimplementing anything.

At the same time, when Dynawo is invoked from a Java orchestration layer
(e.g. a dynamic simulation runner), the network is **already loaded in the
JVM**. In that case it is wasteful to serialize it to disk, deserialize it
again through the native image, and maintain a second copy in memory. JNI
lets the C++ code attach to the running JVM and read the objects that are
already there.

### The solution

`iidm-bridge-cpp` provides:

- A **uniform C++ API** (`Network`, `Generator`, `Load`, `Line`, …) whose
  objects are light handles to Java-side state.
- A **GraalVM backend** for the standalone case (Dynawo called from a shell
  script, no JVM in sight).
- A **JNI backend** for the embedded case (Dynawo called from a Java
  program that already owns the network object).
- A **strategy layer** that selects the right backend transparently at
  runtime, so that Dynawo code never needs to know which mode it is running
  in.

---

## 2. Design Goals and Non-Goals

### Goals

| Goal | Rationale |
|---|---|
| Expose the current IIDM API from C++: reading, changing, creating and removing objects | The C++ code uses IIDM as Java code would; deprecated powsybl APIs are not ported |
| Single C++ API surface for both modes | Dynawo code has zero `#ifdef STANDALONE` branches |
| Delegate all business logic to Java | Correctness, extensions, and future IIDM revisions are free |
| Zero-copy in embedded mode | The JVM already owns the data; the C++ layer only borrows references |
| Lightweight compile footprint | The library itself has no heavy dependencies beyond the JNI headers and (optionally) the GraalVM SDK headers |
| CMake-compatible | Drop-in replacement for the `iidm4cpp` CMake target in Dynawo |

### Non-Goals

- **A full IIDM reimplementation**. That is explicitly what we are moving
  away from: validation, defaults and id checks stay in powsybl, including
  when objects are created from C++ (the C++ adders forward to powsybl's
  adders).
- **Python or other language bindings**. This library targets C++ consumers
  only. pypowsybl covers Python.
- **Replacing the Java serialization layer**. File I/O remains Java's
  responsibility.

---

## 3. High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        Dynawo (C++ simulation core)                     │
│                                                                         │
│  iidm::Network  iidm::Generator  iidm::Load  iidm::Line  …             │
│         │               │               │          │                    │
└─────────┼───────────────┼───────────────┼──────────┼────────────────────┘
          │               │     C++ public API        │
          ▼               ▼               ▼           ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                     iidm-bridge-cpp  (this library)                     │
│                                                                         │
│  ┌──────────────────────────────────────────────────────────────────┐   │
│  │                   BackendProvider  (interface)                   │   │
│  │  getDouble(handle, property)   setDouble(handle, property, val)  │   │
│  │  getString(handle, property)   getChildren(handle, type)  …      │   │
│  └──────────────────────────────────────────────────────────────────┘   │
│           │                                        │                    │
│    ┌──────┴──────┐                        ┌────────┴──────┐             │
│    │  GraalVM    │                        │  JNI          │             │
│    │  Backend    │                        │  Backend      │             │
│    └──────┬──────┘                        └────────┬──────┘             │
└───────────┼────────────────────────────────────────┼────────────────────┘
            │  C ABI (GraalVM native image exports)  │  JNI calls
            ▼                                        ▼
┌─────────────────────────┐           ┌──────────────────────────────────┐
│  libpowsybl-iidm.so     │           │  JVM (already running)           │
│  (native-image output)  │           │                                  │
│                         │           │  com.powsybl.iidm.network.*      │
│  Full powsybl-core IIDM │           │  Network, Generator, Load, …     │
│  compiled to native     │           │  (loaded by the Java program)    │
└─────────────────────────┘           └──────────────────────────────────┘
```

### Runtime mode selection

```
                      ┌─────────────────────────────┐
                      │  NetworkFactory::load(path)  │
                      └──────────────┬──────────────┘
                                     │
              ┌──────────────────────▼──────────────────────┐
              │   Is a JVM reachable via JNI_GetCreatedJavaVMs?  │
              └──────────────┬───────────────┬──────────────┘
                         YES │               │ NO
                             ▼               ▼
                    ┌────────────┐   ┌────────────────┐
                    │ JNIBackend │   │ GraalVMBackend │
                    └────────────┘   └────────────────┘
```

The selection can also be forced explicitly via an environment variable
`IIDM_BRIDGE_MODE=graalvm|jni` or via a compile-time default.

---

## 4. C++ Public API Design

All public headers live under `include/iidm/`. The library is compiled as
a shared library (`libiidmbridge.so`).

### 4.1 Network Components

The public classes mirror the powsybl IIDM interfaces. Each object is a **thin value**: the handle
of a Java object and the backend that holds it, nothing else (§4.2). Objects are obtained from a
`Network` or from other objects; they are cheap to copy, and valid while the `Network` lives.

```
Identifiable              getId(), getNameOrId(), getOptionalName(), setName(), isFictitious(),
│                         getType(), properties, aliases, is<T>() / as<T>()
├── Network               the root (owns the backend session, §4.3)
├── Substation, VoltageLevel, Bus, Switch, HvdcLine
└── Connectable           getTerminals(), connect() / disconnect(), isEquivalent(),
    │                     unsetSolvedValues(), remove()
    ├── Injection         getTerminal()
    │   ├── Generator, Load, Battery, ShuntCompensator, StaticVarCompensator,
    │   │   DanglingLine, BusbarSection
    │   └── HvdcConverterStation    getHvdcType(), getLossFactor(), getHvdcLine(),
    │       │                       getOtherConverterStation()
    │       └── VscConverterStation, LccConverterStation
    ├── Line, TwoWindingsTransformer   + the Branch methods: getTerminal1/2(),
    │                                    getTerminal(side or voltage level id), limits and
    │                                    operational limits groups of each side
    └── ThreeWindingsTransformer   getLegs(), getLeg(side), getTerminal(side or voltage level id)
Branch                    any Line or TwoWindingsTransformer: both convert to a Branch

Objects that are not identifiables: ThreeWindingsTransformer::Leg, RatioTapChanger,
PhaseTapChanger and their steps, ReactiveLimits and their points or planes, Terminal,
VoltageRegulation, OperationalLimitsGroup, LoadingLimits and their temporary limits, shunt
compensator models and their sections, the extensions...
```

- **Base classes, as in powsybl.** A `Generator` is an `Injection`, a `Connectable` and an
  `Identifiable`: it converts to any of them without losing anything, and generic code takes a
  `const Connectable&`. `Branch` is the exception: in powsybl a branch is not always a
  connectable (a tie line), so its methods come from a mixin (`detail::BranchMethods`) and
  `Branch` is a separate class that `Line` and `TwoWindingsTransformer` convert to.
- **Polymorphic results.** `Network::getIdentifiable(id)`, `getConnectable(id)`, `getBranch(id)`,
  `getIdentifiables()`, `getConnectables()`, `getBranches()`, `getHvdcConverterStations()`,
  `VoltageLevel::getConnectables()`, `Terminal::getConnectable()`, `HvdcLine::getConverterStation1()`
  return the base class. `getType()` gives the powsybl `IdentifiableType`; `is<T>()` tells whether
  the object is a `T` (Java `instanceof`), and `as<T>()` returns it as one
  (`PropertyNotFoundException` when it is not).
- **Equality is identity.** `a == b` when both stand for the same Java object, whatever the
  handles (each lookup returns a new handle).
- **Extensions on their powsybl host type.** ActivePowerControl and InjectionObservability are on
  `Injection`, Measurements on `Connectable`, DiscreteMeasurements on `Identifiable`,
  BranchObservability on the branches.
- **Sub-objects are objects.** A leg of a three windings transformer, a tap changer and its steps,
  reactive limits and their points, limits groups, limits and their temporary limits, a shunt
  compensator's model are objects of their own, as in powsybl:
  `twt.getLeg2().getRatioTapChanger()->getStep(-1)`, `gen.getReactiveLimits().getMaxQ(p)`,
  `line.getCurrentLimits1()->getTemporaryLimit(60)`,
  `shunt.getModel<ShuntCompensatorLinearModel>().getBPerSection()`.
  `is<T>()` / `as<T>()` work on any of them (`limits.as<ReactiveCapabilityCurve>()`).
- **Only current powsybl APIs** (powsybl 7.4): `getNameOrId()` / `getOptionalName()` rather than a
  `getName()`, `connect()` returning powsybl's result, the `VoltageRegulation` object of
  generators and of ratio tap changers.

```cpp
for (const Connectable& c : network.getVoltageLevel("VL1")->getConnectables()) {
    if (c.is<Generator>()) {
        Generator g = c.as<Generator>();
        g.setTargetP(g.getTargetP() + 10);
    }
}
Connectable owner = terminal.getConnectable();

g.setProperty("owner", "EDF");
iidm::optional<std::string> owner = g.getProperty("owner"); // empty when absent
g.addAlias("G1-code", "code");                               // type "" for none
iidm::optional<Identifiable> same = network.getIdentifiable("G1-code");
```

The main types:

```
Generator
├── getTargetP() / getMinP() / getMaxP() / getRatedS() → double   (+ setters)
├── getEnergySource() → EnergySource                               (+ setter)
├── isCondenser() → bool
└── + the voltage regulation holder and reactive limits holder methods (below)

Reactive limits holders: Generator, Battery, VscConverterStation
├── getReactiveLimits() → ReactiveLimits
└── getReactiveLimits<L>() → L, for L = MinMaxReactiveLimits, ReactiveCapabilityCurve or
    ReactiveCapabilityShape (ValidationException when they are of another kind)

ReactiveLimits                  getKind() → ReactiveLimitsKind, getMinQ(p) / getMaxQ(p),
│                               is<L>() / as<L>(), properties
├── MinMaxReactiveLimits        getMinQ() / getMaxQ()
├── ReactiveCapabilityCurve     getPoints() (by increasing p: getP(), getMinQ(), getMaxQ()),
│                               getPointCount(), getMinP() / getMaxP(),
│                               getMinQ(p, extrapolateReactiveLimitSlope) / getMaxQ(…)
└── ReactiveCapabilityShape     getMinQ(p, v) / getMaxQ(p, v), isInside(p, q, v), getPlanes()

Voltage regulation holders: Generator, Battery, StaticVarCompensator, ShuntCompensator,
VscConverterStation, RatioTapChanger (powsybl 7.4 VoltageRegulationHolder)
├── getLocalTargetV() / getLocalTargetQ() → double                (+ setters)
├── getVoltageRegulation() → optional<VoltageRegulation>
├── newVoltageRegulation() / removeVoltageRegulation()
├── isRegulating() / isRegulatingWithMode(mode) / isWithMode(mode) → bool
├── getRegulatingTargetV() / getRegulatingTargetQ() → double
├── getRegulatingTerminal() → Terminal / hasRegulatingTerminal() → bool
└── setTargetQToQ() / setTargetVToV()

VoltageRegulation (powsybl 7.4 core model)
├── getMode() → optional<RegulationMode>       setMode(RegulationMode)
├── isRegulating() → bool                      setRegulating(bool)
├── getTargetValue() / getTargetDeadband() / getSlope() → double (+ setters)
└── getTerminal() → optional<Terminal>         setTerminal(Terminal, targetValue)

Tap changers: RatioTapChanger and PhaseTapChanger, of a TwoWindingsTransformer or of a
ThreeWindingsTransformer::Leg (getRatioTapChanger() / getPhaseTapChanger() → optional)
├── getLowTapPosition() (+ setter) / getHighTapPosition() / getStepCount() → int
├── getTapPosition() → int (fails when unset)   findTapPosition() → optional<int>
│   setTapPosition(int) / unsetTapPosition()
├── getSolvedTapPosition() → optional<int>      setSolvedTapPosition(int) / unsetSolvedTapPosition()
├── getStep(tapPosition) → step                 getAllSteps() → std::map<int, step> (by tap position)
├── getCurrentStep() / getSolvedCurrentStep() / getNeutralStep() → optional<step>
├── isRegulating() / hasLoadTapChangingCapabilities() → bool (+ setter)
├── applySolvedValues() / setTapPositionToSolvedTapPosition() / remove()
├── RatioTapChanger: + the voltage regulation holder methods (it regulates through its
│   VoltageRegulation)
└── PhaseTapChanger: getRegulationMode() / getRegulationValue() / getTargetDeadband() /
    getRegulationTerminal() (+ setters), setRegulating(bool)
Steps: getRho() / getR() / getX() / getG() / getB() (+ setters), PhaseTapChangerStep::getAlpha()

ThreeWindingsTransformer::Leg
├── getTerminal() / getSide() / getTransformer()
├── getR() / getX() / getG() / getB() / getRatedU() → double, getRatedS() → optional (+ setters)
├── its tap changers, as a TwoWindingsTransformer
├── its limits, as a flows limits holder (below)
└── getLimits(type) / getAllSelectedLimits(type) → LoadingLimits of a LimitType

Flows limits holders: ThreeWindingsTransformer::Leg, DanglingLine, and each side of a Line or
TwoWindingsTransformer (the same methods, suffixed 1 / 2 or taking the side)
├── getOperationalLimitsGroups() / getOperationalLimitsGroup(id) → groups
├── newOperationalLimitsGroup(id) / removeOperationalLimitsGroup(id)
├── getSelectedOperationalLimitsGroup() / getSelectedOperationalLimitsGroupId() → optional
├── setSelectedOperationalLimitsGroup(id) / cancelSelectedOperationalLimitsGroup() /
│   getOrCreateSelectedOperationalLimitsGroup([id])
├── several selected groups: getAllSelectedOperationalLimitsGroups(), …Ids(), …IdsOrdered(),
│   addSelectedOperationalLimitsGroups(ids) / deselectOperationalLimitsGroups(ids)
└── the limits in force: getCurrentLimits() / getActivePowerLimits() / getApparentPowerLimits()
    (of the selected group), getAllSelected…Limits(), get…LimitsFromId(id)

OperationalLimitsGroup     getId(), isEmpty(), properties; get / new / remove the current,
                           active power and apparent power limits; getLoadingLimits(type)
LoadingLimits: CurrentLimits, ActivePowerLimits, ApparentPowerLimits
├── getPermanentLimit() / getPermanentLimitName() (+ setters), getLimitType(), getDetectionKind()
├── getTemporaryLimits() (by decreasing acceptable duration) / getTemporaryLimit(duration):
│   TemporaryLimit getName() / getValue() / getAcceptableDuration() / isFictitious(), properties
├── getTemporaryLimitValue(duration) / setTemporaryLimitValue(duration, value)
└── remove(), properties

ShuntCompensator (+ the voltage regulation holder methods)
├── getSectionCount() → int (fails when undefined)   findSectionCount() → optional<int>
│   setSectionCount(int) / unsetSectionCount()       getMaximumSectionCount() → int
├── getSolvedSectionCount() / findSolvedSectionCount() → optional<int>
│   setSolvedSectionCount(int) / unsetSolvedSectionCount()
├── getB() / getG() → double (at the section count), getB(n) / getG(n) (at n sections)
├── getModelType() → ShuntCompensatorModelType, getModel() → ShuntCompensatorModel,
│   getModel<M>() → M (ValidationException when the model is of the other type)
└── applySolvedValues() / setSectionCountToSolvedSectionCount()
ShuntCompensatorModel (properties)
├── ShuntCompensatorLinearModel      getBPerSection() / getGPerSection() (+ setters),
│                                    setMaximumSectionCount(int)
└── ShuntCompensatorNonLinearModel   getAllSections(), from the first one: Section getB() / getG()
                                     (+ setters), properties

Load
├── getP0() → double             setP0(double)
├── getQ0() → double             setQ0(double)
└── getLoadType() → LoadType

Terminal
├── getConnectable() → Connectable
├── getSide() → optional<ThreeSides>   (empty for an injection)
├── getBusView() → Bus
├── getP() → double              setP(double)
├── getQ() → double              setQ(double)
├── isConnected() → bool
└── connect() / disconnect() → bool    (true when this changed it)

Bus (from a bus-breaker view, or a terminal)
├── getV() → double              setV(double)
└── getAngle() → double          setAngle(double)
```

### 4.2 Handle Model

Every object holds the handle of its Java object:

```cpp
// include/iidm/ObjectHandle.h
namespace iidm {

// An opaque integer that identifies a Java object within a backend session. Each lookup of an
// object returns a new handle: compare the objects (==), not the handles.
using ObjectHandle = std::int64_t;
constexpr ObjectHandle INVALID_HANDLE = 0;

} // namespace iidm
```

The root of every class, `detail::ObjectRef`, holds it with the backend; the classes add methods
only:

```cpp
namespace iidm {

namespace detail {
class ObjectRef {
public:
    bool isValid() const;                            // bound to an object
    bool operator==(const ObjectRef& other) const;   // the same Java object
protected:
    HandleRef        handle_;   // releases the Java reference with the last wrapper (§7)
    BackendProvider* backend_;  // non-owning, owned by Network
};
}

class Generator : public Injection {
public:
    static constexpr int TYPE = prop::GENERATOR;     // for is<Generator>() / as<Generator>()

    Generator() = default;
    explicit Generator(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getTargetP() const;
    Generator& setTargetP(double targetP);
    // …
};

} // namespace iidm
```

Setters return the object for chaining (`gen.setTargetP(200).setMinP(0)`); setters of a base
class return the base class (`setName`, `setFictitious`, `setProperty`).

### 4.3 Network & NetworkFactory

```cpp
// include/iidm/Network.h
namespace iidm {

// An identifiable that owns its backend session
class Network : public Identifiable {
public:
    ~Network();
    Network(Network&&) noexcept;
    Network& operator=(Network&&) noexcept;

    // No copy: a Network owns its backend session
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    // Iteration helpers
    std::vector<Identifiable>            getIdentifiables() const;
    std::vector<Connectable>             getConnectables() const;
    std::vector<Branch>                  getBranches() const;
    std::vector<Substation>              getSubstations() const;
    std::vector<Generator>               getGenerators() const;
    // … and so on for each type

    // Lookup by ID (or alias)
    iidm::optional<Identifiable>           getIdentifiable(const std::string& id) const;
    iidm::optional<Generator>              getGenerator(const std::string& id) const;
    // … and so on for each type

    // Backend accessor (advanced usage only)
    BackendProvider& getBackend();

private:
    friend class NetworkFactory;
    explicit Network(std::unique_ptr<BackendProvider> backend);
    std::unique_ptr<BackendProvider> owned_;
};

} // namespace iidm
```

```cpp
// include/iidm/NetworkFactory.h
namespace iidm {

enum class BackendMode {
    AUTO,    // probe for existing JVM first, fall back to GraalVM
    GRAALVM, // always use GraalVM native image
    JNI      // always use existing JVM (throws if no JVM found)
};

struct NetworkLoadOptions {
    BackendMode mode = BackendMode::AUTO;
    // Path to the GraalVM shared library. If empty, searched on LD_LIBRARY_PATH.
    std::string graalvmLibPath;
    // Java network ID to look up in embedded mode (optional; if empty, takes
    // the first network registered in the JNI registry).
    std::string jniNetworkId;
};

class NetworkFactory {
public:
    // Load a network from an XIIDM file (standalone or embedded).
    static Network load(const std::string& filePath,
                        const NetworkLoadOptions& options = {});

    // Embedded mode only: wrap a network that is already in the JVM without
    // loading from file. The Java side must have called
    // IidmBridgeRegistry.register(network) before this point.
    static Network wrap(const std::string& networkId,
                        const NetworkLoadOptions& options = {});
};

} // namespace iidm
```

### 4.4 Value Types and Enumerations

Enumerations are defined as `enum class` in `include/iidm/Enums.h` and
mirror the Java equivalents:

```cpp
namespace iidm {

enum class EnergySource { HYDRO, NUCLEAR, WIND, THERMAL, SOLAR, OTHER };
enum class LoadType { UNDEFINED, AUXILIARY, FICTITIOUS };
enum class TopologyKind { NODE_BREAKER, BUS_BREAKER };
enum class Country { FR, DE, ES, IT, BE, NL, GB, /* … */ };
enum class HvdcConverterStationMode { RECTIFIER, INVERTER };
enum class RegulationMode { VOLTAGE, REACTIVE_POWER, VOLTAGE_PER_REACTIVE_POWER };
enum class PhaseTapChangerRegulationMode { CURRENT_LIMITER, ACTIVE_POWER_CONTROL };
enum class ReactiveLimitsKind { MIN_MAX, CURVE, SHAPE };
enum class SwitchKind { BREAKER, DISCONNECTOR, LOAD_BREAK_SWITCH };

} // namespace iidm
```

Optional doubles use `std::optional<double>` to represent undefined or NaN
values rather than sentinel values, providing a cleaner API than the Java
`Double.isNaN()` idiom.

---

### 4.5 Creating and Removing Objects

Objects are created with adders shaped like powsybl's, and removed with `remove()`:

```cpp
Network network = NetworkFactory::create("demo");          // or load an existing one
Substation s = network.newSubstation().setId("S").setCountry(Country::FR).add();
VoltageLevel vl = s.newVoltageLevel().setId("VL").setNominalV(400)
                   .setTopologyKind(TopologyKind::BUS_BREAKER).add();
vl.getBusBreakerView().newBus().setId("B").add();
Generator g = vl.newGenerator().setId("G").setBus("B")
                .setMinP(0).setMaxP(200).setTargetP(100).setLocalTargetV(400)
                .newVoltageRegulation().withMode(RegulationMode::VOLTAGE).withRegulating(true).add()
                .add();
g.newActivePowerControl().withDroop(4).withParticipate(true).add();
Load load = vl.newLoad().setId("L").setBus("B").setP0(80).setQ0(10).add();

load.remove();
g.removeActivePowerControl();
```

- **Validation is powsybl's.** A missing or invalid field, a duplicate id or an unknown bus throws
  `ValidationException` from `add()`, and nothing is created.
- **Only current APIs.** The generator's voltage regulation follows powsybl 7.4: a nested
  `newVoltageRegulation()` adder, local targets on the generator (`setLocalTargetV/Q`). Without a
  terminal the regulation is local and its target value is NaN; `VoltageRegulation::setTerminal(terminal,
  targetValue)` switches to remote regulation (both change together).
- **Removal.** `remove()` removes the object from the network (a voltage level or substation with its
  equipment). powsybl refuses some removals, e.g. a voltage level that branches still connect:
  `ValidationException`. A removed object's wrapper keeps its handle: powsybl decides what using it
  does (changes are rejected with `ValidationException`).
- **Available so far:** network, substation, voltage level (with or without substation), bus-breaker
  bus, generator, load, generator voltage regulation, ActivePowerControl (generator, battery).

Underneath, there is one backend primitive, `invoke(handle, code)` (§5.1): `invoke(parent, TYPE)`
returns a Java adder, the adder's fields are set with the usual setters and the object's own
property codes (`setDouble(adder, GEN_TARGET_P, 100)`), and `invoke(adder, ACT_ADD)` adds the
object. The Java side (`Creation.java`) maps each field to the powsybl adder method.

## 5. Backend Abstraction Layer

### 5.1 BackendProvider Interface

Every class of §4 goes through these primitives only, each taking the handle of an object and an
`int` code:

```cpp
// include/iidm/BackendProvider.h
namespace iidm {

class BackendProvider {
public:
    virtual ~BackendProvider();

    // ── Properties ───────────────────────────────────────────────────────
    virtual double      getDouble(ObjectHandle h, int property) const = 0;
    virtual int         getInt(ObjectHandle h, int property) const = 0;
    virtual bool        getBool(ObjectHandle h, int property) const = 0;
    virtual std::string getString(ObjectHandle h, int property) const = 0;
    virtual void setDouble(ObjectHandle h, int property, double value) = 0;
    virtual void setInt   (ObjectHandle h, int property, int value) = 0;
    virtual void setBool  (ObjectHandle h, int property, bool value) = 0;
    virtual void setString(ObjectHandle h, int property, const std::string& value) = 0;

    // Properties that may have no value: empty optional (NaN for doubles)
    virtual iidm::optional<double>      getOptDouble(ObjectHandle h, int property) const = 0;
    virtual iidm::optional<int>         getOptInt(ObjectHandle h, int property) const;
    virtual iidm::optional<std::string> getOptString(ObjectHandle h, int property) const;

    // Lists, and values under a key (properties, aliases); an empty value removes the key
    virtual std::vector<int>            getIntList(ObjectHandle h, int listCode) const = 0;
    virtual std::vector<std::string>    getStringList(ObjectHandle h, int listCode) const;
    virtual iidm::optional<std::string> getKeyedString(ObjectHandle h, int property, const std::string& key) const;
    virtual void setKeyedString(ObjectHandle h, int property, const std::string& key,
                                const iidm::optional<std::string>& value);

    // A method that takes numeric arguments (getMinQ(p) of reactive limits); bools come as 1 / 0
    virtual double evaluate(ObjectHandle h, int code, const std::vector<double>& args) const;

    // ── Navigation ───────────────────────────────────────────────────────
    virtual std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const = 0;
    virtual ObjectHandle getRelated(ObjectHandle h, int relation) const = 0;
    virtual ObjectHandle getRelatedByIndex(ObjectHandle h, int relation, int index) const = 0;
    virtual ObjectHandle getRelatedByString(ObjectHandle h, int relation, const std::string& key) const = 0;
    virtual void setRelated(ObjectHandle h, int relation, ObjectHandle value);
    virtual ObjectHandle findById(int objectType, const std::string& id) const = 0;
    // Whether two handles stand for the same object
    virtual bool isSameObject(ObjectHandle h, ObjectHandle other) const;

    // ── Actions ──────────────────────────────────────────────────────────
    // invoke(parent, TYPE) → adder; invoke(adder, ACT_ADD) → new object;
    // invoke(object, ACT_REMOVE) removes it; ACT_CONNECT / ACT_DISCONNECT
    virtual ObjectHandle invoke(ObjectHandle h, int code);

    // ── Network and lifecycle ────────────────────────────────────────────
    virtual ObjectHandle getNetworkHandle() const = 0;
    virtual void loadNetwork(const std::string& filePath) = 0;
    virtual void createNetwork(const std::string& id);
    virtual void close() = 0;
    // … in-memory load and save, handle release
};

} // namespace iidm
```

Codes are defined **once**, in `include/iidm/PropertyCodes.def`. `PropertyCodes.h` expands it into
`iidm::prop::NAME` constants, and the Java build generates `com.powsybl.iidmbridge.PropertyCodes`
from the same file, so the C++ library, both backends and the Java side cannot disagree on a number:

```cpp
// include/iidm/PropertyCodes.def
// Object types
IIDM_CODE(SUBSTATION,                  1)
IIDM_CODE(VOLTAGE_LEVEL,               2)
IIDM_CODE(GENERATOR,                   3)
// …
IIDM_CODE(IDENTIFIABLE,                43)
IIDM_CODE(CONNECTABLE,                 44)
// Identifiable
IIDM_CODE(ID,                          100)
IIDM_CODE(NAME,                        101)
// Generator
IIDM_CODE(GEN_TARGET_P,                200)
// … and so on
```

A code is named after the powsybl interface that declares the method (`CONVERTER_STATION_LOSS_FACTOR`
for `HvdcConverterStation.getLossFactor`, whatever the station), and means the same in every
primitive. A type code stands for its type: `getChildren(parent, TYPE)` lists the parent's objects
of that type, `findById(TYPE, id)` looks one up, `getBool(object, TYPE)` tells whether the object is
one, `invoke(parent, TYPE)` starts a new one. For a property that may have no value,
`getString`/`getInt` give a value or fail (`getString(NAME)` is the name, or else the id) while
`getOptString`/`getOptInt` give it as it is (`getOptString(NAME)` is empty without a name). A
method with arguments keeps the code of its name: `getDouble(limits, RL_MIN_Q)` is
`MinMaxReactiveLimits.getMinQ()`, `evaluate(limits, RL_MIN_Q, {p})` is `ReactiveLimits.getMinQ(p)`;
the number of arguments chooses between overloads.

### 5.2 GraalVM Native Image Backend

#### Overview

The GraalVM backend loads `libpowsybl-iidm-native.so` at runtime via
`dlopen`, resolves function pointers, and calls into the native image. The
native image is produced by compiling the Java side (see §6.1) with
GraalVM's `native-image` tool.

#### Loading the shared library

```cpp
// src/graalvm/GraalVMBackend.h
namespace iidm {

class GraalVMBackend : public BackendProvider {
public:
    explicit GraalVMBackend(const std::string& libraryPath = "");
    ~GraalVMBackend() override;

    // BackendProvider implementation …

private:
    void* libHandle_ = nullptr;    // dlopen handle

    // GraalVM isolate (one per process for the native image)
    graal_isolate_t*        isolate_ = nullptr;
    graal_isolatethread_t*  thread_  = nullptr;

    // C function pointers resolved from the shared library
    using FnLoadNetwork      = ObjectHandle(*)(graal_isolatethread_t*, const char*);
    using FnGetDouble        = double(*)(graal_isolatethread_t*, ObjectHandle, int);
    using FnSetDouble        = void(*)(graal_isolatethread_t*, ObjectHandle, int, double);
    using FnGetString        = const char*(*)(graal_isolatethread_t*, ObjectHandle, int);
    using FnGetChildren      = void(*)(graal_isolatethread_t*, ObjectHandle, int,
                                       ObjectHandle*, int*);
    // … other function pointer types

    FnLoadNetwork  fnLoadNetwork_  = nullptr;
    FnGetDouble    fnGetDouble_    = nullptr;
    FnSetDouble    fnSetDouble_    = nullptr;
    FnGetString    fnGetString_    = nullptr;
    FnGetChildren  fnGetChildren_  = nullptr;
    // …

    void resolveSymbols();
};

} // namespace iidm
```

#### The GraalVM isolate

A GraalVM native image process maintains an **isolate** — a heap-isolated
execution context. Each thread that calls into the image must hold an
`isolatethread_t`. The backend creates a single global isolate at
construction time and tears it down when `close()` is called. All calls
from the same C++ thread reuse the same `isolatethread_t`.

```cpp
// src/graalvm/GraalVMBackend.cpp (excerpt)

void GraalVMBackend::init(const std::string& libPath) {
    libHandle_ = dlopen(libPath.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (!libHandle_) throw IidmException("Cannot load " + libPath + ": " + dlerror());

    resolveSymbols();

    // Create the GraalVM isolate
    auto createIsolate = reinterpret_cast<int(*)(graal_create_isolate_params_t*,
                                                  graal_isolate_t**,
                                                  graal_isolatethread_t**)>(
        dlsym(libHandle_, "graal_create_isolate"));
    if (createIsolate(nullptr, &isolate_, &thread_) != 0)
        throw IidmException("Failed to create GraalVM isolate");
}

double GraalVMBackend::getDouble(ObjectHandle h, int property) const {
    return fnGetDouble_(thread_, h, property);
}

void GraalVMBackend::setDouble(ObjectHandle h, int property, double value) {
    fnSetDouble_(thread_, h, property, value);
}
```

#### Strings

Strings cross the boundary as UTF-8 C strings, decoded and encoded in Java with
`StandardCharsets.UTF_8` (a null pointer is "no value" for the optional and keyed getters).
When the native image returns a string, it is allocated with `UnmanagedMemory`; the C++ backend
copies it into a `std::string` immediately and releases it with `iidm_free`. A string list is one
buffer of strings each followed by a 0 byte. **No raw pointers escape the backend layer.**

The JNI backend converts the same way: UTF-8 to UTF-16 in C++ (Java's own decoder when the bytes
are not well-formed UTF-8), and back as `String.getBytes(UTF_8)` does. JNI's `NewStringUTF` /
`GetStringUTFChars` are not used: they take modified UTF-8, which differs outside the BMP.

### 5.3 JNI Embedded Backend

#### Overview

In embedded mode Dynawo is started from a Java program. The JVM is already
running and contains a loaded `com.powsybl.iidm.network.Network` object.
The `JNIBackend` attaches to that JVM and obtains a reference to the
network through a static registry class on the Java side (see §6.2).

#### Attaching to the JVM

```cpp
// src/jni/JNIBackend.cpp (excerpt)

void JNIBackend::attachToJVM() {
    // Locate any existing JVMs
    JavaVM* jvms[1];
    jsize   count = 0;
    jint rc = JNI_GetCreatedJavaVMs(jvms, 1, &count);
    if (rc != JNI_OK || count == 0)
        throw IidmException("No JVM found; cannot use JNI backend");

    jvm_ = jvms[0];

    // Attach this thread to the JVM
    JNIEnv* env = nullptr;
    rc = jvm_->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
    if (rc != JNI_OK)
        throw IidmException("Failed to attach to JVM thread");

    env_ = env;
    cacheMethodIds();
}
```

#### One dispatch table: `PropertyDispatcher`

The JNI backend does not call IIDM methods itself. Each `BackendProvider` operation is a
single static JNI call to the object API of `PropertyDispatcher` — the same Java dispatch
table the GraalVM backend uses, so both backends behave identically:

```cpp
// src/jni/JNIBackend.cpp
double JNIBackend::getDouble(ObjectHandle h, int property) const {
    double result = env_->CallStaticDoubleMethod(cache_.dispatcherClass, cache_.disp_getDouble,
                                                 toObject(h), static_cast<jint>(property));
    checkJNIException(env_);
    return result;
}
```

Only the registry calls (`IidmBridgeRegistry`: network lookup, load / save) and the
dispatcher methods are resolved, once, at construction (`JNIMethodCache.h`). A simple
property read costs about 150–250 ns, the same as the former hand-written JNI calls.

#### Handle = global JNI reference

In the JNI backend, an `ObjectHandle` is the integer value of a `jobject` **global
reference**. Each handle returned to C++ (children, related objects, lookups) is a new
global reference; it is deleted (`DeleteGlobalRef`) when the last C++ wrapper using it is
destroyed — see §7.

#### Exception bridging

JNI does not propagate Java exceptions automatically. After every JNI call
the backend checks `ExceptionOccurred()` and, if set, clears it and throws
a C++ `IidmException` containing the Java exception's message.

### 5.4 Extensions

IIDM extensions are optional property bags that can be attached to network
components (e.g. `ActivePowerControl` on any injection). The bridge exposes
them through the same `BackendProvider` property-code mechanism as core
properties, but the two backends handle the dispatch differently.

#### C++ surface

An extension is an object of its own, reached from its host. Its wrapper holds the extension's
handle and uses the extension's codes; the host has `hasX()`, `getX()` (`ExtensionNotFoundException`
when absent), `newX()` and `removeX()`:

```cpp
// include/iidm/ActivePowerControl.h
class ActivePowerControl : public detail::ObjectRef {
public:
    explicit ActivePowerControl(ObjectHandle handle, BackendProvider* backend);
    bool   isParticipate() const;                     // EXT_APC_PARTICIPATE
    ActivePowerControl& setParticipate(bool participate);
    double getDroop() const;                          // EXT_APC_DROOP
    ActivePowerControl& setDroop(double droop);
    // …
};

// include/iidm/Injection.h: the host type of ActivePowerControl
bool                    hasActivePowerControl() const;
ActivePowerControl      getActivePowerControl() const;
ActivePowerControlAdder newActivePowerControl();
void                    removeActivePowerControl();
```

Each extension has a block of codes in `PropertyCodes.def`. Its first code is the extension's
**type code**, used for every operation on the host; the others are its properties:

```cpp
IIDM_CODE(ACTIVE_POWER_CONTROL, 3000) // getRelated(host, …): the extension (INVALID_HANDLE when absent)
                                      // invoke(host, …): its adder; setRelated(host, …, INVALID_HANDLE): removes it
IIDM_CODE(EXT_APC_DROOP,        3001) // on the extension object
IIDM_CODE(EXT_APC_PARTICIPATE,  3002)
```

The extension type is the same whatever the host, so a generator's and a battery's
ActivePowerControl use the same codes. What an extension holds is made of objects too: the
quality of each quantity of an observability extension (`getRelated(extension, OBS_QUALITY_P)`,
an `ObservabilityQuality`), the measurements of a `Measurements` extension (children of type
`MEASUREMENT`, one of them by id with `getRelatedByString`, a new one with `invoke`), the areas
of an `ObservabilityArea`.

#### GraalVM backend — automatic

The GraalVM entry points (`iidm_get_double`, `iidm_get_bool`, …) forward
**every** property code to `PropertyDispatcher.java` without a hard-coded
switch. Adding an extension only requires new cases in `PropertyDispatcher`
(and `Creation.java` for its adder):

```java
// PropertyDispatcher.java
case ACTIVE_POWER_CONTROL -> extension(obj, ActivePowerControl.class); // getRelatedObject
case EXT_APC_DROOP        -> activePowerControl(obj).getDroop();         // getDouble
case EXT_APC_PARTICIPATE  -> activePowerControl(obj).isParticipate();    // getBool
```

No C++ changes are needed in the GraalVM backend.

#### JNI backend — same dispatch table

The JNI backend sends every property, child and relation code to the object-based
overloads of `PropertyDispatcher` (`getDouble(Object, int)`, `getChildObjects(Object, int)`,
`getRelatedObject(Object, int)`, …), so a new extension only needs `PropertyDispatcher`
cases and works on both backends. No JNI method signature has to be kept in sync with
powsybl-core.

#### Supported extensions

Every extension below has an adder (`newX()`), except `ObservabilityArea` for now.

| Extension | Hosts | Codes |
|---|---|---|
| `ActivePowerControl` | all injections | `ACTIVE_POWER_CONTROL`, `EXT_APC_*` |
| `CoordinatedReactiveControl` | Generator | `COORDINATED_REACTIVE_CONTROL`, `EXT_CRC_*` |
| `HvdcAngleDroopActivePowerControl` | HvdcLine | `HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL`, `EXT_HADAPC_*` |
| `HvdcOperatorActivePowerRange` | HvdcLine | `HVDC_OPERATOR_ACTIVE_POWER_RANGE`, `EXT_HOAR_*` |
| `SlackTerminal` | VoltageLevel | `SLACK_TERMINAL`, `EXT_ST_*`, `REL_TERMINAL` |
| `Measurements` | all connectables | `MEASUREMENTS`; its `MEASUREMENT`s: `MEAS_*` |
| `DiscreteMeasurements` | all identifiables | `DISCRETE_MEASUREMENTS`; its `DISCRETE_MEASUREMENT`s: `DMEAS_*` |
| `InjectionObservability` | all injections incl. BusbarSection | `INJECTION_OBSERVABILITY`, `EXT_OBS_OBSERVABLE`, `OBS_QUALITY_*` |
| `BranchObservability` | all branches | `BRANCH_OBSERVABILITY`, `EXT_OBS_OBSERVABLE`, `OBS_QUALITY_*` |
| `TwoWindingsTransformerToBeEstimated` | 2WT | `TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED`, `EXT_TWO_WT_TBE_*` |
| `ThreeWindingsTransformerToBeEstimated` | 3WT | `THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED`, `EXT_THREE_WT_TBE_*` |
| `ObservabilityArea` | VoltageLevel (read-only) | `OBSERVABILITY_AREA`, `EXT_OBS_AREA_*`; its `AREA_CHARACTERISTICS`: `OBS_AREA_*` |

The `VoltagePerReactivePowerControl`, `RemoteReactivePowerControl` and battery `VoltageRegulation`
extensions were removed from powsybl-core 7.4 (`extensions.removed`): their data is the core
`VoltageRegulation` of the holder (mode `VOLTAGE_PER_REACTIVE_POWER` and its slope, a remote
`REACTIVE_POWER` regulation, the battery's regulation), so they are not ported.

The state estimation extensions are exercised end to end by
`examples/state_estimation_test.xiidm` (see `examples/StateEstimationReport.h`).

#### Checklist for adding a new extension

| Step | File(s) |
|---|---|
| Codes: the type code, then one per property | `include/iidm/PropertyCodes.def` (one line each) |
| C++ extension wrapper and adder | `include/iidm/MyExt.h` + `src/MyExt.cpp`, `include/iidm/MyExtAdder.h` (+ `CMakeLists.txt`, `iidm.h`) |
| Host accessors: `hasMyExt`, `getMyExt`, `newMyExt`, `removeMyExt`, on the class of the host type | e.g. `Injection.h` / `Injection.cpp` |
| `PropertyDispatcher` cases (getRelatedObject, properties) and `Creation` cases (adder, fields, removal) | `PropertyDispatcher.java`, `Creation.java` — serve **both** backends |
| C++ unit tests | `test/TestMyExt.cpp` with `MockBackend` |
| Java dispatcher tests | `java/src/test/...` (covers the handle and object APIs) |

### 5.5 Backend Selection Strategy

```cpp
// src/BackendSelector.cpp

std::unique_ptr<BackendProvider> selectBackend(const NetworkLoadOptions& opts) {
    // Explicit override via environment variable
    const char* envMode = std::getenv("IIDM_BRIDGE_MODE");
    BackendMode effectiveMode = opts.mode;
    if (envMode) {
        if (std::string(envMode) == "graalvm") effectiveMode = BackendMode::GRAALVM;
        else if (std::string(envMode) == "jni")     effectiveMode = BackendMode::JNI;
    }

    if (effectiveMode == BackendMode::JNI) {
        return std::make_unique<JNIBackend>(opts.jniNetworkId);
    }
    if (effectiveMode == BackendMode::GRAALVM) {
        return std::make_unique<GraalVMBackend>(opts.graalvmLibPath);
    }

    // AUTO: probe for existing JVM
    JavaVM* jvms[1]; jsize count = 0;
    if (JNI_GetCreatedJavaVMs(jvms, 1, &count) == JNI_OK && count > 0) {
        return std::make_unique<JNIBackend>(opts.jniNetworkId);
    }
    return std::make_unique<GraalVMBackend>(opts.graalvmLibPath);
}
```

---

## 6. Java-Side Design

The Java side lives in the Maven/Gradle module `iidm-bridge-java`. It has
two distinct roles depending on the mode.

### 6.1 GraalVM Entry Points

GraalVM's `native-image` tool compiles any method annotated with
`@CEntryPoint` into an exported C function. The Java module defines one
entry point per BackendProvider operation.

```java
// com/powsybl/iidmbridge/graalvm/IidmEntryPoints.java

public final class IidmEntryPoints {

    @CEntryPoint(name = "iidm_load_network")
    public static long loadNetwork(
            IsolateThread thread,
            CCharPointer filePath) {
        String path = CTypeConversion.toJavaString(filePath);
        Network network = NetworkSerDe.read(Path.of(path));
        return NetworkRegistry.register(network);
    }

    @CEntryPoint(name = "iidm_get_double")
    public static double getDouble(
            IsolateThread thread,
            long handle,
            int property) {
        return PropertyDispatcher.getDouble(handle, property);
    }

    @CEntryPoint(name = "iidm_set_double")
    public static void setDouble(
            IsolateThread thread,
            long handle,
            int property,
            double value) {
        PropertyDispatcher.setDouble(handle, property, value);
    }

    @CEntryPoint(name = "iidm_get_string")
    public static CCharPointer getString(
            IsolateThread thread,
            long handle,
            int property) {
        String value = PropertyDispatcher.getString(handle, property);
        return CTypeConversion.toCString(value).get();
    }

    @CEntryPoint(name = "iidm_free_string")
    public static void freeString(IsolateThread thread, CCharPointer ptr) {
        // Managed by GraalVM allocator; call into UnmanagedMemory
        UnmanagedMemory.free(ptr);
    }

    @CEntryPoint(name = "iidm_get_children")
    public static void getChildren(
            IsolateThread thread,
            long parentHandle,
            int childType,
            CLongPointer outHandles,
            CIntPointer outCount) {
        long[] handles = PropertyDispatcher.getChildren(parentHandle, childType);
        outCount.write(handles.length);
        for (int i = 0; i < handles.length; i++) {
            outHandles.addressOf(i).write(handles[i]);
        }
    }
}
```

#### NetworkRegistry

In the GraalVM case, Java objects cannot be passed as pointers across the
C boundary. Instead, a `NetworkRegistry` maps long integer handles to Java
object references:

```java
// com/powsybl/iidmbridge/graalvm/NetworkRegistry.java

public final class NetworkRegistry {
    // handle → Java object (any IIDM object, not just Network)
    private static final Map<Long, Object> registry = new ConcurrentHashMap<>();
    private static final AtomicLong nextHandle = new AtomicLong(1);

    public static long register(Object obj) {
        long handle = nextHandle.getAndIncrement();
        registry.put(handle, obj);
        return handle;
    }

    public static Object lookup(long handle) {
        Object obj = registry.get(handle);
        if (obj == null) throw new IllegalArgumentException("Unknown handle: " + handle);
        return obj;
    }

    public static void release(long handle) {
        registry.remove(handle);
    }
}
```

#### PropertyDispatcher

```java
// com/powsybl/iidmbridge/PropertyDispatcher.java

public final class PropertyDispatcher {

    public static double getDouble(long handle, int property) {
        Object obj = NetworkRegistry.lookup(handle);
        return switch (property) {
            case PropertyCodes.GEN_TARGET_P -> ((Generator) obj).getTargetP();
            case PropertyCodes.LOCAL_TARGET_Q -> ((VoltageRegulationHolder<?>) obj).getLocalTargetQ();
            case PropertyCodes.GEN_MIN_P    -> ((Generator) obj).getMinP();
            case PropertyCodes.GEN_MAX_P    -> ((Generator) obj).getMaxP();
            case PropertyCodes.LOAD_P0      -> ((Load) obj).getP0();
            case PropertyCodes.LOAD_Q0      -> ((Load) obj).getQ0();
            case PropertyCodes.TERMINAL_P   -> ((Terminal) obj).getP();
            case PropertyCodes.TERMINAL_Q   -> ((Terminal) obj).getQ();
            case PropertyCodes.BUS_V        -> ((Bus) obj).getV();
            case PropertyCodes.BUS_ANGLE    -> ((Bus) obj).getAngle();
            // …
            default -> throw new IllegalArgumentException("Unknown property: " + property);
        };
    }

    public static void setDouble(long handle, int property, double value) {
        Object obj = NetworkRegistry.lookup(handle);
        switch (property) {
            case PropertyCodes.GEN_TARGET_P -> ((Generator) obj).setTargetP(value);
            case PropertyCodes.LOCAL_TARGET_Q -> ((VoltageRegulationHolder<?>) obj).setLocalTargetQ(value);
            case PropertyCodes.LOAD_P0      -> ((Load) obj).setP0(value);
            case PropertyCodes.LOAD_Q0      -> ((Load) obj).setQ0(value);
            case PropertyCodes.TERMINAL_P   -> ((Terminal) obj).setP(value);
            case PropertyCodes.TERMINAL_Q   -> ((Terminal) obj).setQ(value);
            case PropertyCodes.BUS_V        -> ((Bus) obj).setV(value);
            case PropertyCodes.BUS_ANGLE    -> ((Bus) obj).setAngle(value);
            // …
            default -> throw new IllegalArgumentException("Unknown property: " + property);
        }
    }

    public static long[] getChildren(long parentHandle, int childType) {
        Object obj = NetworkRegistry.lookup(parentHandle);
        Stream<?> children = switch (childType) {
            case PropertyCodes.GENERATOR   -> ((Network) obj).getGeneratorStream();
            case PropertyCodes.LOAD        -> ((Network) obj).getLoadStream();
            case PropertyCodes.LINE        -> ((Network) obj).getLineStream();
            case PropertyCodes.SUBSTATION  -> ((Network) obj).getSubstationStream();
            case PropertyCodes.VOLTAGE_LEVEL -> ((Network) obj).getVoltageLevelStream();
            // …
            default -> throw new IllegalArgumentException("Unknown child type: " + childType);
        };
        return children
            .mapToLong(NetworkRegistry::register)
            .toArray();
    }
}
```

### 6.2 JNI Registry for Embedded Mode

When Dynawo is launched from a Java orchestrator, the Java side registers
the network before handing off control to the C++ code:

```java
// com/powsybl/iidmbridge/jni/IidmBridgeRegistry.java

/**
 * Used in embedded mode to make a network available to the C++ JNI backend.
 * The Java program must call {@code register} before Dynawo's C++ main starts.
 */
public final class IidmBridgeRegistry {

    private static final Map<String, Network> networks = new ConcurrentHashMap<>();

    /** Register a network under a given ID. */
    public static void register(String id, Network network) {
        networks.put(id, network);
    }

    /** Register under the default ID "default". */
    public static void register(Network network) {
        register("default", network);
    }

    /** Called from JNI to retrieve the network object. */
    public static Network get(String id) {
        Network n = networks.get(id);
        if (n == null) throw new IllegalStateException("No network registered with id: " + id);
        return n;
    }

    /** Called from JNI to list all registered network IDs. */
    public static String[] listIds() {
        return networks.keySet().toArray(new String[0]);
    }
}
```

**Typical Java orchestrator code:**

```java
// Java side before launching Dynawo
Network network = NetworkSerDe.read(Path.of("case.xiidm"));
IidmBridgeRegistry.register("case1", network);

// Launch Dynawo via JNI or process API
DynawoLauncher.run(simulationParameters);
// At this point, Dynawo's C++ backend attaches to this JVM,
// finds "case1" via IidmBridgeRegistry, and uses it directly.
```

---

## 7. Lifecycle and Memory Management

### Object lifetime rules

| Object | Owner | Lifetime |
|---|---|---|
| `Network` | Caller (move-only) | Until `Network` goes out of scope |
| `BackendProvider` | `Network` (unique_ptr) | Destroyed with `Network` |
| Component objects (e.g. `Generator`) | Caller (value type) | Usable as long as parent `Network` is alive |
| Object handles (JNI global refs, GraalVM registry entries) | Component objects, via `HandleRef` | Released when the last wrapper using them is destroyed, or when the backend closes |
| GraalVM isolate | `GraalVMBackend` | One per process; destroyed in destructor |
| JNI thread attachment | `JNIBackend` | Detached in destructor via `DetachCurrentThread` |

### Handle release

Every handle returned by a backend holds a Java-side reference (a JNI global reference, or
an entry of the GraalVM `NetworkRegistry`). Wrappers hold it through `HandleRef`, which
counts the wrappers using a handle — across copies, and across sub-wrappers built from the
same handle (the views read through their host) — and calls
`BackendProvider::releaseHandle()` when the last one is destroyed. So a loop like

```cpp
for (int step = 0; step < 1000000; ++step)
    p += gen.getTerminal().getP();   // a new terminal handle each time
```

keeps a constant memory footprint on both backends. When a backend closes it releases every
handle still held; wrappers destroyed afterwards release nothing (`BackendProvider::
liveHandleCount()` reports the handles currently held).

### Invalidation

Calling a method on a component object after the parent `Network` has been destroyed is
**undefined behavior**; destroying it afterwards is safe. Objects never bound to a network
(default-constructed, moved-from `Network`) throw `InvalidHandleException` (§9).

### GraalVM isolate lifecycle

Because a process can have only **one** GraalVM isolate for a given native
image library, the `GraalVMBackend` uses a reference-counted singleton
pattern for the isolate. Multiple `Network` objects sharing the same
`libpowsybl-iidm-native.so` path share one isolate.

---

## 8. Thread Safety

| Backend | Thread model |
|---|---|
| **GraalVM** | Each OS thread must attach to the isolate before calling. The backend creates an `isolatethread_t` on first use per thread (thread-local storage). Multiple threads may call concurrently; GraalVM's Java code is responsible for its own internal synchronization. |
| **JNI** | Each OS thread that calls backend methods must be attached to the JVM (`AttachCurrentThread`). The backend performs this lazily on first use per thread via a thread-local flag. JVM internal locking handles Java-side concurrency. |

C++ component objects themselves are **not thread-safe** at the object
level (i.e., calling `setTargetP` and `getTargetP` on the same `Generator`
from two threads simultaneously requires external synchronization). This
matches the contract of the Java IIDM objects themselves.

---

## 9. Error Handling

All errors are reported via C++ exceptions, with the **same exception type on both
backends**. A Java-side failure never aborts the process: it surfaces as one of these
exceptions and the backend stays usable.

```cpp
// include/iidm/IidmException.h
namespace iidm {

class IidmException : public std::runtime_error { /* … */ };            // base of all

class BackendNotAvailableException : public IidmException { /* … */ };
class InvalidHandleException       : public IidmException { /* … */ };  // handle 0 / unknown
class PropertyNotFoundException    : public IidmException { /* … */ };  // code unknown or not
                                                                         // applicable, optional part absent
class ExtensionNotFoundException   : public PropertyNotFoundException { /* … */ };  // getX() of an
                                                                         // absent extension
class ValidationException          : public IidmException { /* … */ };  // the model rejected the change
class NetworkNotFoundException     : public IidmException { /* … */ };

} // namespace iidm
```

```cpp
try {
    gen.getActivePowerControl().setMaxTargetP(500.0);   // outside [minP, maxP]
} catch (const iidm::ValidationException& e) {
    // e.what(): "maxTargetP value (500.0) is not between minP and maxP for component G"
}
```

| Java failure | C++ exception |
|---|---|
| `BridgeErrors.InvalidHandleException` | `InvalidHandleException` |
| `BridgeErrors.PropertyNotFoundException`, `ClassCastException`, `NullPointerException` | `PropertyNotFoundException` |
| `PowsyblException` (incl. `ValidationException`), `IllegalArgumentException`, `UnsupportedOperationException` | `ValidationException` |
| `BridgeErrors.NetworkNotFoundException` | `NetworkNotFoundException` |
| anything else | `IidmException` (message prefixed with the Java class) |

The classification lives in one place, `BridgeErrors.java`
(`kindOf(Throwable)` / `message(Throwable)`), and is used by both backends:

- **GraalVM** — an exception escaping a `@CEntryPoint` would abort the process, so every
  entry point catches `Throwable`, stores it in a thread-local and returns its kind as a
  status (0 = success; results go through out-pointers). On a non-zero status
  `GraalVMBackend::check()` fetches the message with `iidm_take_last_error` and throws via
  `detail::throwBridgeError` (`src/BridgeErrors.h`). Successful calls cost nothing extra.
- **JNI** — `checkJNIException()` classifies the pending Java exception with the same
  `BridgeErrors` methods and throws via `detail::throwBridgeError`. A null receiver
  (handle 0) is rejected in C++ before any JNI call.

Objects that are not bound to a network — default-constructed wrappers
(`iidm::Generator g; g.getTargetP();`) or a moved-from `Network` — throw
`InvalidHandleException` too: their backend is a shared null object
(`detail::nullBackend()`) whose every operation throws, so no call dereferences a null
pointer. `isValid()` and `operator==` never throw.

`test/TestBackendErrors.cpp` runs the same failure scenarios against the real GraalVM
library (`IIDM_BRIDGE_GRAALVM_LIB`) and against a JVM created in-process
(`IIDM_BRIDGE_JAR`); `test/TestBackendConformance.cpp` checks that both backends throw the
same exception type for every failing call (§13).

---

## 10. Directory Layout

```
iidm-bridge-cpp/
│
├── CMakeLists.txt                # Top-level CMake
├── README.md                     # This file
│
├── include/
│   └── iidm/
│       ├── iidm.h                # Convenience all-in-one header
│       ├── Network.h
│       ├── NetworkFactory.h
│       ├── ObjectHandle.h
│       ├── BackendProvider.h
│       ├── PropertyCodes.def     # single definition of the property codes
│       ├── PropertyCodes.h       # expands PropertyCodes.def into iidm::prop constants
│       ├── IidmException.h
│       ├── Enums.h
│       ├── Terminal.h
│       ├── Bus.h
│       ├── Substation.h
│       ├── VoltageLevel.h
│       ├── Generator.h
│       ├── Load.h
│       ├── Line.h
│       ├── TwoWindingsTransformer.h
│       ├── ThreeWindingsTransformer.h
│       ├── HvdcLine.h
│       ├── DanglingLine.h
│       ├── ShuntCompensator.h
│       ├── StaticVarCompensator.h
│       ├── VscConverterStation.h
│       └── LccConverterStation.h
│
├── src/
│   ├── Network.cpp
│   ├── NetworkFactory.cpp
│   ├── BackendSelector.cpp
│   ├── Generator.cpp
│   ├── Load.cpp
│   ├── Terminal.cpp
│   ├── Bus.cpp
│   ├── …
│   ├── graalvm/
│   │   ├── GraalVMBackend.h
│   │   └── GraalVMBackend.cpp
│   └── jni/
│       ├── JNIBackend.h
│       ├── JNIBackend.cpp
│       ├── JNIMethodCache.h
│       └── JNIExceptionBridge.h
│
├── java/                         # Maven module: iidm-bridge-java
│   ├── pom.xml
│   └── src/main/java/com/powsybl/iidmbridge/
│       ├── PropertyDispatcher.java
│       ├── graalvm/
│       │   ├── IidmEntryPoints.java
│       │   └── NetworkRegistry.java
│       └── jni/
│           └── IidmBridgeRegistry.java
│
├── test/
│   ├── CMakeLists.txt
│   ├── TestGraalVMBackend.cpp
│   ├── TestJNIBackend.cpp
│   ├── TestNetwork.cpp
│   ├── TestGenerator.cpp
│   └── resources/
│       └── ieee14.xiidm
│
├── cmake/
│   ├── FindGraalVM.cmake
│   ├── FindJNI.cmake             # or use CMake's built-in FindJNI
│   └── IidmBridgeConfig.cmake.in
│
└── examples/
    ├── standalone/
    │   └── main.cpp              # Standalone mode example
    └── embedded/
        ├── JavaLauncher.java     # Starts JVM, registers network, calls Dynawo
        └── DynawoEmbedded.cpp   # C++ side of embedded example
```

---

## 11. Build System (CMake)

### Top-level CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(iidm-bridge-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(IIDM_BRIDGE_ENABLE_GRAALVM "Build the GraalVM backend" ON)
option(IIDM_BRIDGE_ENABLE_JNI     "Build the JNI backend" ON)
option(IIDM_BRIDGE_BUILD_TESTS    "Build unit tests" ON)
option(IIDM_BRIDGE_BUILD_EXAMPLES "Build examples" ON)

# ── JNI (always needed for AUTO mode probe) ─────────────────────────────────
find_package(JNI REQUIRED)

# ── GraalVM ─────────────────────────────────────────────────────────────────
if(IIDM_BRIDGE_ENABLE_GRAALVM)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    find_package(GraalVM REQUIRED)    # sets GRAALVM_INCLUDE_DIRS
endif()

# ── Library sources ─────────────────────────────────────────────────────────
set(IIDM_BRIDGE_SOURCES
    src/Network.cpp
    src/NetworkFactory.cpp
    src/BackendSelector.cpp
    src/Generator.cpp
    src/Load.cpp
    src/Terminal.cpp
    src/Bus.cpp
    src/Substation.cpp
    src/VoltageLevel.cpp
    src/Line.cpp
    src/TwoWindingsTransformer.cpp
    src/HvdcLine.cpp
    # …
)

if(IIDM_BRIDGE_ENABLE_GRAALVM)
    list(APPEND IIDM_BRIDGE_SOURCES
        src/graalvm/GraalVMBackend.cpp)
endif()

if(IIDM_BRIDGE_ENABLE_JNI)
    list(APPEND IIDM_BRIDGE_SOURCES
        src/jni/JNIBackend.cpp)
endif()

add_library(iidmbridge SHARED ${IIDM_BRIDGE_SOURCES})

target_include_directories(iidmbridge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${JNI_INCLUDE_DIRS}
        $<$<BOOL:${IIDM_BRIDGE_ENABLE_GRAALVM}>:${GRAALVM_INCLUDE_DIRS}>
)

target_link_libraries(iidmbridge
    PRIVATE
        ${JNI_LIBRARIES}
        ${CMAKE_DL_LIBS}    # for dlopen
)

target_compile_definitions(iidmbridge PRIVATE
    $<$<BOOL:${IIDM_BRIDGE_ENABLE_GRAALVM}>:IIDM_BRIDGE_GRAALVM_ENABLED>
    $<$<BOOL:${IIDM_BRIDGE_ENABLE_JNI}>:IIDM_BRIDGE_JNI_ENABLED>
)

# ── Installation & CMake config ──────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS iidmbridge EXPORT IidmBridgeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT IidmBridgeTargets
    FILE IidmBridgeTargets.cmake
    NAMESPACE IidmBridge::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/IidmBridge
)

# ── Tests ───────────────────────────────────────────────────────────────────
if(IIDM_BRIDGE_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    add_subdirectory(test)
endif()
```

### Consuming from Dynawo

```cmake
# In Dynawo's CMakeLists.txt
find_package(IidmBridge REQUIRED)

target_link_libraries(dynawo_core
    PRIVATE
        IidmBridge::iidmbridge
)
```

---

## 12. Integration with Dynawo

### Replacing iidm4cpp

The library provides the same conceptual objects that Dynawo already uses
from `iidm4cpp`. Migration steps:

1. Replace the `powsybl::iidm::Network` include with `iidm::Network` from
   `<iidm/Network.h>`.
2. Replace `powsybl::iidm::Generator` with `iidm::Generator`, etc.
3. Replace direct field/member access (which iidm4cpp allowed) with the
   getter/setter calls that were already the idiomatic usage.
4. Replace the network loading call:
   ```cpp
   // Before (iidm4cpp)
   powsybl::iidm::Network network = powsybl::iidm::NetworkSerDe::read(path);

   // After (iidm-bridge-cpp)
   iidm::Network network = iidm::NetworkFactory::load(path);
   ```
5. Remove the `iidm4cpp` CMake dependency; add `IidmBridge`.

### Standalone mode (default)

```
dynawo --network case.xiidm --jobs jobs.xml
         │
         └── NetworkFactory::load("case.xiidm")
               └── BackendSelector → GraalVMBackend
                     └── dlopen("libpowsybl-iidm-native.so")
                           └── iidm_load_network("case.xiidm")
```

The `libpowsybl-iidm-native.so` must be on `LD_LIBRARY_PATH` or its path
provided via the `IIDM_BRIDGE_GRAALVM_LIB` environment variable.

### Embedded mode

```java
// Java orchestrator
Network network = NetworkSerDe.read(Paths.get("case.xiidm"));
IidmBridgeRegistry.register("case1", network);

DynawoRunner runner = new DynawoRunner();
runner.run(simulationParams);   // triggers Dynawo C++ main
```

```cpp
// C++ main (called from JNI by DynawoRunner)
iidm::NetworkLoadOptions opts;
opts.mode = iidm::BackendMode::JNI;
opts.jniNetworkId = "case1";
iidm::Network network = iidm::NetworkFactory::wrap("case1", opts);
// From here on, all generator.getTargetP() calls go directly to the
// Java Network object that is already in memory.
```

---

## 13. Testing Strategy

### Unit tests (C++ / GoogleTest)

Each component class has a dedicated test file. Tests run against a small
embedded XIIDM test network (`ieee14.xiidm`). Both backends are tested
independently.

```
TestGraalVMBackend   — backend protocol tests (load, getDouble, setDouble, …)
TestJNIBackend       — same protocol, against a JVM launched in-process
TestNetwork          — Network traversal, getGenerator by id, …
TestGenerator        — all getter/setter round-trips
TestLoad             — P0/Q0 round-trips
TestLine             — R/X/B1/B2 round-trips
TestTerminal         — P/Q/connected state
TestBus              — V/angle round-trips
TestBackendSelector  — AUTO mode picks JNI when JVM present, GraalVM otherwise
```

### Integration tests

A full integration test loads the IEEE-14 bus network, reads and modifies
generator target P values, and verifies round-trips match the original Java
results. This test is run with both backends.

### GraalVM / JNI conformance

`test/TestBackendConformance.cpp` makes sure the two backends cannot drift apart. It loads each
test network (`examples/*.xiidm`, `test/data/*.xiidm`) into both backends in the same process
and walks every object reachable from the network root in lockstep. For every object, every
code of `PropertyCodes.def` and every accessor, both backends must return the same value, the
same children / related objects (visited in turn) or the same C++ exception type. Keyed and
indexed lookups and `findById` are tried with every id and node number met on the way, keyed
strings with the object's property names and aliases (indexed lookups also with its int values,
such as tap positions, and the sides), `evaluate` with a fixed set of argument lists; an
identifiable looked up again by id must be the same object (`isSameObject`). Every value read is then written back through its
setter (same acceptance, same value read back), properties and aliases are removed and added
back, terminals and connectables disconnected and connected, and both networks must save to the
same XIIDM. One network is built through the C++ adders, with properties, aliases and names out
of ASCII (it found that JNI's modified UTF-8 corrupted characters outside the BMP). New codes and
new test data are picked up automatically. It runs when both `IIDM_BRIDGE_GRAALVM_LIB` and
`IIDM_BRIDGE_JAR` are set (CI `graalvm` and `dynawo` jobs), in 2 to 4 s per network.

### Property codes

There is nothing to keep in sync: `PropertyCodes.java` is generated by the Maven
build (`generate-sources`, see `java/pom.xml`) from `include/iidm/PropertyCodes.def`,
the same file `PropertyCodes.h` includes.

### Dynawo regression tests

Existing Dynawo test cases that previously used `iidm4cpp` are re-run with
`iidm-bridge-cpp` in GraalVM mode. Pass/fail results are compared against
the `iidm4cpp` baseline to validate the migration.

---

## 14. Comparison with libiidm4cpp and pypowsybl

| Aspect | libiidm4cpp | pypowsybl (cpp layer) | **iidm-bridge-cpp** |
|---|---|---|---|
| Language | C++ | C++ + Python | C++ |
| Java delegation | None | Full (via GraalVM) | Full (GraalVM or JNI) |
| API style | Full object model | Pandas DataFrames | Object model with getters/setters |
| Embedded JVM support | No | No | Yes (JNI backend) |
| Maintenance burden | High (full reimplementation) | Low | Low |
| Network creation | Yes | Yes | Yes (powsybl adders, §4.5) |
| Topology access | Yes | Via DataFrames | Yes (read-only) |
| Extensions | Partially | Via DataFrames | Delegated to Java |
| Use case | General C++ IIDM consumer | Data science / Python | Dynawo simulation core |

---

## 15. Roadmap

### Phase 1 — Minimal viable library (covers Dynawo's current usage)

- [ ] `BackendProvider` interface and `PropertyCodes`
- [ ] `Network`, `Substation`, `VoltageLevel`, `Bus`
- [ ] `Generator`, `Load`, `Terminal`
- [ ] `Line`, `TwoWindingsTransformer`
- [ ] `GraalVMBackend` with `dlopen`
- [ ] `JNIBackend` with method-ID caching
- [ ] `BackendSelector` (AUTO mode)
- [ ] CMake build + install
- [ ] GoogleTest suite
- [ ] Java module: `PropertyDispatcher`, `NetworkRegistry`, `IidmBridgeRegistry`
- [ ] GraalVM `native-image` build script for `libpowsybl-iidm-native.so`

### Phase 2 — Extended component coverage

- [ ] `ThreeWindingsTransformer`
- [ ] `HvdcLine`, `VscConverterStation`, `LccConverterStation`
- [ ] `DanglingLine`
- [ ] `ShuntCompensator`, `StaticVarCompensator`
- [ ] `Switch` (bus-breaker and node-breaker views)
- [ ] `ConnectedComponent` and `SynchronousComponent`

### Phase 3 — Production hardening

- [ ] Multi-thread isolate management for GraalVM
- [ ] JNI class/method ID refresh on hot-reload scenarios
- [ ] XIIDM extension passthrough (ActivePowerControl, etc.) via
      generic `getExtension<T>()` returning a property map
- [ ] Performance benchmarks (GraalVM vs JNI round-trip latency)
- [ ] CI pipeline (GitHub Actions): build + test on Linux/macOS,
      GraalVM 21+ and JDK 17+

### Phase 4 — Dynawo migration

- [ ] Drop-in CMake shim providing the `powsybl::iidm::*` namespace
      aliases pointing to `iidm::*` (to ease incremental migration)
- [ ] Full Dynawo regression test suite passing with `iidm-bridge-cpp`
- [ ] Remove `libiidm4cpp` from Dynawo's dependency list
