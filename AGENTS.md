# AGENTS.md — iidm4cpp

AI agent guidance for the `iidm4cpp` / `iidm-bridge-cpp` project.
Written in progressive disclosure order: read as far as your task requires.

---

## Level 1 — Orientation (read this always)

### What this project is

A C++ bridge library that gives application code (e.g. Dynawo) object-oriented
getter/setter access to PowSyBl IIDM power-network models.  All data lives in
Java; the C++ side holds opaque handles and dispatches every property read/write
through one of two backends:

- **GraalVM** — Java compiled to a native shared library (`libpowsybl-iidm-native.so`)
- **JNI** — Java running in an embedded JVM

The public surface is identical regardless of backend. `BackendSelector` can
pick one automatically at runtime (`AUTO` mode).

### Quick build (JNI only, the default CI path)

```bash
cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DIIDM_BRIDGE_ENABLE_JNI=ON \
      -DIIDM_BRIDGE_ENABLE_GRAALVM=OFF \
      -DIIDM_BRIDGE_BUILD_JAVA=OFF \
      -DIIDM_BRIDGE_BUILD_NATIVE=OFF \
      -DIIDM_BRIDGE_BUILD_TESTS=ON
cmake --build build --parallel
```

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

Unit tests use `MockBackend` and need no external files. The accessor table
(`TestAccessors`), the conformance and the integration tests need the real backends, through
environment variables — see Level 3 (they are skipped without them).

### Java module

```bash
cd java && mvn clean verify
```

---

## Level 2 — Conventions (read before writing any code)

### Namespace & naming

| Thing | Convention | Example |
|---|---|---|
| Namespace | `iidm` | `namespace iidm { ... }` |
| Classes | `PascalCase` | `Generator`, `NetworkFactory` |
| Methods | `camelCase` | `getTargetP()`, `setTargetP()` |
| Private members | `camelCase_` (trailing `_`) | `handle_`, `isolateId_` |
| Enum values | `UPPER_SNAKE_CASE` | `EnergySource::NUCLEAR` |

### Key patterns

**Handle model** — every component is a handle on a Java object plus a `BackendProvider*`,
held by the root class `detail::ObjectRef`. It never copies network data into C++ memory.

```cpp
// Typical component header shape
class Load : public Injection {               // the powsybl interface it extends
public:
    static constexpr int TYPE = prop::LOAD;
    Load() = default;
    explicit Load(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    // A property is one inline line: the ObjectRef helpers getDouble/getInt/getBool/getString,
    // getOptDouble/getOptInt/getOptString, and setDouble/setInt/... which return the wrapper
    double getP0() const { return getDouble(prop::LOAD_P0); }
    Load& setP0(double p0) { return setDouble(*this, prop::LOAD_P0, p0); }
    LoadType getLoadType() const { return static_cast<LoadType>(getInt(prop::LOAD_TYPE)); }
};
```

Only methods that do more than forward one property (lists, related objects, adders, anything
returning another wrapper) are defined in `src/X.cpp`.

**Class model** — the C++ classes follow the powsybl interfaces:

- `Identifiable` → `Connectable` → `Injection` → `HvdcConverterStation` are base classes; a class
  derives from the one its powsybl interface extends, and gets its methods (id, names, properties,
  aliases, terminals, connect/disconnect, remove, the extensions of that host type...). Never
  re-declare a base-class method in a derived class.
- A powsybl interface that unrelated types share (`Branch`: lines and transformers, but also tie
  lines, which are not connectables; `VoltageRegulationHolder`: generators, batteries, SVCs,
  shunts, VSC stations, ratio tap changers; `ReactiveLimitsHolder`; `TapChanger`, `TapChangerStep`,
  the tap changer holders, `FlowsLimitsHolder`: legs and boundary lines) is a CRTP mixin
  `detail::XMethods<Self>`, defined in `src/X.cpp` and explicitly instantiated there for each
  class that uses it; plus a class `X` for results of that type when there are some, which every
  implementing class converts to (see `Branch.h`). Two mixins with the same method: a `using`
  picks one (`RatioTapChanger`).
- Sub-objects are objects with their own handle, as in powsybl (a 3WT leg, a tap changer, a step),
  reached from their parent by a relation (`getRelated(holder, RATIO_TAP_CHANGER)`), children
  (`getChildren(transformer, THREE_WINDINGS_TRANSFORMER_LEG)`) or an indexed relation (a step by
  tap position, a leg by side). Never encode the parent's side or kind in property codes.
