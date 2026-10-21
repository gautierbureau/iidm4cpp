#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PropertyCodes.h>
#include <initializer_list>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// The variants of a network (powsybl VariantManager): copies of its state (setpoints, flows,
// voltages, tap positions, switch positions...). Its objects show and change the working variant.
class VariantManager : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::VARIANT_MANAGER;
    // The variant of a new network (powsybl VariantManagerConstants.INITIAL_VARIANT_ID)
    static const char* const INITIAL_VARIANT_ID;

    VariantManager() = default;
    explicit VariantManager(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    // Sorted
    std::vector<std::string> getVariantIds() const;
    std::string getWorkingVariantId() const;
    // ValidationException when there is no such variant
    VariantManager& setWorkingVariant(const std::string& variantId);
    // Copies a variant into new ones. ValidationException when one exists already, unless
    // mayOverwrite: it is then replaced.
    void cloneVariant(const std::string& sourceVariantId, const std::vector<std::string>& targetVariantIds,
                      bool mayOverwrite = false);
    // For a braced list of ids, which would be ambiguous between the other two
    void cloneVariant(const std::string& sourceVariantId, std::initializer_list<std::string> targetVariantIds,
                      bool mayOverwrite = false) {
        cloneVariant(sourceVariantId, std::vector<std::string>(targetVariantIds), mayOverwrite);
    }
    void cloneVariant(const std::string& sourceVariantId, const std::string& targetVariantId,
                      bool mayOverwrite = false);
    // ValidationException for the initial variant. Removing the working variant leaves none: the
    // state of the objects can then neither be read nor changed until one is set.
    void removeVariant(const std::string& variantId);
    // Allowed, each thread has its own working variant (none for another thread until it sets one)
    VariantManager& allowVariantMultiThreadAccess(bool allow);
    bool isVariantMultiThreadAccessAllowed() const;
};

} // namespace iidm
