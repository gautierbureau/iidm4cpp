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
| Expose getters and setters only | Dynawo does not need topology mutation or network creation |
| Single C++ API surface for both modes | Dynawo code has zero `#ifdef STANDALONE` branches |
| Delegate all business logic to Java | Correctness, extensions, and future IIDM revisions are free |
| Zero-copy in embedded mode | The JVM already owns the data; the C++ layer only borrows references |
| Lightweight compile footprint | The library itself has no heavy dependencies beyond the JNI headers and (optionally) the GraalVM SDK headers |
| CMake-compatible | Drop-in replacement for the `iidm4cpp` CMake target in Dynawo |

### Non-Goals

- **Network creation or topology modification** from C++. Dynawo is a
  consumer, not a builder.
- **A full IIDM reimplementation**. That is explicitly what we are moving
  away from.
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

The public object types mirror the IIDM object model. Each class is a
**thin value type** that wraps an opaque handle. Objects are obtained from
a `Network` instance or from other objects via navigation methods. They are
cheap to copy (handle copy only) and their lifetime is bounded by the
`Network` that produced them.

```
Network
├── getSubstations() → std::vector<Substation>
├── getVoltageLevels() → std::vector<VoltageLevel>
├── getGenerators() → std::vector<Generator>
├── getLoads() → std::vector<Load>
├── getLines() → std::vector<Line>
├── getTwoWindingsTransformers() → std::vector<TwoWindingsTransformer>
├── getThreeWindingsTransformers() → std::vector<ThreeWindingsTransformer>
├── getHvdcLines() → std::vector<HvdcLine>
├── getDanglingLines() → std::vector<DanglingLine>
├── getShuntCompensators() → std::vector<ShuntCompensator>
├── getStaticVarCompensators() → std::vector<StaticVarCompensator>
├── getVscConverterStations() → std::vector<VscConverterStation>
└── getLccConverterStations() → std::vector<LccConverterStation>

Substation
├── getId() → std::string
├── getName() → std::string
├── getCountry() → std::optional<Country>
└── getVoltageLevels() → std::vector<VoltageLevel>

VoltageLevel
├── getId() / getName()
├── getNominalV() → double
├── getLowVoltageLimit() → double
├── getHighVoltageLimit() → double
├── getTopologyKind() → TopologyKind
└── getBusBreakerView() / getNodeBreakerView()

Generator
├── getId() / getName()
├── getTargetP() → double        setTargetP(double)
├── getTargetQ() → double        setTargetQ(double)
├── getTargetV() → double        setTargetV(double)
├── getMinP() → double
├── getMaxP() → double
├── isVoltageRegulatorOn() → bool  setVoltageRegulatorOn(bool)
├── getTerminal() → Terminal
└── getEnergySource() → EnergySource

Load
├── getId() / getName()
├── getP0() → double             setP0(double)
├── getQ0() → double             setQ0(double)
├── getLoadType() → LoadType
└── getTerminal() → Terminal

Line / TwoWindingsTransformer / HvdcLine / …
(similar getter/setter patterns for their specific attributes)

Terminal
├── getBusView() → BusView
├── getP() → double              setP(double)
├── getQ() → double              setQ(double)
├── isConnected() → bool
└── connect() / disconnect()

Bus (from BusView or BusBreakerView)
├── getId() → std::string
├── getV() → double              setV(double)
├── getAngle() → double          setAngle(double)
└── getConnectedComponent() → ConnectedComponent
```

### 4.2 Handle Model

Every network object holds an opaque `ObjectHandle`:

```cpp
// include/iidm/ObjectHandle.h
namespace iidm {

// An ObjectHandle is an opaque integer that uniquely identifies a Java
// object within a backend session.  It is valid for the lifetime of the
// Network that produced it.
using ObjectHandle = std::int64_t;
constexpr ObjectHandle INVALID_HANDLE = 0;

} // namespace iidm
```

All component classes follow the same pattern:

```cpp
// include/iidm/Generator.h
namespace iidm {

class Generator {
public:
    // Constructors / destructor managed by the library
    Generator() = default;
    explicit Generator(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getTargetP() const;
    Generator& setTargetP(double targetP);

    double getTargetQ() const;
    Generator& setTargetQ(double targetQ);

    double getTargetV() const;
    Generator& setTargetV(double targetV);

    double getMinP() const;
    double getMaxP() const;

    bool isVoltageRegulatorOn() const;
    Generator& setVoltageRegulatorOn(bool on);

    EnergySource getEnergySource() const;

    Terminal getTerminal() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    // Equality is handle-based, not value-based
    bool operator==(const Generator& other) const;

private:
    ObjectHandle handle_ = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;  // non-owning pointer, owned by Network
};

} // namespace iidm
```