- Every class of a powsybl type has `static constexpr int TYPE = prop::<ITS_TYPE_CODE>;`:
  `is<T>()` / `as<T>()` (on any wrapper, answered by Java `instanceof` in `getBool`) and the
  network lists use it.
- `==` is the identity of the Java object (`isSameObject`), never a handle comparison.

**Property codes** — C++/Java communicate via integer keys defined **once** in
`include/iidm/PropertyCodes.def` (`IIDM_CODE(NAME, value)  // comment`).
`PropertyCodes.h` expands it into `iidm::prop::NAME`; the Maven build generates
the Java `PropertyCodes` class from it. There is no second copy to keep in sync. A backend opens
only on a jar or native image built from the same codes: both sides hash them
(`src/PropertyCodesCheck.cpp`, `PropertyDispatcher.codesHash`), and the backend compares the hashes.

**Fluent setters** — setters return `*this` so callers can chain:

```cpp
gen.setTargetP(100.0).setTargetQ(0.0);
```

**Optional values** — values that may be absent are `iidm::optional<T>`
(`include/iidm/stdcxx/optional.hpp`): `std::optional` in C++17, `boost::optional` in C++11/14.
Never use `std::optional` directly.

**Copies** — wrappers are copyable: a copy shares the reference-counted handle
(`HandleRef`) on the same Java object and copies no network data. Only `Network`,
which owns its backend, and the adders, which each drive one Java adder, are
move-only: keep their copy operations deleted.

**Stream / byte-buffer I/O** — `NetworkFactory::load(std::istream&)` and
`NetworkFactory::loadFromBytes(...)` parse a network from an in-memory buffer
instead of a file path; `Network::save(std::ostream&)` and
`Network::saveToBytes()` go the other way. Both go through the backend
methods `loadNetworkBytes` / `saveNetworkBytes`; `NetworkLoadOptions::filenameHint`
selects the format (default `"network.xiidm"`).

### File layout

```
include/iidm/   Public headers (one per component + iidm.h convenience header)
src/            Implementations; backend-specific code in graalvm/ and jni/
java/           Maven module for the Java bridge
test/           GoogleTest tests; MockBackend.h fakes the backend, CompleteNetwork.h builds a network
                with every type of object on a real one (for TestAccessors and TestBackendConformance)
examples/       Standalone (GraalVM) and embedded (JNI) usage examples
cmake/          CMake helper modules (FindGraalVM.cmake, etc.)
```

### Adding a new network component

1. Add a header under `include/iidm/MyComponent.h`: derive from the class of the powsybl interface
   it extends (`Injection`, `Connectable`, `Identifiable`; mixins for the others), declare
   `TYPE`, and only the methods of its own powsybl interface.
2. Its property accessors are inline one-liners in the header (see the shape above); the rest of its
   methods, if any, go in `src/MyComponent.cpp`.
3. Add a type code and its property codes to `include/iidm/PropertyCodes.def`: one code per method
   of a powsybl interface, named after that interface.
4. Expose the component through `Network` (or the appropriate parent) with a
   getter that returns by value or `std::vector`.
5. In `java/.../PropertyTable.java`: one line per property, with its getter, setter and adder field
   (`doubles(LOAD_P0, Load::getP0, Load::setP0, LoadAdder::setP0);`, `null` where there is none).
   In `PropertyDispatcher.java`: the cases of the properties that take more than one call (and in
   `Creation.java`, of such adder fields), its type code in `getBool` (instanceof), `childStream`
   and `findObjectById`.
6. Tests: each one-line accessor is a line of the table of `test/TestAccessors.cpp`, on an object of
   the network that `test/CompleteNetwork.h` builds (add one of the component there):
   `WRITE(load, getP0, setP0, 80.0, 81.0);` reads the value of the network, sets another and reads it
   back, on both backends. Only the methods with logic of their own (several calls, conversions,
   validation) get a test in `test/TestMyComponent.cpp` with `MockBackend`.

### Primitives for values that may be absent, lists and keys

- An optional value: `getOptInt` / `getOptString` (and `getOptDouble`, where NaN is no value).
  The plain getter of the same code gives a non-null value or fails (`getString(NAME)` is the
  name or else the id); never invent sentinels for ints (tap positions can be negative).
- A list of strings: `getStringList` (sets are sorted on the Java side, so that both backends
  give the same order).
