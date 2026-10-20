#pragma once

#include <iidm/Branch.h>
#include <iidm/Connectable.h>
#include <iidm/TapChangerHolder.h>
#include <iidm/TwoWindingsTransformerToBeEstimated.h>
#include <iidm/TwoWindingsTransformerToBeEstimatedAdder.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class TwoWindingsTransformer : public Connectable,
                               public detail::BranchMethods<TwoWindingsTransformer>,
                               public detail::TapChangerHolderMethods<TwoWindingsTransformer> {
public:
    static constexpr int TYPE = prop::TWO_WINDINGS_TRANSFORMER;

    TwoWindingsTransformer() = default;
    explicit TwoWindingsTransformer(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    double getR() const;
    double getX() const;
    double getG() const;
    double getB() const;
    double getRatedU1() const;
    double getRatedU2() const;
    iidm::optional<double> getRatedS() const;

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasTwoWindingsTransformerToBeEstimated() const;
    TwoWindingsTransformerToBeEstimated getTwoWindingsTransformerToBeEstimated() const;
    TwoWindingsTransformerToBeEstimatedAdder newTwoWindingsTransformerToBeEstimated();
    void removeTwoWindingsTransformerToBeEstimated();
};

} // namespace iidm
