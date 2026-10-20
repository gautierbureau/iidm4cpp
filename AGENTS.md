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
      -DIIDM_BRIDGE_BUILD_TESTS=ON
cmake --build build --parallel
```

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

Unit tests use `MockBackend` and need no external files.
Integration tests require environment variables — see Level 3.

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
| Private members | `snake_case_` (trailing `_`) | `handle_`, `backend_` |
| Enum values | `UPPER_SNAKE_CASE` | `ComponentType::GENERATOR` |

### Key patterns

**Handle model** — every component is a handle on a Java object plus a `BackendProvider*`,
held by the root class `detail::ObjectRef`. It never copies network data into C++ memory.

```cpp
// Typical component header shape
class Generator : public Injection {          // the powsybl interface it extends
public:
    static constexpr int TYPE = prop::GENERATOR;
    Generator() = default;
    explicit Generator(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getTargetP() const;
    Generator& setTargetP(double mw);         // chainable setter
};
```

**Class model** — the C++ classes follow the powsybl interfaces:

- `Identifiable` → `Connectable` → `Injection` → `HvdcConverterStation` are base classes; a class
  derives from the one its powsybl interface extends, and gets its methods (id, names, properties,
  aliases, terminals, connect/disconnect, remove, the extensions of that host type...). Never
  re-declare a base-class method in a derived class.
- A powsybl interface that unrelated types share (`Branch`: lines and transformers, but also tie
  lines, which are not connectables; `VoltageRegulationHolder`: generators, batteries, SVCs,
  shunts, VSC stations, ratio tap changers; `ReactiveLimitsHolder`; `TapChanger`, `TapChangerStep`,
  the tap changer holders, `FlowsLimitsHolder`: legs and dangling lines) is a CRTP mixin
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
the Java `PropertyCodes` class from it. There is no second copy to keep in sync.

**Fluent setters** — setters return `*this` so callers can chain:

```cpp
gen.setTargetP(100.0).setTargetQ(0.0);
```

**`std::optional`** — nullable properties return `std::optional<T>`.

**Move-only components** — copy constructors are deleted; move constructors are
provided.  Do not attempt to copy component objects.

**Stream / byte-buffer I/O** — `NetworkFactory::load(std::istream&)` and
`NetworkFactory::loadFromBytes(...)` parse a network from an in-memory buffer
instead of a file path; `Network::save(std::ostream&)` and
`Network::saveToBytes()` go the other way. Both routes are wired through new
backend methods `loadNetworkBytes` / `saveNetworkBytes`; `NetworkLoadOptions::filenameHint`
selects the format (default `"network.xiidm"`).

### File layout

```
include/iidm/   Public headers (one per component + iidm.h convenience header)
src/            Implementations; backend-specific code in graalvm/ and jni/
java/           Maven module for the Java bridge
test/           GoogleTest unit tests; MockBackend.h fakes the backend
examples/       Standalone (GraalVM) and embedded (JNI) usage examples
cmake/          CMake helper modules (FindGraalVM.cmake, etc.)
```

### Adding a new network component

1. Add a header under `include/iidm/MyComponent.h`: derive from the class of the powsybl interface
   it extends (`Injection`, `Connectable`, `Identifiable`; mixins for the others), declare
   `TYPE`, and only the methods of its own powsybl interface.
2. Add `src/MyComponent.cpp`.
3. Add a type code and its property codes to `include/iidm/PropertyCodes.def`: one code per method
   of a powsybl interface, named after that interface.
4. Expose the component through `Network` (or the appropriate parent) with a
   getter that returns by value or `std::vector`.
5. In `java/.../PropertyDispatcher.java`: its property cases, its type code in `getBool` (instanceof),
   `childStream` and `findObjectById`.
6. Add unit tests in `test/TestMyComponent.cpp` using `MockBackend`.

### Primitives for values that may be absent, lists and keys

- An optional value: `getOptInt` / `getOptString` (and `getOptDouble`, where NaN is no value).
  The plain getter of the same code gives a non-null value or fails (`getString(NAME)` is the
  name or else the id); never invent sentinels for ints (tap positions can be negative).
- A list of strings: `getStringList` (sets are sorted on the Java side, so that both backends
  give the same order).
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
extension code**. The supported extensions are listed in README §5.4.

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

2. **C++ classes** — `include/iidm/Foo.h` + `src/Foo.cpp` (the wrapper, holding the extension's
   handle) and `include/iidm/FooAdder.h` (`detail::AdderBase`, one line per field, `add()`).

3. **Host accessors** — `hasFoo()`, `getFoo()` (`ExtensionNotFoundException` when absent),
   `newFoo()` and `removeFoo()` on the class of its powsybl host type: an extension of any
   injection goes on `Injection` (see `ActivePowerControl` in `Injection.h`), of a generator only
   on `Generator`. Each is one line, with the helpers of `src/Extensions.h`.

4. **Java** — in `PropertyDispatcher.java`, `getRelatedObject` returns the extension for `FOO`,
   and the property cases cast the extension object; in `Creation.java`, `newAdder` creates its
   adder, the `set*` methods map its fields, and `clear` removes it:

   ```java
   // PropertyDispatcher.java
   case FOO            -> extension(obj, Foo.class);                       // getRelatedObject
   case EXT_FOO_SOME_D -> ((Foo<?>) obj).getSomeD();                       // getDouble
   // Creation.java
   case FOO            -> newExtension(owner, FooAdder.class);             // newAdder
   case EXT_FOO_SOME_D -> ((FooAdder<?>) a).withSomeD(value);              // setDouble (adder field)
   ```

5. **Unit tests** — `test/TestFoo.cpp` using `MockBackend`, plus a Java test of the dispatcher
   and creation cases (see `ExtensionDispatchTest`, `CreationDispatchTest`). Put the extension in
   a test network (`examples/*.xiidm`): `TestBackendConformance` then compares every new code,
   and the adder, on both backends without any change to the test.

Larger extensions can keep their Java logic in a helper class called from the
dispatcher cases (e.g. `StateEstimationDispatch.java`). The objects an extension holds are
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
`backend_->invoke(handle_, prop::ACT_REMOVE)`. On the Java side, `Creation.java` maps each
(adder, field code) to the powsybl adder method, and `remove` to powsybl's removal. Validation
stays powsybl's. A change that takes several values at once goes the same way: see
`VoltageRegulation::setTerminal`.

Only current powsybl APIs are ported: never a method marked `@Deprecated`, nor an extension
from `extensions.removed` (e.g. voltage regulation goes through the 7.4 `VoltageRegulation`
object, not `setVoltageRegulatorOn` / `setTargetV`). The Java build enforces it: `java/pom.xml`
compiles with `-Xlint:deprecation -Werror`, so a call to a deprecated powsybl method fails it.

#### Handles

Wrappers store their handle in a `HandleRef` member (`HandleRef handle_;`, constructed
with `handle_(handle, backend)`), which releases the Java-side reference when the last
wrapper using it is destroyed. A new wrapper class must follow the same pattern; a
sub-wrapper built from the parent's handle shares the count automatically.

#### Errors

Throw from Java with the `BridgeErrors` exception types (`PropertyNotFoundException`
for unknown codes). An absent extension or optional object is null from
`getRelatedObject` (INVALID_HANDLE in C++), and the C++ getter decides: `getFoo()` throws
`ExtensionNotFoundException`, an optional object comes back empty. Both backends turn Java
exceptions into the matching C++ exception (README §9); never let a GraalVM `@CEntryPoint`
throw.

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
| `IIDM_BRIDGE_ENABLE_GRAALVM` | `OFF` | Build the GraalVM backend |
| `IIDM_BRIDGE_BUILD_TESTS` | `OFF` | Build GoogleTest suite |
| `IIDM_BRIDGE_BUILD_EXAMPLES` | `OFF` | Build the example programs |

### GraalVM backend build

Requires GraalVM SDK headers (`graal_isolate.h`, etc.):

```bash
cmake -S . -B build -G Ninja \
      -DIIDM_BRIDGE_ENABLE_GRAALVM=ON \
      -DGRAALVM_INCLUDE_DIRS=/path/to/graalvm/include
cmake --build build --parallel
```

### Integration tests

Set these environment variables before `ctest`:

```bash
export IIDM_IEEE14_NETWORK_PATH=/path/to/examples/IEEE14.xiidm
export IIDM_BRIDGE_GRAALVM_LIB=/path/to/libpowsybl-iidm-native.so  # GraalVM backend tests
export IIDM_BRIDGE_JAR=/path/to/java/target/iidm-bridge-java-1.0.0-SNAPSHOT-fat.jar  # JNI backend tests (in-process JVM)
ctest --test-dir build --output-on-failure
```

With both `IIDM_BRIDGE_GRAALVM_LIB` and `IIDM_BRIDGE_JAR` set, `TestBackendConformance`
walks every object of the test networks on both backends in lockstep and fails on any
difference (value, children, related objects, exception type, setter behaviour, saved XIIDM).
A backend change is not done until it passes.

### CI pipeline summary (`.github/workflows/ci.yml`)

| Job | What it does |
|---|---|
| `java` | Maven build + test with JDK 21 |
| `cpp` | CMake Release build matrix (Ubuntu, macOS, Windows; C++11/14/17), JNI backend, unit tests |
| `graalvm` | Builds GraalVM native image (`libpowsybl-iidm-native.so`) and runs unit tests; Linux leg uploads artifacts |
| `integration` | Downloads native artifacts, runs examples and integration tests against IEEE-14 network |
| `dynawo` | Self-contained full build (installs GraalVM CE 21 + Maven via dnf, builds JAR + native image + C++ library) and runs unit and end-to-end tests inside `dynawo/dynawo-ci:latest` |
| `cpp-matrix` | Tests various backend combinations (JNI-only, no backends) on Linux |

The `dynawo` image is Fedora 29: GCC 8.3, CMake 3.14 (no `ctest --test-dir`), Maven 3.5.4,
GoogleTest 1.8.1 (no `GTEST_SKIP`, so the build fetches a newer one) and Unix Makefiles. Build
changes (CMake, Maven plugins) must work there too: the job's steps can be replayed in
`docker run -it dynawo/dynawo-ci:latest`.

---

## Level 4 — Architecture deep-dive (read when working on backends or memory model)

### Backend abstraction

`BackendProvider` (`include/iidm/BackendProvider.h`) is the pure-virtual
interface that both backends implement.  It exposes:

- `getProperty(handle, code)` / `setProperty(handle, code, value)` — typed
  overloads for `string`, `double`, `int`, `bool`, `optional<...>`.
- `buildList<T>(handle, listCode)` — constructs a `std::vector<T>` of child
  components.
- `findById<T>(handle, id)` — locates a component by string id.

### GraalVM backend (`src/graalvm/`)

- Calls into `libpowsybl-iidm-native.so` via the C entry-point signatures
  generated by `@CEntryPoint` annotations in `IidmEntryPoints.java`.
- Creates/destroys a GraalVM isolate per process.  Not thread-safe across
  isolate boundaries.

### JNI backend (`src/jni/`)

- Attaches to an existing JVM (created by the application, e.g. Dynawo) via
  `JNI_GetCreatedJavaVMs`.
- Uses `IidmBridgeRegistry.java` to map Java object references to handles.
- Thread-safe: each native thread attaches its own `JNIEnv`.

### Memory lifecycle

```
NetworkFactory::load(path)
  → returns Network (owning ObjectHandle)
    → Network::getGenerators()
        → returns std::vector<Generator>   (each holds borrowed handle)
```

- `Network` destruction releases the root Java object reference.
- Component handles (Generator, Load, etc.) are **borrowed** — they are
  invalidated when their parent `Network` is destroyed.
- Never store a component reference beyond its Network's lifetime.

### AUTO backend selection

`BackendSelector` (in `src/BackendSelector.cpp`):
1. Tries to `dlopen` `libpowsybl-iidm-native.so` from `LD_LIBRARY_PATH`.
2. Falls back to JNI if `JNI_GetCreatedJavaVMs` returns a running JVM.
3. Throws `BackendNotAvailableException` if neither is found.
