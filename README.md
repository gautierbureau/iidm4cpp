# iidm-bridge

[![CI](https://github.com/gautierbureau/iidm4cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/gautierbureau/iidm4cpp/actions/workflows/ci.yml)
[![MPL-2.0 License](https://img.shields.io/badge/license-MPL_2.0-blue.svg)](https://www.mozilla.org/en-US/MPL/2.0/)

The [PowSyBl](https://www.powsybl.org) IIDM network model for C++, served by powsybl-core itself.

iidm-bridge replaces [powsybl-iidm4cpp](https://github.com/powsybl/powsybl-iidm4cpp), the C++
reimplementation of the IIDM network model. It reimplements nothing: every call goes to the Java
network model of [powsybl-core](https://github.com/powsybl/powsybl-core) 7.4, through one of two
backends that share one Java dispatch table (`PropertyDispatcher`):

- **Standalone mode (GraalVM):** the Java side compiled into a native library,
  `libpowsybl-iidm-native`, that the C++ library loads with `dlopen`. No JVM is started.
- **Embedded mode (JNI):** the C++ code runs next to a JVM that already exists (Dynawo launched
  from Java, for example) and works on the network objects of that JVM, without a copy.

Both modes have the same C++ API. It follows powsybl's interfaces (`Network`, `Generator`, `Load`,
`Line`...) in `namespace iidm`, with the headers in [`include/iidm`](include/iidm), and covers the
whole IIDM network model and every extension of powsybl-core 7.4 (the DC detailed model apart):
reading and changing values, creating objects through adders, removing them, variants, and use
from any thread. Validation stays powsybl's.

## Requirements

- A C++17 compiler, or a C++11/14 one with the [Boost](https://www.boost.org) headers
  (`iidm::optional` is then `boost::optional`). CI builds with GCC, Clang and MSVC.
- [CMake](https://cmake.org) 3.14.5 or later.
- A JDK 21 and [Maven](https://maven.apache.org), for the Java side (built into a fat jar with
  powsybl-core) and the JNI backend.
- [GraalVM](https://www.graalvm.org) for JDK 21 with `native-image`, for the native image of the
  standalone mode: Linux and macOS (Windows builds the JNI backend only).

## Build

With the default options, CMake builds everything: both backends, the jar (with Maven) and the
native image (with `native-image`). Both tools must be on the `PATH`:

```bash
export JAVA_HOME=/path/to/graalvm-jdk-21
export PATH=$JAVA_HOME/bin:$PATH
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

This gives the library `build/libiidmbridge.so`, the native image
`build/graalvm-native/libpowsybl-iidm-native.so`, the jar
`java/target/iidm-bridge-java-1.0.0-SNAPSHOT-fat.jar` and the examples in `build/examples`
(Linux names throughout: on macOS, read `.dylib` and `DYLD_LIBRARY_PATH`).

The JNI backend alone needs any JDK 21 and Maven (the jar is still built: the JVM needs it):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DIIDM_BRIDGE_ENABLE_GRAALVM=OFF -DIIDM_BRIDGE_BUILD_NATIVE=OFF
cmake --build build --parallel 4
```

Add `-DIIDM_BRIDGE_BUILD_TESTS=ON` to either for the [tests](#tests). The main options:

| Option | Default | What |
|---|---|---|
| `IIDM_BRIDGE_ENABLE_GRAALVM` | `ON` | The GraalVM backend (standalone mode) |
| `IIDM_BRIDGE_ENABLE_JNI` | `ON` | The JNI backend (embedded mode) |
| `IIDM_BRIDGE_BUILD_JAVA` | `ON` | Builds the jar with Maven |
| `IIDM_BRIDGE_BUILD_NATIVE` | `ON` | Builds the native image from the jar with `native-image` |
| `IIDM_BRIDGE_BUILD_TESTS` | `OFF` | The tests (GoogleTest 1.10 or later, downloaded if not installed) |
| `IIDM_BRIDGE_BUILD_EXAMPLES` | `ON` | `examples/standalone` and `examples/embedded` |
| `IIDM_BRIDGE_ENABLE_COVERAGE` | `OFF` | gcov instrumentation, and a `coverage` target (gcovr) |
| `CMAKE_CXX_STANDARD` | `17` | `11` or `14` need Boost (`-DBOOST_ROOT=...` if CMake does not find it) |

### Use it from another CMake project

`cmake --install build --prefix /opt/iidm-bridge` installs the headers, the library, the native
image and a CMake package. In your `CMakeLists.txt`:

```cmake
find_package(Boost)   # needed if Boost was found when the library was built
find_package(IidmBridge 1.0 REQUIRED)
target_link_libraries(my_app PRIVATE IidmBridge::iidmbridge)
```

and configure your project with `-DCMAKE_PREFIX_PATH=/opt/iidm-bridge`. Notes:

- The C++ standard must match: a library built in C++17 needs C++17 code, one built in C++11/14
  takes any (a mismatch fails to compile).
- With the JNI backend built in, the installed library needs `libjvm` at run time, even in
  standalone mode: add `$JAVA_HOME/lib/server` to `LD_LIBRARY_PATH`.
- The jar is not installed: take it from `java/target`.

## Usage

Load a network, read it, change it, use an extension and save it:

```cpp
#include <iidm/iidm.h>

#include <iostream>

int main() {
    try {
        // Standalone mode: powsybl runs in this process, in the GraalVM native image
        iidm::NetworkLoadOptions options;
        options.mode = iidm::BackendMode::GRAALVM;
        iidm::Network network = iidm::NetworkFactory::load("examples/IEEE14.xiidm", options);
        std::cout << network.getId() << ": " << network.getGeneratorCount() << " generators, "
                  << network.getLoadCount() << " loads\n";

        // Read: lists of objects, and objects by id (an empty optional when there is none)
        for (const iidm::Generator& generator : network.getGenerators()) {
            std::cout << "  " << generator.getId() << " targetP=" << generator.getTargetP()
                      << " MW, P=" << generator.getTerminal().getP() << " MW\n";
        }
        iidm::Load load = network.getLoad("_LOAD__10_EC").value();
        std::cout << "  " << load.getId() << " P0=" << load.getP0() << " MW\n";

        // Change: setters return the object, so they chain
        load.setP0(10.5).setQ0(6.0);

        // Extensions, by their class, as in powsybl
        iidm::Generator gen3 = network.getGenerator("_GEN____3_SM").value();
        iidm::ActivePowerControl control = gen3.getExtension<iidm::ActivePowerControl>();
        std::cout << "  " << gen3.getId() << " droop=" << control.getDroop() << "\n";
        control.setParticipate(true);

        iidm::Generator gen1 = network.getGenerator("_GEN____1_SM").value();
        gen1.newExtension<iidm::ActivePowerControlAdder>().withParticipate(true).withDroop(5.0).add();

        network.save("IEEE14-modified.xiidm");
    } catch (const iidm::IidmException& e) {  // the base of every exception of the library
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
```

Build it as `my_app` with the CMake project above, and run it from the root of this repository
(for `examples/IEEE14.xiidm`). The native image is `NetworkLoadOptions::graalvmLibPath`, else
`IIDM_BRIDGE_GRAALVM_LIB`, else `libpowsybl-iidm-native.so` from the library path:

```text
$ export LD_LIBRARY_PATH=$JAVA_HOME/lib/server   # libjvm, see the notes above
$ IIDM_BRIDGE_GRAALVM_LIB=/opt/iidm-bridge/lib/libpowsybl-iidm-native.so path/to/my_app
ieee14bus: 5 generators, 11 loads
  _GEN____1_SM targetP=232.346 MW, P=-232.39 MW
  _GEN____2_SM targetP=40 MW, P=-40 MW
  ...
  _LOAD__10_EC P0=9 MW
  _GEN____3_SM droop=4
```

powsybl's own log goes to stderr. [`examples/standalone/main.cpp`](examples/standalone/main.cpp)
reads much more of a network (transformers, tap changers, reactive limits, HVDC lines...).

### Embedded mode

The Java program registers its network, then calls the C++ code through a native method:

```java
Network network = Network.read(Path.of("IEEE14.xiidm"));
IidmBridgeRegistry.register("case1", network);   // com.powsybl.iidmbridge.jni.IidmBridgeRegistry
runSimulation("case1");                          // a native method of your C++ library
```

```cpp
// In the native method: the network of the JVM itself, no file and no copy
iidm::Network network = iidm::NetworkFactory::wrap("case1");
network.getGenerator("_GEN____1_SM")->setTargetP(200.0);   // seen by Java at once
```

The JVM needs the jar on its class path. [`examples/embedded`](examples/embedded) is a complete
example, run by the CI. With the default mode, `BackendMode::AUTO`, `NetworkFactory::load` uses
JNI when the process already runs a JVM and GraalVM otherwise; `IIDM_BRIDGE_MODE=graalvm` or `jni`
overrides the mode.

### Good to know

- **Lifetime.** A `Network` owns its backend session and cannot be copied; the objects taken from
  it are light handles to the Java objects, valid while it lives.
- **Beyond reading.** `NetworkFactory::create(id)` makes an empty network; adders shaped like
  powsybl's create objects (`network.newSubstation().setId("S")...add()`), `remove()` removes
  them. Variants come from `network.getVariantManager()`; `NetworkFactory::loadFromBytes` and
  `Network::saveToBytes` work in memory.
- **Threads.** A network can be used from any thread, and its objects passed between threads. As
  in powsybl it is not synchronized: threads that change it at the same time need a lock, or each
  its own variant (`getVariantManager().allowVariantMultiThreadAccess(true)`).

### Errors

The library reports errors with exceptions, the same on both backends; a Java failure never
aborts the process.

| Exception (`iidm::`) | Thrown when |
|---|---|
| `IidmException` | The base of the others; any other failure |
| `BackendNotAvailableException` | The backend is not built in, the native image cannot be loaded, or there is no JVM |
| `ValidationException` | powsybl rejects the call: invalid value, id already used, refused removal... |
| `PropertyNotFoundException` | The method does not apply to this object (`as<T>()` of another type), an optional part is absent |
| `ExtensionNotFoundException` | `getExtension<X>()` on an object without X (a `PropertyNotFoundException`) |
| `InvalidHandleException` | The object belongs to no network (default-constructed, or a moved-from `Network`) |
| `NetworkNotFoundException` | `NetworkFactory::wrap(id)`: no network registered under that id |

### Extensions

The extensions of powsybl-core 7.4, each with its adder (`ActivePowerControlAdder`...):
ActivePowerControl, BatteryShortCircuit, BranchObservability, BusbarSectionPosition,
ConnectablePosition, CoordinatedReactiveControl, DiscreteMeasurements, DynamicModelInfo,
GeneratorEntsoeCategory, GeneratorFortescue, GeneratorShortCircuit, GeneratorStartup,
HvdcAngleDroopActivePowerControl, HvdcOperatorActivePowerRange, IdentifiableShortCircuit,
InjectionObservability, LineCouplings, LineFortescue, LinePosition, LoadAsymmetrical, LoadDetail,
ManualFrequencyRestorationReserve, Measurements, ObservabilityArea, OperatingStatus,
ReferencePriorities, ReferenceTerminals, SecondaryVoltageControl, SlackTerminal, StandbyAutomaton,
SubstationPosition, ThreeWindingsTransformerFortescue, ThreeWindingsTransformerPhaseAngleClock,
ThreeWindingsTransformerToBeEstimated, TwoWindingsTransformerFortescue,
TwoWindingsTransformerPhaseAngleClock and TwoWindingsTransformerToBeEstimated.
VoltagePerReactivePowerControl, RemoteReactivePowerControl and the battery VoltageRegulation
extension are not ported: powsybl-core 7.4 retired them (`extensions.removed`) for the core
`VoltageRegulation` of the equipment.

## Tests

Configure with `-DIIDM_BRIDGE_BUILD_TESTS=ON`, build, then run the tests from the root of the
repository (`ctest --test-dir` needs CMake 3.20; with an older one, run `ctest` in `build`):

```bash
IIDM_BRIDGE_GRAALVM_LIB=$PWD/build/graalvm-native/libpowsybl-iidm-native.so \
IIDM_BRIDGE_JAR=$PWD/java/target/iidm-bridge-java-1.0.0-SNAPSHOT-fat.jar \
IIDM_IEEE14_NETWORK_PATH=$PWD/examples/IEEE14.xiidm \
ctest --test-dir build --output-on-failure
```

The unit tests need none of these variables. The tests on a real backend are skipped without its
variable (an absolute path: the tests run in `test/`):

| Variable | Runs |
|---|---|
| `IIDM_BRIDGE_GRAALVM_LIB` | The tests on the GraalVM backend, with this native image |
| `IIDM_BRIDGE_JAR` | The tests on the JNI backend, in a JVM that the test process starts with this jar |
| `IIDM_IEEE14_NETWORK_PATH` | With the native image, `TestIntegration` on this network (`examples/IEEE14.xiidm`) |

With both backends, `TestBackendConformance` also checks that they give the same results on every
object of the test networks. The Java side has its own tests: `cd java && mvn verify`.

## Contributing

[AGENTS.md](AGENTS.md) describes the architecture, the conventions (property codes, wrappers,
adders, extensions) and how to add a component or an extension, with the tests to write.

## License

[Mozilla Public License 2.0](LICENSE.txt), as powsybl.
