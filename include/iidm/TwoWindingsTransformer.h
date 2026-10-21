#pragma once

#include <iidm/Branch.h>
#include <iidm/Connectable.h>
#include <iidm/TapChangerHolder.h>
#include <iidm/TwoWindingsTransformerFortescue.h>
#include <iidm/TwoWindingsTransformerFortescueAdder.h>
#include <iidm/TwoWindingsTransformerPhaseAngleClock.h>
#include <iidm/TwoWindingsTransformerPhaseAngleClockAdder.h>
#include <iidm/TwoWindingsTransformerToBeEstimated.h>
#include <iidm/TwoWindingsTransformerToBeEstimatedAdder.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class Substation;

class TwoWindingsTransformer : public Connectable,
                               public detail::BranchMethods<TwoWindingsTransformer>,
                               public detail::TapChangerHolderMethods<TwoWindingsTransformer> {
public:
    static constexpr int TYPE = prop::TWO_WINDINGS_TRANSFORMER;

    TwoWindingsTransformer() = default;
    explicit TwoWindingsTransformer(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    // Empty when it is not in a substation
    iidm::optional<Substation> getSubstation() const;

    double getR() const;
    TwoWindingsTransformer& setR(double r);
    double getX() const;
    TwoWindingsTransformer& setX(double x);
    double getG() const;
    TwoWindingsTransformer& setG(double g);
    double getB() const;
    TwoWindingsTransformer& setB(double b);
    double getRatedU1() const;
    TwoWindingsTransformer& setRatedU1(double ratedU1);
    double getRatedU2() const;
    TwoWindingsTransformer& setRatedU2(double ratedU2);
    iidm::optional<double> getRatedS() const;
    // NaN: undefined
    TwoWindingsTransformer& setRatedS(double ratedS);

    // ── Solved values ────────────────────────────────────────────────────
    // The solved tap positions of its tap changers, when set, become their tap positions: of both
    // of them, of its ratio tap changer, of its phase tap changer
    void applySolvedValues();
    void setRatioTapPositionToSolvedTapPosition();
    void setPhaseTapPositionToSolvedTapPosition();

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasTwoWindingsTransformerToBeEstimated() const;
    TwoWindingsTransformerToBeEstimated getTwoWindingsTransformerToBeEstimated() const;
    TwoWindingsTransformerToBeEstimatedAdder newTwoWindingsTransformerToBeEstimated();
    void removeTwoWindingsTransformerToBeEstimated();

    bool hasTwoWindingsTransformerFortescue() const;
    TwoWindingsTransformerFortescue getTwoWindingsTransformerFortescue() const;
    TwoWindingsTransformerFortescueAdder newTwoWindingsTransformerFortescue();
    void removeTwoWindingsTransformerFortescue();

    bool hasTwoWindingsTransformerPhaseAngleClock() const;
    TwoWindingsTransformerPhaseAngleClock getTwoWindingsTransformerPhaseAngleClock() const;
    TwoWindingsTransformerPhaseAngleClockAdder newTwoWindingsTransformerPhaseAngleClock();
    void removeTwoWindingsTransformerPhaseAngleClock();
};

} // namespace iidm
