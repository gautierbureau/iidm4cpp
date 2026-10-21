#pragma once

#include <iidm/Adder.h>
#include <iidm/LoadAsymmetrical.h>

namespace iidm {

// New LoadAsymmetrical extension of a load
class LoadAsymmetricalAdder : public detail::AdderBase<LoadAsymmetricalAdder> {
public:
    LoadAsymmetricalAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    LoadAsymmetricalAdder& withConnectionType(LoadConnectionType connectionType) { return intField(prop::EXT_LA_CONNECTION_TYPE, static_cast<int>(connectionType)); }
    LoadAsymmetricalAdder& withDeltaPa(double deltaPa) { return doubleField(prop::EXT_LA_DELTA_PA, deltaPa); }
    LoadAsymmetricalAdder& withDeltaQa(double deltaQa) { return doubleField(prop::EXT_LA_DELTA_QA, deltaQa); }
    LoadAsymmetricalAdder& withDeltaPb(double deltaPb) { return doubleField(prop::EXT_LA_DELTA_PB, deltaPb); }
    LoadAsymmetricalAdder& withDeltaQb(double deltaQb) { return doubleField(prop::EXT_LA_DELTA_QB, deltaQb); }
    LoadAsymmetricalAdder& withDeltaPc(double deltaPc) { return doubleField(prop::EXT_LA_DELTA_PC, deltaPc); }
    LoadAsymmetricalAdder& withDeltaQc(double deltaQc) { return doubleField(prop::EXT_LA_DELTA_QC, deltaQc); }

    LoadAsymmetrical add() { return LoadAsymmetrical(create(), backend_); }
};

} // namespace iidm