- A list that a keyed or indexed lookup returns (the switches at a node:
  `getRelatedByIndex(vl, REL_NBV_SWITCH, node)`): an object, whose elements are
  `getChildren(list, ELEMENTS)`, or `getIntList(list, ELEMENTS)` for numbers; `detail::elementsOf<T>`
  and `detail::intElementsOf` (`src/Wrap.h`) read it and release it.
- Values under a key: `getKeyedString` / `setKeyedString` (an empty optional removes the key), as
  for properties (`PROPERTY`) and aliases (`ALIAS_TYPE`).
- An action: `invoke(object, ACT_...)`; it returns an object, or the object itself for a boolean
  result (`ACT_CONNECT`: when the terminal was connected by this call).
- A method that takes numeric arguments: `evaluate(object, code, args)`, with the code of the
  method's name (`evaluate(limits, RL_MIN_Q, {p})` is `getMinQ(p)`, `getDouble(limits, RL_MIN_Q)`
  the `getMinQ()` without argument); ints, bools and enum ordinals go as doubles, a bool result
  comes back as 1 / 0. In Java, check the number of arguments after the cast, so that a code of
  another type fails with a ClassCastException.

### Adding a new extension

IIDM extensions (e.g. `ActivePowerControl`) are optional property bags
attached to network components. Both backends dispatch them through
`PropertyDispatcher.java` — **read this section in full before touching
extension code**. The supported extensions are the types of `Extensions.java` (the README lists
them).

#### Steps common to both backends

An extension is an object of its own, as in powsybl: `ActivePowerControl` is the model to copy.

1. **Property codes** — a block in `include/iidm/PropertyCodes.def`. Its first code is the
   extension's type code; the others are its properties, which apply to the extension object.

   ```cpp
   // include/iidm/PropertyCodes.def
   IIDM_CODE(FOO,            9000)  // getRelated(host, FOO): the extension, INVALID_HANDLE when absent;
                                    // invoke(host, FOO): its adder; setRelated(host, FOO, INVALID_HANDLE): removes it
   IIDM_CODE(EXT_FOO_SOME_D, 9001)  // double, on the extension object
   IIDM_CODE(EXT_FOO_SOME_B, 9002)  // bool
   ```

2. **C++ classes** — `include/iidm/Foo.h` (the wrapper, holding the extension's handle, its property
   accessors inline; `src/Foo.cpp` only for its other methods) and `include/iidm/FooAdder.h` (`detail::AdderBase`, one line per field, `add()`).

3. **Nothing on the host** — the templates of `Identifiable` reach any extension by its class:
   `hasExtension<Foo>()`, `findExtension<Foo>()`, `getExtension<Foo>()`
   (`ExtensionNotFoundException` when absent), `newExtension<FooAdder>()` and
   `removeExtension<Foo>()`. They use `Foo::TYPE` (for the adder, the type its `add()` returns).

4. **Java** — the type is one line of `Extensions.java`, which serves the lookup on a host
   (`getRelatedObject`), the type check, the adder (`Creation.newAdder`) and the removal
   (`Creation.clear`); in `PropertyTable.java`, each property is one line: its getter and setter on
   the extension, and its field on the adder (a typed lambda where the type is generic, `null`
   where there is none):

   ```java
   // Extensions.java
   type(FOO, Foo.class, FooAdder.class),
   // PropertyTable.java
   doubles(EXT_FOO_SOME_D, (Foo<?> x) -> x.getSomeD(), (Foo<?> x, double v) -> x.setSomeD(v),
       (FooAdder<?> x, double v) -> x.withSomeD(v));
   ```

5. **Tests** — add the extension to an object of `test/CompleteNetwork.h` (`addEverything`) and a
   line per property to the table of `test/TestAccessors.cpp`; `test/TestFoo.cpp` with
   `MockBackend` only for its methods with logic of their own; a Java test of the dispatcher and
   creation cases (see `ExtensionDispatchTest`, `CreationDispatchTest`). `TestBackendConformance`
   then compares every new code, and the adder, on both backends without any change to the test
   (as it does for the extensions of the test networks, `examples/*.xiidm`).

