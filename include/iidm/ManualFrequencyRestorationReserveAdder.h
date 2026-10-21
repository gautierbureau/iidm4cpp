#pragma once

#include <iidm/Adder.h>
#include <iidm/ManualFrequencyRestorationReserve.h>

namespace iidm {

// New ManualFrequencyRestorationReserve extension of an injection (newManualFrequencyRestorationReserve())
class ManualFrequencyRestorationReserveAdder : public detail::AdderBase<ManualFrequencyRestorationReserveAdder> {
public:
    ManualFrequencyRestorationReserveAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    ManualFrequencyRestorationReserveAdder& withParticipate(bool participate) { return boolField(prop::EXT_MFRR_PARTICIPATE, participate); }

    ManualFrequencyRestorationReserve add() { return ManualFrequencyRestorationReserve(create(), backend_); }
};

} // namespace iidm
