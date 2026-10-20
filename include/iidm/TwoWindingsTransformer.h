#pragma once

#include <iidm/Branch.h>
#include <iidm/Connectable.h>
#include <iidm/Enums.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/RatioTapChanger.h>
#include <iidm/TwoWindingsTransformerToBeEstimated.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class TwoWindingsTransformer : public Connectable, public detail::BranchMethods<TwoWindingsTransformer> {
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

    // ── RatioTapChanger ──────────────────────────────────────────────────────
    bool hasRatioTapChanger() const;
    iidm::optional<RatioTapChanger> getRatioTapChanger() const;
    int  getRatioTapPosition() const;
    TwoWindingsTransformer& setRatioTapPosition(int pos);
    int  getRatioTapLow() const;
    int  getRatioTapHigh() const;
    bool isRatioTapRegulating() const;
    TwoWindingsTransformer& setRatioTapRegulating(bool regulating);
    double getRatioTapTargetV() const;
    TwoWindingsTransformer& setRatioTapTargetV(double v);

    // ── PhaseTapChanger ──────────────────────────────────────────────────────
    bool hasPhaseTapChanger() const;
    iidm::optional<PhaseTapChanger> getPhaseTapChanger() const;
    int  getPhaseTapPosition() const;
    TwoWindingsTransformer& setPhaseTapPosition(int pos);
    int  getPhaseTapLow() const;
    int  getPhaseTapHigh() const;
    bool isPhaseTapRegulating() const;
    TwoWindingsTransformer& setPhaseTapRegulating(bool regulating);
    PhaseTapChangerRegulationMode getPhaseTapRegulationMode() const;
    TwoWindingsTransformer& setPhaseTapRegulationMode(PhaseTapChangerRegulationMode mode);
    double getPhaseTapRegulationValue() const;
    TwoWindingsTransformer& setPhaseTapRegulationValue(double val);

    // ── Extensions ───────────────────────────────────────────────────────
    bool hasTwoWindingsTransformerToBeEstimated() const;
    TwoWindingsTransformerToBeEstimated getTwoWindingsTransformerToBeEstimated() const;
};

} // namespace iidm