The setter methods return `Generator&` to support method chaining:

```cpp
gen.setTargetP(200.0).setTargetQ(50.0).setVoltageRegulatorOn(true);
```

### 4.3 Network & NetworkFactory

```cpp
// include/iidm/Network.h
namespace iidm {

class Network {
public:
    ~Network();
    Network(Network&&) noexcept;
    Network& operator=(Network&&) noexcept;

    // No copy: a Network owns its backend session
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    std::string getId() const;
    std::string getName() const;

    // Iteration helpers
    std::vector<Substation>              getSubstations() const;
    std::vector<VoltageLevel>            getVoltageLevels() const;
    std::vector<Generator>               getGenerators() const;
    std::vector<Load>                    getLoads() const;
    std::vector<Line>                    getLines() const;
    std::vector<TwoWindingsTransformer>  getTwoWindingsTransformers() const;
    std::vector<ThreeWindingsTransformer> getThreeWindingsTransformers() const;
    std::vector<HvdcLine>                getHvdcLines() const;
    std::vector<ShuntCompensator>        getShuntCompensators() const;
    std::vector<StaticVarCompensator>    getStaticVarCompensators() const;
    std::vector<DanglingLine>            getDanglingLines() const;
    std::vector<VscConverterStation>     getVscConverterStations() const;
    std::vector<LccConverterStation>     getLccConverterStations() const;

    // Lookup by ID
    std::optional<Generator>               getGenerator(const std::string& id) const;
    std::optional<Load>                    getLoad(const std::string& id) const;
    std::optional<Line>                    getLine(const std::string& id) const;
    std::optional<TwoWindingsTransformer>  getTwoWindingsTransformer(const std::string& id) const;
    // … and so on for each type

    // Backend accessor (advanced usage only)
    BackendProvider& getBackend();

private:
    friend class NetworkFactory;
    explicit Network(std::unique_ptr<BackendProvider> backend);
    std::unique_ptr<BackendProvider> backend_;
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
enum class StaticVarCompensatorRegulationMode { VOLTAGE, REACTIVE_POWER, OFF };
enum class SwitchKind { BREAKER, DISCONNECTOR, LOAD_BREAK_SWITCH };

} // namespace iidm
```

Optional doubles use `std::optional<double>` to represent undefined or NaN
values rather than sentinel values, providing a cleaner API than the Java
`Double.isNaN()` idiom.

---

## 5. Backend Abstraction Layer

### 5.1 BackendProvider Interface

```cpp
// include/iidm/BackendProvider.h
namespace iidm {

// The complete set of primitive operations any backend must provide.
// Component classes call exclusively into this interface; they have no
// backend-specific code.
class BackendProvider {
public:
    virtual ~BackendProvider() = default;

    // ── Scalar property getters ──────────────────────────────────────────
    virtual double      getDouble(ObjectHandle h, int property) const = 0;
    virtual int         getInt(ObjectHandle h, int property) const = 0;
    virtual bool        getBool(ObjectHandle h, int property) const = 0;
    virtual std::string getString(ObjectHandle h, int property) const = 0;

    // ── Scalar property setters ──────────────────────────────────────────
    virtual void setDouble(ObjectHandle h, int property, double value) = 0;
    virtual void setInt   (ObjectHandle h, int property, int value) = 0;
    virtual void setBool  (ObjectHandle h, int property, bool value) = 0;
    virtual void setString(ObjectHandle h, int property, const std::string& value) = 0;

    // ── Optional scalars (returns std::nullopt when undefined/NaN) ───────
    virtual std::optional<double> getOptDouble(ObjectHandle h, int property) const = 0;

    // ── Navigation (children, terminals, etc.) ───────────────────────────
    virtual std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const = 0;
    virtual ObjectHandle              getRelated(ObjectHandle h, int relation) const = 0;

    // ── Lookup by ID ─────────────────────────────────────────────────────
    virtual ObjectHandle findById(int objectType, const std::string& id) const = 0;

    // ── Network root ─────────────────────────────────────────────────────
    virtual ObjectHandle getNetworkHandle() const = 0;

    // ── Lifecycle ────────────────────────────────────────────────────────
    virtual void loadNetwork(const std::string& filePath) = 0;
    virtual void close() = 0;
};

} // namespace iidm
```

