#pragma once

#include <iidm/Adder.h>
#include <iidm/OperatingStatus.h>

namespace iidm {

// New OperatingStatus extension of a busbar section, a line, a tie line, a transformer, an HVDC
// line or a boundary line
class OperatingStatusAdder : public detail::AdderBase<OperatingStatusAdder> {
public:
    OperatingStatusAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    OperatingStatusAdder& withStatus(OperatingStatus::Status status) { return intField(prop::EXT_OS_STATUS, static_cast<int>(status)); }

    OperatingStatus add() { return OperatingStatus(create(), backend_); }
};

} // namespace iidm
