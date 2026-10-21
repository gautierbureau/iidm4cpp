#include <iidm/ObservabilityArea.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

// ── ObservabilityArea::AreaCharacteristics ────────────────────────────────────

ObservabilityArea::AreaCharacteristics::AreaCharacteristics(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::vector<Terminal> ObservabilityArea::AreaCharacteristics::getTerminals() const {
    return detail::wrapAll<Terminal>(backend_->getChildren(handle_, prop::OBS_AREA_TERMINALS), backend_);
}

std::vector<int> ObservabilityArea::AreaCharacteristics::getNodes() const {
    return backend_->getIntList(handle_, prop::OBS_AREA_NODES);
}

std::vector<Bus> ObservabilityArea::AreaCharacteristics::getBuses() const {
    return detail::wrapAll<Bus>(backend_->getChildren(handle_, prop::OBS_AREA_BUSES), backend_);
}

// ── ObservabilityArea ─────────────────────────────────────────────────────────

ObservabilityArea::ObservabilityArea(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::vector<ObservabilityArea::AreaCharacteristics> ObservabilityArea::getObservabilityAreas() const {
    return detail::wrapAll<AreaCharacteristics>(backend_->getChildren(handle_, AreaCharacteristics::TYPE), backend_);
}

iidm::optional<ObservabilityArea::AreaCharacteristics> ObservabilityArea::getObservabilityAreaByNode(int node) const {
    return detail::wrapOptional<AreaCharacteristics>(backend_->getRelatedByIndex(handle_, prop::REL_OBS_AREA_BY_NODE, node), backend_);
}

iidm::optional<ObservabilityArea::AreaCharacteristics> ObservabilityArea::getObservabilityAreaByBus(const std::string& busId) const {
    return detail::wrapOptional<AreaCharacteristics>(backend_->getRelatedByString(handle_, prop::REL_OBS_AREA_BY_BUS, busId), backend_);
}

iidm::optional<ObservabilityArea::AreaCharacteristics> ObservabilityArea::getObservabilityAreaByBusBreakerBus(const std::string& busId) const {
    return detail::wrapOptional<AreaCharacteristics>(backend_->getRelatedByString(handle_, prop::REL_OBS_AREA_BY_BBV_BUS, busId), backend_);
}

} // namespace iidm
