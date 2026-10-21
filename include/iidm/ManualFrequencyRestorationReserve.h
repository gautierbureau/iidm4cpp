#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The ManualFrequencyRestorationReserve extension of an injection (mFRR): an object of its own,
// from Injection::getExtension<ManualFrequencyRestorationReserve>().
class ManualFrequencyRestorationReserve : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::MANUAL_FREQUENCY_RESTORATION_RESERVE;

    ManualFrequencyRestorationReserve() = default;
    explicit ManualFrequencyRestorationReserve(ObjectHandle handle, BackendProvider* backend);

    // Whether it takes part in the reserve
    bool isParticipate() const { return getBool(prop::EXT_MFRR_PARTICIPATE); }
    ManualFrequencyRestorationReserve& setParticipate(bool participate) {
        return setBool(*this, prop::EXT_MFRR_PARTICIPATE, participate);
    }
};

} // namespace iidm
