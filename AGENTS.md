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

**Handle model** — every component stores an `ObjectHandle` (opaque integer)
plus a `BackendProvider*`.  It never copies network data into C++ memory.

```cpp
// Typical component header shape
class Generator {
public:
    std::string getId() const;
    double getTargetP() const;
    Generator& setTargetP(double mw);   // chainable setter
    // ...
private:
    ObjectHandle handle_;
    BackendProvider* backend_;
};
```

**Property codes** — C++/Java communicate via integer keys defined in
`include/iidm/PropertyCodes.h` and mirrored in
`java/src/main/java/com/powsybl/iidmbridge/PropertyCodes.java`.
**Both files must stay in sync.**  There is a CI test that validates this.

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

1. Add a header under `include/iidm/MyComponent.h` following the existing
   component headers as a template.
2. Add `src/MyComponent.cpp`.
3. Add any new property integer codes to **both** `PropertyCodes.h` and
   `PropertyCodes.java` (same value in each file).
4. Expose the component through `Network` (or the appropriate parent) with a
   getter that returns by value or `std::vector`.
5. Add a corresponding `getMyComponent` / `getMyComponents` entry in
   `java/.../PropertyDispatcher.java`.
6. Add unit tests in `test/TestMyComponent.cpp` using `MockBackend`.

### Adding a new extension

IIDM extensions (e.g. `ActivePowerControl`) are optional property bags
attached to network components. Both backends dispatch them through
`PropertyDispatcher.java` — **read this section in full before touching
extension code**. The supported extensions are listed in README §5.4.

#### Steps common to both backends

1. **Property codes** — add constants to **both** `PropertyCodes.h` and
   `PropertyCodes.java` (same integer values). Reserve three kinds of codes:
   existence check, getters, setters.

   ```cpp
   // include/iidm/PropertyCodes.h
   constexpr int EXT_FOO_EXISTS  = 9000;  // bool (pick the next free block)
   constexpr int EXT_FOO_SOME_D  = 9001;  // double getter/setter
   constexpr int EXT_FOO_SOME_B  = 9002;  // bool getter/setter
   ```

2. **C++ extension class** — add `include/iidm/FooExtension.h` and
   `src/FooExtension.cpp` following `ActivePowerControl` as a template.
   The class holds the same `ObjectHandle` as its host component and calls
   the backend with `EXT_FOO_*` property codes.

3. **Host component accessor** — add `hasFooExtension()` and
   `getFooExtension()` to the host component (e.g. `Generator`).

4. **Java `PropertyDispatcher`** — add cases to the appropriate switch
   blocks:

   ```java
   // PropertyDispatcher.java – getDouble
   case EXT_FOO_SOME_D -> ((Generator) obj).getExtension(FooExtension.class).getSomeD();

   // PropertyDispatcher.java – getBool
   case EXT_FOO_EXISTS  -> ((Generator) obj).getExtension(FooExtension.class) != null;
   case EXT_FOO_SOME_B  -> ((Generator) obj).getExtension(FooExtension.class).isSomeB();
   ```

5. **Unit tests** — `test/TestFooExtension.cpp` using `MockBackend`, plus a
   Java test of the dispatcher cases (see `StateEstimationDispatchTest`).

Larger extensions can keep their Java logic in a helper class called from the
dispatcher cases (e.g. `StateEstimationDispatch.java`). Child objects of an
extension (e.g. `Measurement`) get a child-type code (`getChildren`) and, for
lookups by id, a `getRelatedByString` relation.

#### Both backends — no extra C++ backend work

- **GraalVM**: the `@CEntryPoint` functions delegate every property code to
  the handle-based `PropertyDispatcher` methods.
- **JNI**: `JNIBackend` hand-wires only hot core properties; every code it does
  not handle falls back (in its `default:` branches) to the object-based
  `PropertyDispatcher` overloads — `getDouble(Object, int)`,
  `getChildObjects(Object, int)`, `getRelatedObject(Object, int)`, … — in one
  static JNI call. Do **not** hand-wire extension codes in `JNIBackend.cpp`.

If you do hand-wire a JNI call (hot path only), its `GetMethodID` signature
must match the pinned powsybl-core version exactly: a single mismatch makes
`cacheMethodIds()` throw and the whole JNI backend unusable. Check the
signature with `javap -s` against the powsybl jar.

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
export IIDM_BRIDGE_GRAALVM_LIB=/path/to/libpowsybl-iidm-native.so  # GraalVM only
ctest --test-dir build --output-on-failure
```

### CI pipeline summary (`.github/workflows/ci.yml`)

| Job | What it does |
|---|---|
| `java` | Maven build + test with JDK 21 |
| `cpp` | CMake Release build matrix (Ubuntu, macOS, Windows; C++11/14/17), JNI backend, unit tests |
| `graalvm` | Builds GraalVM native image (`libpowsybl-iidm-native.so`) and runs unit tests; Linux leg uploads artifacts |
| `integration` | Downloads native artifacts, runs examples and integration tests against IEEE-14 network |
| `dynawo` | Self-contained full build (installs GraalVM CE 21 + Maven via dnf, builds JAR + native image + C++ library) and runs unit and end-to-end tests inside `dynawo/dynawo-ci:latest` |
| `cpp-matrix` | Tests various backend combinations (JNI-only, no backends) on Linux |

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
