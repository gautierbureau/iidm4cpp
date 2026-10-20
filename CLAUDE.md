# CLAUDE.md

See **[AGENTS.md](AGENTS.md)** for the authoritative agent/AI guide to this
repository: orientation, coding conventions, build commands, test instructions,
and architecture notes.

## Quick reference

| Task | Command |
|---|---|
| Build (JNI, tests) | `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DIIDM_BRIDGE_ENABLE_JNI=ON -DIIDM_BRIDGE_BUILD_TESTS=ON && cmake --build build --parallel` |
| Run tests | `ctest --test-dir build --output-on-failure` |
| Java module | `cd java && mvn clean verify` |

## Key rules

- Property codes are defined once, in `include/iidm/PropertyCodes.def`
  (`IIDM_CODE(NAME, value)`); `PropertyCodes.h` and the generated Java
  `PropertyCodes` both come from it — never hand-write a code elsewhere.
- Port only current powsybl APIs: nothing marked `@Deprecated`, no extension from
  `extensions.removed` (the package is not released: no compatibility to keep).
- Setters must return `*this` for fluent chaining.
- Component objects are move-only; never add copy constructors.
- All C++ code lives in `namespace iidm`.
