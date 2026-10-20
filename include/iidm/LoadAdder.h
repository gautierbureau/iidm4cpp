#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/Load.h>

namespace iidm {

// New load of a voltage level (VoltageLevel::newLoad())
class LoadAdder : public detail::InjectionAdder<LoadAdder> {
public:
    LoadAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    LoadAdder& setLoadType(LoadType loadType) { return intField(prop::LOAD_TYPE, static_cast<int>(loadType)); }
    LoadAdder& setP0(double p0)               { return doubleField(prop::LOAD_P0, p0); }
    LoadAdder& setQ0(double q0)               { return doubleField(prop::LOAD_Q0, q0); }

    Load add() { return Load(create(), backend_); }
};

} // namespace iidm
