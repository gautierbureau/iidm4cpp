#pragma once

#include <iidm/ObjectHandle.h>
#include <vector>

namespace iidm {

class BackendProvider;

class ShuntCompensatorNonLinearModel {
public:
    struct Section {
        double b;
        double g;
    };

    ShuntCompensatorNonLinearModel() = default;
    explicit ShuntCompensatorNonLinearModel(ObjectHandle shuntHandle, BackendProvider* backend);

    std::vector<Section> getAllSections() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