Properties and object types are encoded as `int` constants defined in a
shared header `include/iidm/PropertyCodes.h` so that both backends and the
Java side refer to the same numbers:

```cpp
// include/iidm/PropertyCodes.h
namespace iidm::prop {

// Object types (used in getChildren / findById)
constexpr int SUBSTATION               = 1;
constexpr int VOLTAGE_LEVEL            = 2;
constexpr int GENERATOR                = 3;
constexpr int LOAD                     = 4;
constexpr int LINE                     = 5;
constexpr int TWO_WINDINGS_TRANSFORMER = 6;
// …

// Shared properties (applicable to many types)
constexpr int ID                       = 100;
constexpr int NAME                     = 101;

// Generator-specific
constexpr int GEN_TARGET_P             = 200;
constexpr int GEN_TARGET_Q             = 201;
constexpr int GEN_TARGET_V             = 202;
constexpr int GEN_MIN_P                = 203;
constexpr int GEN_MAX_P                = 204;
constexpr int GEN_VOLTAGE_REGULATOR_ON = 205;
constexpr int GEN_ENERGY_SOURCE        = 206;

// Load-specific
constexpr int LOAD_P0                  = 300;
constexpr int LOAD_Q0                  = 301;
constexpr int LOAD_TYPE                = 302;

// Terminal
constexpr int TERMINAL_P               = 400;
constexpr int TERMINAL_Q               = 401;
constexpr int TERMINAL_CONNECTED       = 402;
constexpr int TERMINAL_BUS_ID         = 403;

// Bus
constexpr int BUS_V                    = 500;
constexpr int BUS_ANGLE                = 501;

// VoltageLevel
constexpr int VL_NOMINAL_V             = 600;
constexpr int VL_LOW_VOLTAGE_LIMIT     = 601;
constexpr int VL_HIGH_VOLTAGE_LIMIT    = 602;
constexpr int VL_TOPOLOGY_KIND         = 603;

// Line
constexpr int LINE_R                   = 700;
constexpr int LINE_X                   = 701;
constexpr int LINE_G1                  = 702;
constexpr int LINE_B1                  = 703;
constexpr int LINE_G2                  = 704;
constexpr int LINE_B2                  = 705;
// … and so on

} // namespace iidm::prop
```

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

#### String ownership convention

When the native image returns a `const char*` from a string getter, the
string is allocated in the isolate heap. The C++ backend copies it into a
`std::string` immediately and then calls a corresponding
`iidm_free_string(thread_, ptr)` exported function to release the isolate
memory. **No raw pointers escape the backend layer.**

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

#### Method ID caching

JNI method lookups (`GetMethodID`) are expensive. The backend caches all
`jmethodID` and `jfieldID` values it will ever need at construction time,
resolving them once against the Java classes in the live JVM.

```cpp
// src/jni/JNIMethodCache.h
struct JNIMethodCache {
    // Classes
    jclass networkClass;
    jclass generatorClass;
    jclass loadClass;
    jclass terminalClass;
    jclass busClass;
    // …

    // Method IDs
    jmethodID network_getGenerators;
    jmethodID network_getLoads;
    jmethodID generator_getTargetP;
    jmethodID generator_setTargetP;
    jmethodID generator_getTargetQ;
    jmethodID generator_setTargetQ;
    jmethodID generator_getTargetV;
    jmethodID generator_setTargetV;
    jmethodID generator_isVoltageRegulatorOn;
    jmethodID generator_setVoltageRegulatorOn;
    // … all getters and setters for all supported types
};
```

#### Handle = global JNI reference

In the JNI backend, an `ObjectHandle` is the integer value of a `jobject`
**global reference**. The backend casts between `ObjectHandle` and `jobject`
with `reinterpret_cast`. Global references prevent GC collection of the
Java objects as long as the `Network` is alive.

