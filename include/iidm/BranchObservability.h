#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/ObservabilityQuality.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class BackendProvider;

// The BranchObservability extension of a branch (line, two windings transformer): an object of
// its own, from getBranchObservability() of the branch.
class BranchObservability : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::BRANCH_OBSERVABILITY;

    BranchObservability() = default;
    explicit BranchObservability(ObjectHandle handle, BackendProvider* backend);

    bool isObservable() const;
    BranchObservability& setObservable(bool observable);

    // Empty when the quality is not defined
    iidm::optional<ObservabilityQuality> getQualityP1() const;
    iidm::optional<ObservabilityQuality> getQualityP2() const;
    iidm::optional<ObservabilityQuality> getQualityQ1() const;
    iidm::optional<ObservabilityQuality> getQualityQ2() const;

    // Sets the standard deviation, creating the quality if needed (an existing redundancy is kept)
    BranchObservability& setQualityP1(double standardDeviation);
    BranchObservability& setQualityP2(double standardDeviation);
    BranchObservability& setQualityQ1(double standardDeviation);
    BranchObservability& setQualityQ2(double standardDeviation);

    // Sets the standard deviation and the redundancy (empty = undefined)
    BranchObservability& setQualityP1(double standardDeviation, iidm::optional<bool> redundant);
    BranchObservability& setQualityP2(double standardDeviation, iidm::optional<bool> redundant);
    BranchObservability& setQualityQ1(double standardDeviation, iidm::optional<bool> redundant);
    BranchObservability& setQualityQ2(double standardDeviation, iidm::optional<bool> redundant);
};

} // namespace iidm
