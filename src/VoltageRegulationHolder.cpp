#include <iidm/VoltageRegulationHolder.h>
#include <iidm/BackendProvider.h>
#include <iidm/Battery.h>
#include <iidm/Generator.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/VscConverterStation.h>
#include "Wrap.h"
#include <algorithm>

namespace iidm { namespace detail {

template <class Self>
double VoltageRegulationHolderMethods<Self>::getLocalTargetV() const {
    return backend()->getDouble(handle(), prop::LOCAL_TARGET_V);
}

template <class Self>
Self& VoltageRegulationHolderMethods<Self>::setLocalTargetV(double localTargetV) {
    backend()->setDouble(handle(), prop::LOCAL_TARGET_V, localTargetV);
    return static_cast<Self&>(*this);
}

template <class Self>
double VoltageRegulationHolderMethods<Self>::getLocalTargetQ() const {
    return backend()->getDouble(handle(), prop::LOCAL_TARGET_Q);
}

template <class Self>
Self& VoltageRegulationHolderMethods<Self>::setLocalTargetQ(double localTargetQ) {
    backend()->setDouble(handle(), prop::LOCAL_TARGET_Q, localTargetQ);
    return static_cast<Self&>(*this);
}

template <class Self>
iidm::optional<VoltageRegulation> VoltageRegulationHolderMethods<Self>::getVoltageRegulation() const {
    return wrapOptional<VoltageRegulation>(backend()->getRelated(handle(), prop::VOLTAGE_REGULATION), backend());
}

template <class Self>
VoltageRegulationBuilder VoltageRegulationHolderMethods<Self>::newVoltageRegulation() {
    return VoltageRegulationBuilder(backend()->invoke(handle(), prop::VOLTAGE_REGULATION), backend());
}

template <class Self>
void VoltageRegulationHolderMethods<Self>::removeVoltageRegulation() {
    backend()->setRelated(handle(), prop::VOLTAGE_REGULATION, INVALID_HANDLE);
}

template <class Self>
bool VoltageRegulationHolderMethods<Self>::isRegulating() const {
    return backend()->getBool(handle(), prop::REGULATING);
}

namespace {

bool contains(const std::vector<int>& modes, RegulationMode mode) {
    return std::find(modes.begin(), modes.end(), static_cast<int>(mode)) != modes.end();
}

} // namespace

template <class Self>
bool VoltageRegulationHolderMethods<Self>::isRegulatingWithMode(RegulationMode mode) const {
    return contains(backend()->getIntList(handle(), prop::REGULATING_MODES), mode);
}

template <class Self>
bool VoltageRegulationHolderMethods<Self>::isWithMode(RegulationMode mode) const {
    return contains(backend()->getIntList(handle(), prop::WITH_MODES), mode);
}

template <class Self>
double VoltageRegulationHolderMethods<Self>::getRegulatingTargetV() const {
    return backend()->getDouble(handle(), prop::REGULATING_TARGET_V);
}

template <class Self>
double VoltageRegulationHolderMethods<Self>::getRegulatingTargetQ() const {
    return backend()->getDouble(handle(), prop::REGULATING_TARGET_Q);
}

template <class Self>
Terminal VoltageRegulationHolderMethods<Self>::getRegulatingTerminal() const {
    return Terminal(backend()->getRelated(handle(), prop::REL_REGULATING_TERMINAL), backend());
}

template <class Self>
bool VoltageRegulationHolderMethods<Self>::hasRegulatingTerminal() const {
    return backend()->getBool(handle(), prop::HAS_REGULATING_TERMINAL);
}

template <class Self>
void VoltageRegulationHolderMethods<Self>::setTargetQToQ() {
    backend()->invoke(handle(), prop::ACT_SET_TARGET_Q_TO_Q);
}

template <class Self>
void VoltageRegulationHolderMethods<Self>::setTargetVToV() {
    backend()->invoke(handle(), prop::ACT_SET_TARGET_V_TO_V);
}

template class VoltageRegulationHolderMethods<Generator>;
template class VoltageRegulationHolderMethods<Battery>;
template class VoltageRegulationHolderMethods<StaticVarCompensator>;
template class VoltageRegulationHolderMethods<ShuntCompensator>;
template class VoltageRegulationHolderMethods<VscConverterStation>;

} } // namespace iidm::detail
