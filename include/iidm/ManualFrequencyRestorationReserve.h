#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The ManualFrequencyRestorationReserve extension of an injection (mFRR): an object of its own,
// from Injection::getManualFrequencyRestorationReserve().
class ManualFrequencyRestorationReserve : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::MANUAL_FREQUENCY_RESTORATION_RESERVE;

    ManualFrequencyRestorationReserve() = default;
    explicit ManualFrequencyRestorationReserve(ObjectHandle handle, BackendProvider* backend);

    // Whether it takes part in the reserve
    bool isParticipate() const;
    ManualFrequencyRestorationReserve& setParticipate(bool participate);
};

} // namespace iidm
