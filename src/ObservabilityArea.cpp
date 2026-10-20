#include <iidm/ObservabilityArea.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

// ── ObservabilityArea::AreaCharacteristics ────────────────────────────────────

ObservabilityArea::AreaCharacteristics::AreaCharacteristics(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle, backend), backend_(detail::orNullBackend(backend)) {}

int ObservabilityArea::AreaCharacteristics::getAreaNumber() const {
    return backend_->getInt(handle_, prop::OBS_AREA_NUMBER);
}

ObservabilityStatus ObservabilityArea::AreaCharacteristics::getStatus() const {
    return static_cast<ObservabilityStatus>(backend_->getInt(handle_, prop::OBS_AREA_STATUS));
}

std::vector<Terminal> ObservabilityArea::AreaCharacteristics::getTerminals() const {
    auto handles = backend_->getChildren(handle_, prop::OBS_AREA_TERMINALS);
    std::vector<Terminal> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<int> ObservabilityArea::AreaCharacteristics::getNodes() const {
    return backend_->getIntList(handle_, prop::OBS_AREA_NODES);
}

std::vector<Bus> ObservabilityArea::AreaCharacteristics::getBuses() const {
    auto handles = backend_->getChildren(handle_, prop::OBS_AREA_BUSES);
    std::vector<Bus> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

// ── ObservabilityArea ─────────────────────────────────────────────────────────

namespace {

iidm::optional<ObservabilityArea::AreaCharacteristics> toArea(ObjectHandle h, BackendProvider* backend) {
    if (h == INVALID_HANDLE) return {};
    return ObservabilityArea::AreaCharacteristics(h, backend);
}

} // namespace

ObservabilityArea::ObservabilityArea(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle, backend), backend_(detail::orNullBackend(backend)) {}

bool ObservabilityArea::isConsistentWithTopology() const {
    return backend_->getBool(handle_, prop::EXT_OBS_AREA_CONSISTENT);
}

std::vector<ObservabilityArea::AreaCharacteristics> ObservabilityArea::getObservabilityAreas() const {
    auto handles = backend_->getChildren(handle_, prop::OBSERVABILITY_AREA);
    std::vector<AreaCharacteristics> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

iidm::optional<ObservabilityArea::AreaCharacteristics> ObservabilityArea::getObservabilityAreaByNode(int node) const {
    return toArea(backend_->getRelatedByIndex(handle_, prop::REL_OBS_AREA_BY_NODE, node), backend_);
}

iidm::optional<ObservabilityArea::AreaCharacteristics> ObservabilityArea::getObservabilityAreaByBus(const std::string& busId) const {
    return toArea(backend_->getRelatedByString(handle_, prop::REL_OBS_AREA_BY_BUS, busId), backend_);
}

iidm::optional<ObservabilityArea::AreaCharacteristics> ObservabilityArea::getObservabilityAreaByBusBreakerBus(const std::string& busId) const {
    return toArea(backend_->getRelatedByString(handle_, prop::REL_OBS_AREA_BY_BBV_BUS, busId), backend_);
}

bool ObservabilityArea::operator==(const ObservabilityArea& other) const {
    return handle_ == other.handle_;
}

bool ObservabilityArea::operator!=(const ObservabilityArea& other) const {
    return !(*this == other);
}

} // namespace iidm
