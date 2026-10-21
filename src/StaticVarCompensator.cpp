#include <iidm/StaticVarCompensator.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include "Extensions.h"

namespace iidm {

bool StaticVarCompensator::hasStandbyAutomaton() const {
    return detail::hasExtension<StandbyAutomaton>(backend_, handle_);
}

StandbyAutomaton StaticVarCompensator::getStandbyAutomaton() const {
    return detail::getExtension<StandbyAutomaton>(backend_, handle_, "StandbyAutomaton");
}

StandbyAutomatonAdder StaticVarCompensator::newStandbyAutomaton() {
    return detail::newExtension<StandbyAutomaton, StandbyAutomatonAdder>(backend_, handle_);
}

void StaticVarCompensator::removeStandbyAutomaton() {
    detail::removeExtension<StandbyAutomaton>(backend_, handle_);
}

} // namespace iidm
