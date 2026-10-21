#include <iidm/PhaseTapChanger.h>
#include <iidm/BackendProvider.h>
#include "Wrap.h"

namespace iidm {

iidm::optional<Terminal> PhaseTapChanger::getRegulationTerminal() const {
    return detail::wrapOptional<Terminal>(backend_->getRelated(handle_, prop::REL_REGULATION_TERMINAL), backend_);
}

PhaseTapChanger& PhaseTapChanger::setRegulationTerminal(const Terminal& regulationTerminal) {
    backend_->setRelated(handle_, prop::REL_REGULATION_TERMINAL, detail::Access::handle(regulationTerminal));
    return *this;
}

} // namespace iidm
