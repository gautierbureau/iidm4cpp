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
attached to network components. The two backends handle them differently —
**read this section in full before touching extension code**.

#### Steps common to both backends

1. **Property codes** — add constants to **both** `PropertyCodes.h` and
   `PropertyCodes.java` (same integer values). Reserve three kinds of codes:
   existence check, getters, setters.

   ```cpp
   // include/iidm/PropertyCodes.h
   constexpr int EXT_FOO_EXISTS  = 4000;  // bool
   constexpr int EXT_FOO_SOME_D  = 4001;  // double getter/setter
   constexpr int EXT_FOO_SOME_B  = 4002;  // bool getter/setter
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

5. **Unit tests** — `test/TestFooExtension.cpp` using `MockBackend`.

#### GraalVM backend — no extra work

The GraalVM backend delegates every property code to `PropertyDispatcher`
through the `@CEntryPoint` functions. Once you add the Java cases above, the
GraalVM path works automatically.

#### JNI backend — explicit wiring required

The JNI backend calls Java methods directly and bypasses `PropertyDispatcher`.
You must wire each extension property code by hand:

1. **`src/jni/JNIMethodCache.h`** — add a `jclass` for the extension
   interface and a `jmethodID` for each getter/setter you need, plus one for
   `getExtensionByName` on the host class:

   ```cpp
   jclass    fooExtensionClass        = nullptr;
   jmethodID generator_getExtByName   = nullptr;  // shared if already cached
   jmethodID foo_getSomeD             = nullptr;
   jmethodID foo_setSomeD             = nullptr;
   jmethodID foo_isSomeB              = nullptr;
   jmethodID foo_setSomeB             = nullptr;
   ```

2. **`src/jni/JNIBackend.cpp` — `cacheMethodIds()`** — initialise the new
   fields. Use `getExtensionByName(String)` (not `getExtension(Class)`) to
   avoid passing a class object through JNI:

   ```cpp
   cacheClass(cache_.fooExtensionClass,
       "com/powsybl/iidm/network/extensions/FooExtension");
   cache_.generator_getExtByName = env_->GetMethodID(cache_.generatorClass,
       "getExtensionByName",
       "(Ljava/lang/String;)Lcom/powsybl/commons/extensions/Extension;");
   cache_.foo_getSomeD = env_->GetMethodID(cache_.fooExtensionClass, "getSomeD", "()D");
   cache_.foo_setSomeD = env_->GetMethodID(cache_.fooExtensionClass, "setSomeD", "(D)V");
   cache_.foo_isSomeB  = env_->GetMethodID(cache_.fooExtensionClass, "isSomeB",  "()Z");
   cache_.foo_setSomeB = env_->GetMethodID(cache_.fooExtensionClass, "setSomeB", "(Z)V");
   checkJNIException(env_);
   ```

3. **Private helper** — add a `fetchFooExtension(jobject)` method (returns a
   local JNI ref; caller must `DeleteLocalRef`):

   ```cpp
   jobject JNIBackend::fetchFooExtension(jobject host) const {
       jstring name = env_->NewStringUTF("fooExtension");  // the NAME constant
       jobject ext  = env_->CallObjectMethod(host, cache_.generator_getExtByName, name);
       env_->DeleteLocalRef(name);
       checkJNIException(env_);
       return ext;
   }
   ```

4. **Switch cases** — add to `getDouble`, `setDouble`, `getBool`, `setBool`:

   ```cpp
   case prop::EXT_FOO_EXISTS: {
       jobject ext = fetchFooExtension(obj);
       result = (ext != nullptr) ? JNI_TRUE : JNI_FALSE;
       if (ext) env_->DeleteLocalRef(ext);
       break;
   }
   case prop::EXT_FOO_SOME_D: {
       jobject ext = fetchFooExtension(obj);
       if (!ext) throw PropertyNotFoundException("FooExtension not present");
       result = env_->CallDoubleMethod(ext, cache_.foo_getSomeD);
       env_->DeleteLocalRef(ext);
       break;
   }
   ```

#### Asymmetry summary

| What to change | GraalVM | JNI |
|---|---|---|
| `PropertyCodes.h` + `.java` | required | required |
| C++ extension class + host accessor | required | required |
| `PropertyDispatcher.java` cases | required | not required |
| `JNIMethodCache.h` fields | — | required |
| `JNIBackend.cpp` cache init + switch cases | — | required |

Forgetting the JNI half causes `PropertyNotFoundException` at runtime when
the JNI backend is active, while the GraalVM backend continues to work.

---

## Level 3 — Build variants & testing (read when modifying build or CI)

### All CMake options

| Option | Default | Meaning |
|---|---|---|
| `IIDM_BRIDGE_ENABLE_JNI` | `ON` | Build the JNI backend |
| `IIDM_BRIDGE_ENABLE_GRAALVM` | `OFF` | Build the GraalVM backend |
| `IIDM_BRIDGE_BUILD_TESTS` | `ON` | Build GoogleTest suite |
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
| `cpp-jni` | CMake Release build, JNI backend, unit tests |
| `graalvm-stub` | Builds a stub native image to generate C headers |
| `graalvm-native` | Builds real `libpowsybl-iidm-native.so` |
| `integration` | Runs integration tests against IEEE-14 network |
| `matrix` | Tests various backend combinations (JNI-only, no backends) |

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
