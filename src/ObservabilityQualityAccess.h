#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/ObservabilityQuality.h>
#include "Wrap.h"

namespace iidm { namespace detail {

// The qualities of an observability extension: `code` is the quantity's (OBS_QUALITY_P...).

inline iidm::optional<ObservabilityQuality> getQuality(BackendProvider* backend, ObjectHandle extension, int code) {
    return wrapOptional<ObservabilityQuality>(backend->getRelated(extension, code), backend);
}

// Sets the standard deviation, creating the quality if needed; an existing redundancy is kept.
inline void setQuality(BackendProvider* backend, ObjectHandle extension, int code, double standardDeviation) {
    backend->setDouble(extension, code, standardDeviation);
}

// Sets both the standard deviation and the redundancy (empty = undefined).
inline void setQuality(BackendProvider* backend, ObjectHandle extension, int code, double standardDeviation,
                       const iidm::optional<bool>& redundant) {
    setQuality(backend, extension, code, standardDeviation);
    ObservabilityQuality(backend->getRelated(extension, code), backend).setRedundant(redundant);
}

} } // namespace iidm::detail
