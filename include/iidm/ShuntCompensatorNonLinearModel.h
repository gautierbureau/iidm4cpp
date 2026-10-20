#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <vector>

namespace iidm {

class BackendProvider;

class ShuntCompensatorNonLinearModel : public detail::ObjectRef {
public:
    struct Section {
        double b;
        double g;
    };

    ShuntCompensatorNonLinearModel() = default;
    explicit ShuntCompensatorNonLinearModel(ObjectHandle shuntHandle, BackendProvider* backend);

    std::vector<Section> getAllSections() const;
};

} // namespace iidm