An extension's Java code stays in `PropertyTable` and `PropertyDispatcher`, like every other: its
table lines, the cases of what takes more than one call, plus a helper at the end of
`PropertyDispatcher` when a case needs more than an expression (e.g. the observability areas,
paired with their voltage level to resolve their nodes and buses). The objects an extension holds are
objects too: a `Measurement` of a `Measurements` extension has the type code `MEASUREMENT`,
used for `getChildren` (all of them), `getRelatedByString` (one by id) and `invoke` (the adder of
a new one); the quality of P of an observability extension is the relation `OBS_QUALITY_P`.

#### Both backends — no extra C++ backend work

- **GraalVM**: the `@CEntryPoint` functions delegate every property code to
  the handle-based `PropertyDispatcher` methods.
- **JNI**: every `JNIBackend` operation is one static JNI call to the object-based
  `PropertyDispatcher` overloads — `getDouble(Object, int)`,
  `getChildObjects(Object, int)`, `getRelatedObject(Object, int)`, … Do **not** add
  per-property JNI calls to `JNIBackend.cpp`: the dispatcher is the only dispatch table.

#### Creating and removing objects

Adders are C++ classes deriving from `detail::IdentifiableAdder` / `detail::InjectionAdder`
(`include/iidm/Adder.h`), with one line per field: `return doubleField(prop::GEN_MIN_P, minP);`.
A field uses the code of the property it sets. The parent's `newX()` is
`XAdder(backend_->invoke(handle_, prop::X), backend_)`, and `remove()` is
`backend_->invoke(handle_, prop::ACT_REMOVE)`. On the Java side, a field is set by the adder column
of its property's line in `PropertyTable.java`; `Creation.java` has the fields that take more than
one call, and `remove` (powsybl's removal). Validation stays powsybl's. A change that takes several
values at once goes the same way (see `VoltageRegulation::setTerminal`): in `Creation.newAdder`, a
`PendingCall` names the call's fields and makes the call on `ACT_ADD`, without a class of its own:

```java
case REL_TERMINAL -> run((VoltageRegulation) owner, fields(REL_TERMINAL, VR_TARGET_VALUE),
    (regulation, c) -> regulation.setTerminal(c.get(REL_TERMINAL), c.dbl(VR_TARGET_VALUE)));
```

Only current powsybl APIs are ported: never a method marked `@Deprecated`, nor an extension
from `extensions.removed` (e.g. voltage regulation goes through the 7.4 `VoltageRegulation`
object, not `setVoltageRegulatorOn` / `setTargetV`). The Java build enforces it: `java/pom.xml`
compiles with `-Xlint:deprecation -Werror`, so a call to a deprecated powsybl method fails it.

#### Handles

Wrappers hold their handle in the `HandleRef handle_` of `detail::ObjectRef` (adders in the
`adder_` of `detail::AdderBase`), which releases the Java-side reference when the last wrapper
using it is destroyed. A new wrapper class gets it by passing `(handle, backend)` to its base
class, as `Load` does; a sub-wrapper built from the parent's handle shares the count automatically.

#### Errors

Throw from Java with the `BridgeErrors` exception types (`PropertyNotFoundException`
for unknown codes). An absent extension or optional object is null from
`getRelatedObject` (INVALID_HANDLE in C++), and the C++ getter decides: `getExtension<Foo>()` throws
`ExtensionNotFoundException`, an optional object comes back empty. Both backends turn a Java
exception into the same C++ exception: `BridgeErrors.kindOf` classifies it (first match below), and
`detail::throwBridgeError` (`src/BridgeErrors.h`) throws the C++ exception of that kind.

| Java exception | C++ exception |
|---|---|
| `BridgeErrors.InvalidHandleException` | `InvalidHandleException` |
| `BridgeErrors.PropertyNotFoundException`, `ClassCastException`, `NullPointerException` | `PropertyNotFoundException` |
| `BridgeErrors.NetworkNotFoundException` | `NetworkNotFoundException` |
| `PowsyblException`, `IllegalArgumentException`, `IllegalStateException`, `UnsupportedOperationException` | `ValidationException` |
| anything else | `IidmException` (the message starts with the Java class) |

- **GraalVM**: an exception escaping a `@CEntryPoint` would abort the process, so never let one
  throw: each catches `Throwable` and returns its kind as a status (0: success), and
  `GraalVMBackend::check()` takes the message with `iidm_take_last_error`.
- **JNI**: `checkJNIException()` (`src/jni/JNIExceptionBridge.h`) classifies the pending Java
  exception with the same `BridgeErrors` methods.

#### Checking the JNI path end to end

`examples/embedded` runs C++ inside a real JVM through JNI and fails (non-zero
exit) on any C++ exception:

```bash
javac -proc:none -cp java/target/iidm-bridge-java-1.0.0-SNAPSHOT-fat.jar \
      examples/embedded/JavaLauncher.java -d build/java-classes
LD_LIBRARY_PATH=build:build/examples/embedded \
java -cp "java/target/iidm-bridge-java-1.0.0-SNAPSHOT-fat.jar:build/java-classes" \
     -Djava.library.path=build/examples/embedded \
     JavaLauncher examples/state_estimation_test.xiidm
```

---

## Level 3 — Build variants & testing (read when modifying build or CI)

### All CMake options

| Option | Default | Meaning |
|---|---|---|
| `IIDM_BRIDGE_ENABLE_JNI` | `ON` | Build the JNI backend |
| `IIDM_BRIDGE_ENABLE_GRAALVM` | `ON` | Build the GraalVM backend |
| `IIDM_BRIDGE_BUILD_JAVA` | `ON` | Build the fat jar with Maven |
| `IIDM_BRIDGE_BUILD_NATIVE` | `ON` | Build the native image with `native-image` (even with the GraalVM backend off) |
| `IIDM_BRIDGE_BUILD_TESTS` | `OFF` | Build GoogleTest suite |
| `IIDM_BRIDGE_BUILD_EXAMPLES` | `ON` | Build the example programs |
| `IIDM_BRIDGE_ENABLE_COVERAGE` | `OFF` | gcov instrumentation and a `coverage` target |

### GraalVM backend build

The GraalVM backend includes no GraalVM header: it loads the native image at run time and looks up
its entry points, whose signatures `src/graalvm/GraalVMBackend.h` declares. The build still
involves GraalVM:

- With `IIDM_BRIDGE_BUILD_NATIVE=ON` (the default), the build makes the native image from the jar
  with GraalVM's `native-image`, which must be on the `PATH`, into
  `build/graalvm-native/libpowsybl-iidm-native.so` (`.dylib` on macOS), before the library.
- With `IIDM_BRIDGE_BUILD_NATIVE=OFF`, the configure looks for a GraalVM install (`GRAALVM_HOME`,
  `cmake/FindGraalVM.cmake`) for an include directory, and fails without one unless
  `-DGRAALVM_INCLUDE_DIRS` is given (any directory: nothing is read from it).

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DIIDM_BRIDGE_BUILD_TESTS=ON
cmake --build build --parallel
```

The backend loads the library given by `NetworkLoadOptions::graalvmLibPath`, else by
`IIDM_BRIDGE_GRAALVM_LIB`, else the library of that name on the library path.

### Integration tests

Set these environment variables before `ctest`:

```bash
export IIDM_IEEE14_NETWORK_PATH=/path/to/examples/IEEE14.xiidm
export IIDM_BRIDGE_GRAALVM_LIB=/path/to/libpowsybl-iidm-native.so  # GraalVM backend tests
export IIDM_BRIDGE_JAR=/path/to/java/target/iidm-bridge-java-1.0.0-SNAPSHOT-fat.jar  # JNI backend tests (in-process JVM)
ctest --test-dir build --output-on-failure
```

With either set, `TestAccessors` checks every one-line accessor on that backend: the value it reads
in the network of `test/CompleteNetwork.h`, and the value its setter writes, read back. In CI, the
`cpp` jobs run the JNI tests with the jar of the `java` job, on every platform and C++ standard.
With both `IIDM_BRIDGE_GRAALVM_LIB` and `IIDM_BRIDGE_JAR` set, `TestBackendConformance`
walks every object of the test networks on both backends in lockstep and fails on any
difference (value, children, related objects, exception type, setter behaviour, saved XIIDM).
A backend change is not done until it passes.

### CI pipeline summary (`.github/workflows/ci.yml`)

| Job | What it does |
|---|---|
| `java` | Maven build + test with JDK 21 |
| `cpp` | CMake Release build matrix (Ubuntu, macOS, Windows; C++11/14/17), JNI backend, unit tests, and the tests on the real JNI backend with the jar of `java` |
| `graalvm` | Builds both backends, the jar and the native image on Linux and macOS, and runs every test on both backends (conformance included); the Linux leg uploads the native image |
| `integration` | Downloads native artifacts, runs examples and integration tests against IEEE-14 network |
| `dynawo` | Self-contained full build inside `dynawo/dynawo-ci:latest` (installs Maven with dnf and GraalVM CE 21 from its release archive; builds the jar, the native image and the C++ library) and runs unit and end-to-end tests |
| `cpp-matrix` | Tests various backend combinations (JNI-only, no backends) on Linux |

The `dynawo` image is Fedora 29: GCC 8.3, CMake 3.14 (no `ctest --test-dir`), Maven 3.5.4,
GoogleTest 1.8.1 (no `GTEST_SKIP`, so the build fetches a newer one) and Unix Makefiles. Build
changes (CMake, Maven plugins) must work there too: the job's steps can be replayed in
`docker run -it dynawo/dynawo-ci:latest`.

---

## Level 4 — Architecture deep-dive (read when working on backends or memory model)

### Backend abstraction

`BackendProvider` (`include/iidm/BackendProvider.h`) is the interface of `GraalVMBackend`,
`JNIBackend` and, in the tests, `MockBackend`. A wrapper bound to no network (default-constructed,
or a moved-from `Network`) uses `NullBackend`, which throws on every call. The operations take a
handle and a code of `PropertyCodes.def`:

- values: `getDouble` / `setDouble` and the same for ints, bools and strings; `getOptDouble`,
  `getOptInt`, `getOptString`; `getStringList`, `getIntList`; `getKeyedString` / `setKeyedString`;
- navigation: `getChildren`, `getRelated`, `getRelatedByIndex`, `getRelatedByString`,
  `setRelated`, `findById`, `getNetworkHandle`;
- calls: `evaluate` (methods with numeric arguments), `invoke` (adders, actions, removal),
  `isSameObject`;
- the network: `loadNetwork`, `loadNetworkBytes`, `createNetwork`, `saveNetwork`,
  `saveNetworkBytes`, `close`;
- `releaseHandle`, which `HandleRef` calls when the last wrapper of a handle is destroyed.

### GraalVM backend (`src/graalvm/`)

- Calls into `libpowsybl-iidm-native.so` via the C entry-point signatures
  generated by `@CEntryPoint` annotations in `IidmEntryPoints.java`.
- Creates an isolate per network, torn down when the network closes.
- A handle is the number of the object in the isolate's `graalvm/NetworkRegistry.java`.
- Any thread can use it: pass `thread()` (the calling thread's isolate thread, attached on its first
  call, detached when the thread ends) to every entry point; never store an isolate thread.

### JNI backend (`src/jni/`)

- Uses an existing JVM (created by the application, e.g. Dynawo), found via
  `JNI_GetCreatedJavaVMs`.
- A handle is a JNI global reference to the Java object.
- Networks are exchanged by id through `jni/IidmBridgeRegistry.java`: the backend takes the network
  registered under its id (`"default"` when none is given), and registers there the networks it
  loads or creates.
- Any thread can use it: each operation starts with `JNIEnv* env = this->env();` (the calling
  thread's environment; a thread is attached on its first call and detached when it ends) and passes
  `env` to the helpers; never store a `JNIEnv*`.

### Memory lifecycle

- A `Network` owns its backend: each network that `NetworkFactory` loads or creates gets a backend
  of its own (with GraalVM, its own isolate), and destroying the `Network` closes it.
- Every handle that a backend returns holds a reference on the Java side. `HandleRef` counts the
  wrappers of each handle (copies, and sub-wrappers built from it) and releases the reference with
  the last one; closing the backend releases the handles still held.
- Wrappers keep a plain pointer to the backend of their network. When the `Network` is destroyed,
  its closed backend goes to the `HandleTracker` that the network's wrappers share
  (`HandleTracker::keep`) and is freed with the last of them: an object used after its `Network`
  is destroyed throws `InvalidHandleException`.

### AUTO backend selection

`selectBackend` (`src/BackendSelector.cpp`) opens the backend of each network that `NetworkFactory`
loads or creates:

1. `IIDM_BRIDGE_MODE` (`graalvm` or `jni`), when set, replaces `NetworkLoadOptions::mode`.
2. `JNI` and `GRAALVM` open that backend, or throw `BackendNotAvailableException` when it is not
   compiled in.
3. `AUTO` opens the JNI backend when a JVM runs in the process (`JNI_GetCreatedJavaVMs`), else the
   GraalVM backend, which loads the library described in "GraalVM backend build". With neither, it
   throws `BackendNotAvailableException`.
