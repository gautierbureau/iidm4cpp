#pragma once

#include <iidm/Adder.h>
#include <iidm/HvdcOperatorActivePowerRange.h>

namespace iidm {

// New HvdcOperatorActivePowerRange extension of an HVDC line (newHvdcOperatorActivePowerRange())
class HvdcOperatorActivePowerRangeAdder : public detail::AdderBase<HvdcOperatorActivePowerRangeAdder> {
public:
    HvdcOperatorActivePowerRangeAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    HvdcOperatorActivePowerRangeAdder& withOprFromCS1toCS2(double value) { return doubleField(prop::EXT_HOAR_OPR_CS1_TO_CS2, value); }
    HvdcOperatorActivePowerRangeAdder& withOprFromCS2toCS1(double value) { return doubleField(prop::EXT_HOAR_OPR_CS2_TO_CS1, value); }

    HvdcOperatorActivePowerRange add() { return HvdcOperatorActivePowerRange(create(), backend_); }
};

} // namespace iidm
