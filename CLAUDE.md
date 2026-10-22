# CLAUDE.md

See **[AGENTS.md](AGENTS.md)** for the authoritative agent/AI guide to this
repository: orientation, coding conventions, build commands, test instructions,
and architecture notes.

## Quick reference

| Task | Command |
|---|---|
| Build (JNI only, tests) | `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DIIDM_BRIDGE_ENABLE_JNI=ON -DIIDM_BRIDGE_ENABLE_GRAALVM=OFF -DIIDM_BRIDGE_BUILD_JAVA=OFF -DIIDM_BRIDGE_BUILD_NATIVE=OFF -DIIDM_BRIDGE_BUILD_TESTS=ON && cmake --build build --parallel` |
| Java module (and the jar) | `cd java && mvn clean verify` |
| Run tests | `IIDM_BRIDGE_JAR=$PWD/java/target/iidm-bridge-java-1.0.0-SNAPSHOT-fat.jar ctest --test-dir build --output-on-failure` |

The default options build everything: both backends, the jar (Maven) and the GraalVM
native image (`native-image` on the `PATH`). Without `IIDM_BRIDGE_JAR`, the tests on the
real JNI backend are skipped; the GraalVM backend's need `IIDM_BRIDGE_GRAALVM_LIB`
(AGENTS.md, Level 3).

## Key rules

- Property codes are defined once, in `include/iidm/PropertyCodes.def`
  (`IIDM_CODE(NAME, value)`); `PropertyCodes.h` and the generated Java
  `PropertyCodes` both come from it — never hand-write a code elsewhere.
- Port only current powsybl APIs: nothing marked `@Deprecated`, no extension from
  `extensions.removed` (the package is not released: no compatibility to keep).
- Setters must return `*this` for fluent chaining.
- Wrappers (`Generator`, `Terminal`...) are copyable: copies share one reference-counted
  handle on the same Java object. Only `Network`, which owns its backend, and the adders
  are move-only; keep their copy operations deleted.
- All C++ code lives in `namespace iidm`.
