#pragma once

#include <iidm/Adder.h>
#include <iidm/CoordinatedReactiveControl.h>

namespace iidm {

// New CoordinatedReactiveControl extension of a generator
class CoordinatedReactiveControlAdder : public detail::AdderBase<CoordinatedReactiveControlAdder> {
public:
    CoordinatedReactiveControlAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    CoordinatedReactiveControlAdder& withQPercent(double qPercent) { return doubleField(prop::EXT_CRC_Q_PERCENT, qPercent); }

    CoordinatedReactiveControl add() { return CoordinatedReactiveControl(create(), backend_); }
};

} // namespace iidm
