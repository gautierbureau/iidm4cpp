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

- Keep `include/iidm/PropertyCodes.h` and `java/.../PropertyCodes.java` in sync
  (same integer constants in both files).
- Setters must return `*this` for fluent chaining.
- Component objects are move-only; never add copy constructors.
- All C++ code lives in `namespace iidm`.
