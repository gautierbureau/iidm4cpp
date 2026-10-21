#pragma once

#include <iidm/Adder.h>
#include <iidm/IdentifiableShortCircuit.h>

namespace iidm {

// New IdentifiableShortCircuit extension of an identifiable
class IdentifiableShortCircuitAdder : public detail::AdderBase<IdentifiableShortCircuitAdder> {
public:
    IdentifiableShortCircuitAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    IdentifiableShortCircuitAdder& withIpMin(double ipMin) { return doubleField(prop::EXT_ISC_IP_MIN, ipMin); }
    IdentifiableShortCircuitAdder& withIpMax(double ipMax) { return doubleField(prop::EXT_ISC_IP_MAX, ipMax); }

    IdentifiableShortCircuit add() { return IdentifiableShortCircuit(create(), backend_); }
};

} // namespace iidm
