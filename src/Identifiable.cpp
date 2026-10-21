#include <iidm/Identifiable.h>
#include <iidm/BackendProvider.h>
#include <iidm/Network.h>
#include "Extensions.h"

namespace iidm {

std::string Identifiable::getId() const {
    return backend_->getString(handle_, prop::ID);
}

Identifiable& Identifiable::setId(const std::string& id) {
    backend_->setString(handle_, prop::ID, id);
    return *this;
}

Network Identifiable::getNetwork() const {
    return Network(backend_->getRelated(handle_, prop::NETWORK), backend_);
}

Network Identifiable::getParentNetwork() const {
    return Network(backend_->getRelated(handle_, prop::REL_PARENT_NETWORK), backend_);
}

std::string Identifiable::getNameOrId() const {
    return backend_->getString(handle_, prop::NAME);
}

iidm::optional<std::string> Identifiable::getOptionalName() const {
    return backend_->getOptString(handle_, prop::NAME);
}

Identifiable& Identifiable::setName(const std::string& name) {
    backend_->setString(handle_, prop::NAME, name);
    return *this;
}

bool Identifiable::isFictitious() const {
    return backend_->getBool(handle_, prop::FICTITIOUS);
}

Identifiable& Identifiable::setFictitious(bool fictitious) {
    backend_->setBool(handle_, prop::FICTITIOUS, fictitious);
    return *this;
}

IdentifiableType Identifiable::getType() const {
    return static_cast<IdentifiableType>(backend_->getInt(handle_, prop::IDENTIFIABLE_TYPE));
}

// ── Aliases ──────────────────────────────────────────────────────────────────

std::vector<std::string> Identifiable::getAliases() const {
    return backend_->getStringList(handle_, prop::ALIASES);
}

bool Identifiable::hasAliases() const {
    return !getAliases().empty();
}

iidm::optional<std::string> Identifiable::getAliasType(const std::string& alias) const {
    return backend_->getKeyedString(handle_, prop::ALIAS_TYPE, alias);
}

iidm::optional<std::string> Identifiable::getAliasFromType(const std::string& aliasType) const {
    return backend_->getKeyedString(handle_, prop::ALIAS_FROM_TYPE, aliasType);
}

Identifiable& Identifiable::addAlias(const std::string& alias, const std::string& aliasType, bool ensureAliasUnicity) {
    backend_->setKeyedString(handle_, ensureAliasUnicity ? prop::UNIQUE_ALIAS_TYPE : prop::ALIAS_TYPE, alias, aliasType);
    return *this;
}

Identifiable& Identifiable::removeAlias(const std::string& alias) {
    backend_->setKeyedString(handle_, prop::ALIAS_TYPE, alias, iidm::optional<std::string>());
    return *this;
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool Identifiable::hasDiscreteMeasurements() const {
    return detail::hasExtension<DiscreteMeasurements>(backend_, handle_);
}

DiscreteMeasurements Identifiable::getDiscreteMeasurements() const {
    return detail::getExtension<DiscreteMeasurements>(backend_, handle_, "DiscreteMeasurements");
}

DiscreteMeasurementsAdder Identifiable::newDiscreteMeasurements() {
    return detail::newExtension<DiscreteMeasurements, DiscreteMeasurementsAdder>(backend_, handle_);
}

void Identifiable::removeDiscreteMeasurements() {
    detail::removeExtension<DiscreteMeasurements>(backend_, handle_);
}

bool Identifiable::hasIdentifiableShortCircuit() const {
    return detail::hasExtension<IdentifiableShortCircuit>(backend_, handle_);
}

IdentifiableShortCircuit Identifiable::getIdentifiableShortCircuit() const {
    return detail::getExtension<IdentifiableShortCircuit>(backend_, handle_, "IdentifiableShortCircuit");
}

IdentifiableShortCircuitAdder Identifiable::newIdentifiableShortCircuit() {
    return detail::newExtension<IdentifiableShortCircuit, IdentifiableShortCircuitAdder>(backend_, handle_);
}

void Identifiable::removeIdentifiableShortCircuit() {
    detail::removeExtension<IdentifiableShortCircuit>(backend_, handle_);
}

bool Identifiable::hasOperatingStatus() const {
    return detail::hasExtension<OperatingStatus>(backend_, handle_);
}

OperatingStatus Identifiable::getOperatingStatus() const {
    return detail::getExtension<OperatingStatus>(backend_, handle_, "OperatingStatus");
}

OperatingStatusAdder Identifiable::newOperatingStatus() {
    return detail::newExtension<OperatingStatus, OperatingStatusAdder>(backend_, handle_);
}

void Identifiable::removeOperatingStatus() {
    detail::removeExtension<OperatingStatus>(backend_, handle_);
}

bool Identifiable::hasDynamicModelInfo() const {
    return detail::hasExtension<DynamicModelInfo>(backend_, handle_);
}

DynamicModelInfo Identifiable::getDynamicModelInfo() const {
    return detail::getExtension<DynamicModelInfo>(backend_, handle_, "DynamicModelInfo");
}

DynamicModelInfoAdder Identifiable::newDynamicModelInfo() {
    return detail::newExtension<DynamicModelInfo, DynamicModelInfoAdder>(backend_, handle_);
}

void Identifiable::removeDynamicModelInfo() {
    detail::removeExtension<DynamicModelInfo>(backend_, handle_);
}

} // namespace iidm
