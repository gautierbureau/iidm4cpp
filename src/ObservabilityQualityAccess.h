#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/ObservabilityQuality.h>
#include <iidm/PropertyCodes.h>

namespace iidm { namespace detail {

// Reads / writes the observability quality block starting at `base` (see OBS_QUALITY_*_OFF).

inline iidm::optional<ObservabilityQuality> readQuality(const BackendProvider* backend, ObjectHandle h, int base) {
    if (!backend->getBool(h, base + prop::OBS_QUALITY_EXISTS_OFF)) return {};
    ObservabilityQuality quality;
    quality.standardDeviation = backend->getDouble(h, base + prop::OBS_QUALITY_STD_DEV_OFF);
    int redundant = backend->getInt(h, base + prop::OBS_QUALITY_REDUNDANT_OFF);
    if (redundant >= 0) quality.redundant = (redundant != 0);
    return quality;
}

// Sets the standard deviation, creating the quality if needed; an existing redundancy is kept.
inline void writeQuality(BackendProvider* backend, ObjectHandle h, int base, double standardDeviation) {
    backend->setDouble(h, base + prop::OBS_QUALITY_STD_DEV_OFF, standardDeviation);
}

// Sets both the standard deviation and the redundancy (empty = undefined).
inline void writeQuality(BackendProvider* backend, ObjectHandle h, int base, double standardDeviation,
                         const iidm::optional<bool>& redundant) {
    writeQuality(backend, h, base, standardDeviation);
    backend->setInt(h, base + prop::OBS_QUALITY_REDUNDANT_OFF, redundant ? (*redundant ? 1 : 0) : -1);
}

} } // namespace iidm::detail
