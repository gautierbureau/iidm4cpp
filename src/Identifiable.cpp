#include <iidm/Identifiable.h>
#include <iidm/BackendProvider.h>
#include <iidm/Network.h>
#include <cctype>

namespace iidm {

Network Identifiable::getNetwork() const {
    return Network(backend_->getRelated(handle_, prop::NETWORK), backend_);
}

Network Identifiable::getParentNetwork() const {
    return Network(backend_->getRelated(handle_, prop::REL_PARENT_NETWORK), backend_);
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

namespace {

// The class of a type code: ActivePowerControl for ACTIVE_POWER_CONTROL
std::string className(int type) {
    const char* code = "";
    switch (type) {
#define IIDM_CODE(NAME, value) case value: code = #NAME; break;
#include <iidm/PropertyCodes.def>
#undef IIDM_CODE
    }
    std::string name;
    bool word = true;
    for (const char* c = code; *c != '\0'; ++c) {
        if (*c == '_') {
            word = true;
            continue;
        }
        name += word ? *c : static_cast<char>(std::tolower(static_cast<unsigned char>(*c)));
        word = false;
    }
    return name;
}

} // namespace

void Identifiable::extensionNotFound(int type) const {
    throw ExtensionNotFoundException(className(type) + " extension not present on " + getId());
}

} // namespace iidm
