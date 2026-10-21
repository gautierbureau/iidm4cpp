#include <iidm/ShuntCompensatorModel.h>
#include <iidm/BackendProvider.h>
#include <iidm/ShuntCompensatorLinearModel.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include "Wrap.h"

namespace iidm {

// ── ShuntCompensatorLinearModel ──────────────────────────────────────────────

// ── ShuntCompensatorNonLinearModel ───────────────────────────────────────────

std::vector<ShuntCompensatorNonLinearModel::Section> ShuntCompensatorNonLinearModel::getAllSections() const {
    return detail::wrapAll<Section>(backend_->getChildren(handle_, Section::TYPE), backend_);
}

} // namespace iidm
