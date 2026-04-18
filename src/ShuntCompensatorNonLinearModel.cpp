#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ShuntCompensatorNonLinearModel::ShuntCompensatorNonLinearModel(ObjectHandle shuntHandle,
                                                               BackendProvider* backend)
    : handle_(shuntHandle), backend_(backend) {}

std::vector<ShuntCompensatorNonLinearModel::Section>
ShuntCompensatorNonLinearModel::getAllSections() const {
    auto handles = backend_->getChildren(handle_, prop::SHUNT_SECTION);
    std::vector<Section> result;
    result.reserve(handles.size());
    for (auto h : handles) {
        Section s;
        s.b = backend_->getDouble(h, prop::SHUNT_SECTION_B);
        s.g = backend_->getDouble(h, prop::SHUNT_SECTION_G);
        result.push_back(s);
    }
    return result;
}

} // namespace iidm