```cpp
// src/jni/JNIBackend.cpp (excerpt)

double JNIBackend::getDouble(ObjectHandle h, int property) const {
    auto obj = reinterpret_cast<jobject>(h);
    switch (property) {
        case prop::GEN_TARGET_P:
            return env_->CallDoubleMethod(obj, cache_.generator_getTargetP);
        case prop::GEN_TARGET_Q:
            return env_->CallDoubleMethod(obj, cache_.generator_getTargetQ);
        // …
        default:
            throw IidmException("Unknown double property: " + std::to_string(property));
    }
    checkJNIException();
}

void JNIBackend::setDouble(ObjectHandle h, int property, double value) {
    auto obj = reinterpret_cast<jobject>(h);
    switch (property) {
        case prop::GEN_TARGET_P:
            env_->CallVoidMethod(obj, cache_.generator_setTargetP, value);
            break;
        // …
    }
    checkJNIException();
}
```

#### Exception bridging

JNI does not propagate Java exceptions automatically. After every JNI call
the backend checks `ExceptionOccurred()` and, if set, clears it and throws
a C++ `IidmException` containing the Java exception's message.

### 5.4 Extensions

IIDM extensions are optional property bags that can be attached to network
components (e.g. `ActivePowerControl` on `Generator`). The bridge exposes
them through the same `BackendProvider` property-code mechanism as core
properties, but the two backends handle the dispatch differently.

#### C++ surface

An extension is a thin wrapper class that holds the same `ObjectHandle` as
its host component and routes calls through `EXT_*` property codes:

```cpp
// include/iidm/ActivePowerControl.h
class ActivePowerControl {
public:
    explicit ActivePowerControl(ObjectHandle handle, BackendProvider* backend);

    bool   isParticipate() const;                     // EXT_APC_PARTICIPATE
    ActivePowerControl& setParticipate(bool v);
    double getDroop() const;                          // EXT_APC_DROOP
    ActivePowerControl& setDroop(double v);

    bool isValid() const { return handle_ != INVALID_HANDLE; }
private:
    ObjectHandle   handle_;
    BackendProvider* backend_;
};

// include/iidm/Generator.h (additions)
bool             hasActivePowerControl() const;  // EXT_APC_EXISTS
ActivePowerControl getActivePowerControl() const;
```

Property codes for extensions occupy a dedicated range in `PropertyCodes.h`
(and the mirrored `PropertyCodes.java`):

```cpp
constexpr int EXT_APC_EXISTS      = 3000;  // bool: is extension present?
constexpr int EXT_APC_DROOP       = 3001;  // double getter/setter
constexpr int EXT_APC_PARTICIPATE = 3002;  // bool getter/setter
```

#### GraalVM backend — automatic

The GraalVM entry points (`iidm_get_double`, `iidm_get_bool`, …) forward
**every** property code to `PropertyDispatcher.java` without a hard-coded
switch. Adding an extension only requires new cases in `PropertyDispatcher`:

```java
// PropertyDispatcher.java
case EXT_APC_DROOP    -> ((Generator) obj).getExtension(ActivePowerControl.class).getDroop();
case EXT_APC_EXISTS   -> ((Generator) obj).getExtension(ActivePowerControl.class) != null;
case EXT_APC_PARTICIPATE -> ((Generator) obj).getExtension(ActivePowerControl.class).isParticipate();
```

No C++ changes are needed in the GraalVM backend.

#### JNI backend — explicit wiring

The JNI backend calls Java methods directly and must explicitly handle every
property code. Each extension requires work in two files:

**`src/jni/JNIMethodCache.h`** — cache the extension interface class and its
method IDs:

```cpp
jclass    activePowerControlClass      = nullptr;
jmethodID generator_getExtensionByName = nullptr;
jmethodID apc_getDroop                 = nullptr;
jmethodID apc_setDroop                 = nullptr;
jmethodID apc_isParticipate            = nullptr;
jmethodID apc_setParticipate           = nullptr;
```

**`src/jni/JNIBackend.cpp`** — initialise the cache, add a helper, and add
switch cases:

