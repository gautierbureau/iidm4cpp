#include <iidm/Component.h>
#include <iidm/BackendProvider.h>
#include <iidm/Bus.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

std::vector<Bus> Component::getBuses() const {
    return detail::wrapAll<Bus>(backend_->getChildren(handle_, prop::BUS), backend_);
}

} // namespace iidm
