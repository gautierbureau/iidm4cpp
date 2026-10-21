#include <iidm/VariantManager.h>
#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

#ifndef __cpp_inline_variables
// From C++17 on, the header defines it
const char* const VariantManager::INITIAL_VARIANT_ID = "InitialState";
#endif

std::vector<std::string> VariantManager::getVariantIds() const {
    return backend_->getStringList(handle_, prop::VARIANT_IDS);
}

std::string VariantManager::getWorkingVariantId() const {
    return backend_->getString(handle_, prop::WORKING_VARIANT_ID);
}

VariantManager& VariantManager::setWorkingVariant(const std::string& variantId) {
    backend_->setString(handle_, prop::WORKING_VARIANT_ID, variantId);
    return *this;
}

void VariantManager::cloneVariant(const std::string& sourceVariantId, const std::vector<std::string>& targetVariantIds,
                                  bool mayOverwrite) {
    HandleRef clone(backend_->invoke(handle_, prop::VARIANT_CLONE), backend_);
    backend_->setString(clone, prop::VARIANT_SOURCE_ID, sourceVariantId);
    for (const std::string& targetVariantId : targetVariantIds) {
        backend_->setString(clone, prop::VARIANT_TARGET_ID, targetVariantId);
    }
    backend_->setBool(clone, prop::VARIANT_MAY_OVERWRITE, mayOverwrite);
    backend_->invoke(clone, prop::ACT_ADD);
}

void VariantManager::cloneVariant(const std::string& sourceVariantId, const std::string& targetVariantId,
                                  bool mayOverwrite) {
    cloneVariant(sourceVariantId, std::vector<std::string>(1, targetVariantId), mayOverwrite);
}

void VariantManager::removeVariant(const std::string& variantId) {
    backend_->setString(handle_, prop::REMOVE_VARIANT, variantId);
}

VariantManager& VariantManager::allowVariantMultiThreadAccess(bool allow) {
    backend_->setBool(handle_, prop::VARIANT_MULTI_THREAD_ACCESS, allow);
    return *this;
}

bool VariantManager::isVariantMultiThreadAccessAllowed() const {
    return backend_->getBool(handle_, prop::VARIANT_MULTI_THREAD_ACCESS);
}

} // namespace iidm