```cpp
// In cacheMethodIds():
cacheClass(cache_.activePowerControlClass,
    "com/powsybl/iidm/network/extensions/ActivePowerControl");
cache_.generator_getExtensionByName = env_->GetMethodID(cache_.generatorClass,
    "getExtensionByName",
    "(Ljava/lang/String;)Lcom/powsybl/commons/extensions/Extension;");
cache_.apc_getDroop      = env_->GetMethodID(cache_.activePowerControlClass, "getDroop",      "()D");
cache_.apc_setDroop      = env_->GetMethodID(cache_.activePowerControlClass, "setDroop",      "(D)V");
cache_.apc_isParticipate = env_->GetMethodID(cache_.activePowerControlClass, "isParticipate", "()Z");
cache_.apc_setParticipate= env_->GetMethodID(cache_.activePowerControlClass, "setParticipate","(Z)V");

// Private helper (returns a local JNI ref; caller must DeleteLocalRef):
jobject JNIBackend::fetchApcExtension(jobject gen) const {
    jstring name = env_->NewStringUTF("activePowerControl");
    jobject ext  = env_->CallObjectMethod(gen, cache_.generator_getExtensionByName, name);
    env_->DeleteLocalRef(name);
    checkJNIException(env_);
    return ext;
}

// In getBool():
case prop::EXT_APC_EXISTS: {
    jobject apc = fetchApcExtension(obj);
    result = (apc != nullptr) ? JNI_TRUE : JNI_FALSE;
    if (apc) env_->DeleteLocalRef(apc);
    break;
}
case prop::EXT_APC_PARTICIPATE: {
    jobject apc = fetchApcExtension(obj);
    if (!apc) throw PropertyNotFoundException("ActivePowerControl extension not present");
    result = env_->CallBooleanMethod(apc, cache_.apc_isParticipate);
    env_->DeleteLocalRef(apc);
    break;
}
```

> **Key asymmetry**: because the GraalVM backend routes all codes through
> `PropertyDispatcher`, a new extension only needs Java-side changes for that
> path. The JNI backend must also receive explicit JNI method lookups and
> switch cases — omitting this causes `PropertyNotFoundException` at runtime
> with the JNI backend while the GraalVM backend continues to work.

#### Checklist for adding a new extension

| Step | File(s) | GraalVM | JNI |
|---|---|---|---|
| `EXT_*` property constants | `PropertyCodes.h` + `PropertyCodes.java` | required | required |
| C++ extension wrapper | `include/iidm/MyExt.h` + `src/MyExt.cpp` | required | required |
| Host component accessors | e.g. `Generator.h` / `Generator.cpp` | required | required |
| `PropertyDispatcher` cases | `PropertyDispatcher.java` | required | not needed |
| Method-ID cache fields | `JNIMethodCache.h` | — | required |
| Cache init + switch cases | `JNIBackend.cpp` | — | required |
| Unit tests | `test/TestMyExt.cpp` | required | required |

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
            case PropertyCodes.GEN_TARGET_Q -> ((Generator) obj).getTargetQ();
            case PropertyCodes.GEN_TARGET_V -> ((Generator) obj).getTargetV();
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
            case PropertyCodes.GEN_TARGET_Q -> ((Generator) obj).setTargetQ(value);
            case PropertyCodes.GEN_TARGET_V -> ((Generator) obj).setTargetV(value);
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
| Component objects (e.g. `Generator`) | Caller (value type) | Valid as long as parent `Network` is alive |
| GraalVM isolate | `GraalVMBackend` | One per process; destroyed in destructor |
| JNI global refs | `JNIBackend` | Deleted in destructor via `DeleteGlobalRef` |
| JNI thread attachment | `JNIBackend` | Detached in destructor via `DetachCurrentThread` |

### Invalidation

Calling any method on a component object after the parent `Network` has
been destroyed is **undefined behavior**. In debug builds, a validity
check is performed and an `IidmException` is thrown.

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

All errors are reported via exceptions. The hierarchy is:

```cpp
// include/iidm/IidmException.h
namespace iidm {

class IidmException : public std::runtime_error {
public:
    explicit IidmException(const std::string& message)
        : std::runtime_error(message) {}
};

class BackendNotAvailableException : public IidmException { /* … */ };
class InvalidHandleException       : public IidmException { /* … */ };
class PropertyNotFoundException    : public IidmException { /* … */ };
class NetworkNotFoundException     : public IidmException { /* … */ };

} // namespace iidm
```

Java exceptions thrown within GraalVM entry points are caught in Java,
converted to an error code (integer), stored in a thread-local on the
isolate, and translated to a C++ exception on the C++ side. This keeps the
C ABI clean (no `setjmp`/`longjmp` across the boundary).

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
│       ├── PropertyCodes.h
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
│       ├── PropertyCodes.java    # Must stay in sync with PropertyCodes.h
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

### Property code consistency test

An automated test (run as part of the Java build) parses `PropertyCodes.h`
and verifies that every constant has a corresponding entry in
`PropertyCodes.java` with the same integer value, preventing silent
desynchronisation.

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
| Network creation | Yes | Yes | No (by design) |
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
