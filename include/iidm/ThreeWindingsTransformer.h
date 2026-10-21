#pragma once

#include <iidm/Connectable.h>
#include <iidm/Enums.h>
#include <iidm/FlowsLimitsHolder.h>
#include <iidm/Overload.h>
#include <iidm/TapChangerHolder.h>
#include <iidm/Terminal.h>
#include <iidm/ThreeWindingsTransformerFortescue.h>
#include <iidm/ThreeWindingsTransformerFortescueAdder.h>
#include <iidm/ThreeWindingsTransformerPhaseAngleClock.h>
#include <iidm/ThreeWindingsTransformerPhaseAngleClockAdder.h>
#include <iidm/ThreeWindingsTransformerToBeEstimated.h>
#include <iidm/ThreeWindingsTransformerToBeEstimatedAdder.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class Substation;

class ThreeWindingsTransformer : public Connectable {
public:
    // One of its windings, with its terminal, tap changers and limits
    class Leg : public detail::ObjectRef,
                public detail::TapChangerHolderMethods<Leg>,
                public detail::FlowsLimitsHolderMethods<Leg> {
    public:
        static constexpr int TYPE = prop::THREE_WINDINGS_TRANSFORMER_LEG;

        Leg() = default;
        explicit Leg(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

        Terminal getTerminal() const;
        ThreeSides getSide() const { return static_cast<ThreeSides>(getInt(prop::LEG_SIDE)); }
        ThreeWindingsTransformer getTransformer() const;

        double getR() const { return getDouble(prop::LEG_R); }
        Leg& setR(double r) { return setDouble(*this, prop::LEG_R, r); }

        double getX() const { return getDouble(prop::LEG_X); }
        Leg& setX(double x) { return setDouble(*this, prop::LEG_X, x); }

        double getG() const { return getDouble(prop::LEG_G); }
        Leg& setG(double g) { return setDouble(*this, prop::LEG_G, g); }

        double getB() const { return getDouble(prop::LEG_B); }
        Leg& setB(double b) { return setDouble(*this, prop::LEG_B, b); }

        double getRatedU() const { return getDouble(prop::LEG_RATED_U); }
        Leg& setRatedU(double ratedU) { return setDouble(*this, prop::LEG_RATED_U, ratedU); }

        iidm::optional<double> getRatedS() const { return getOptDouble(prop::LEG_RATED_S); }
        Leg& setRatedS(double ratedS) { return setDouble(*this, prop::LEG_RATED_S, ratedS); }

        // Limits of a type (current, active power or apparent power): of the selected group, of
        // every selected group
        iidm::optional<LoadingLimits> getLimits(LimitType type) const { return limits(type); }
        std::vector<LoadingLimits> getAllSelectedLimits(LimitType type) const { return allSelectedLimits(type); }
    };

    static constexpr int TYPE = prop::THREE_WINDINGS_TRANSFORMER;

    ThreeWindingsTransformer() = default;
    explicit ThreeWindingsTransformer(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    // Empty when it is not in a substation
    iidm::optional<Substation> getSubstation() const;

    double getRatedU0() const { return getDouble(prop::THREE_WT_RATED_U0); }
    ThreeWindingsTransformer& setRatedU0(double ratedU0) { return setDouble(*this, prop::THREE_WT_RATED_U0, ratedU0); }

    Leg getLeg1() const;
    Leg getLeg2() const;
    Leg getLeg3() const;
    Leg getLeg(ThreeSides side) const;
    // Leg 1, 2, 3
    std::vector<Leg> getLegs() const;

    Terminal getTerminal(ThreeSides side) const;
    // The terminal in this voltage level
    Terminal getTerminal(const std::string& voltageLevelId) const;
    // The side of one of its terminals (ValidationException for another terminal)
    ThreeSides getSide(const Terminal& terminal) const;

    // ── Solved values ────────────────────────────────────────────────────
    // The solved tap positions of the tap changers of its legs, when set, become their tap
    // positions: of all of them, of the ratio tap changers, of the phase tap changers
    void applySolvedValues();
    void setRatioTapPositionToSolvedTapPosition();
    void setPhaseTapPositionToSolvedTapPosition();

    // ── Overloads: a flow (current, active or apparent power) over the limits in force; with a
    // limit reduction, over these limits multiplied by it ──────────────────────────────────────
    // Whether the current is over the permanent limit, on any side
    bool isOverloaded() const { return getBool(prop::OVERLOADED); }
    bool isOverloaded(double limitReductionValue) const;
    // The shortest acceptable duration of the overloads of the current (seconds; INT_MAX without any)
    int getOverloadDuration() const { return getInt(prop::OVERLOAD_DURATION); }
    bool checkPermanentLimit(ThreeSides side, LimitType type) const;
    bool checkPermanentLimit(ThreeSides side, double limitReductionValue, LimitType type) const;
    bool checkPermanentLimit1(LimitType type) const;
    bool checkPermanentLimit1(double limitReductionValue, LimitType type) const;
    bool checkPermanentLimit2(LimitType type) const;
    bool checkPermanentLimit2(double limitReductionValue, LimitType type) const;
    bool checkPermanentLimit3(LimitType type) const;
    bool checkPermanentLimit3(double limitReductionValue, LimitType type) const;
    // The overload of a side (empty without one), the overloads of every selected group of a side
    iidm::optional<Overload> checkTemporaryLimits(ThreeSides side, LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits(ThreeSides side, double limitReductionValue, LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits1(LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits1(double limitReductionValue, LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits2(LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits2(double limitReductionValue, LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits3(LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits3(double limitReductionValue, LimitType type) const;
    std::vector<Overload> checkAllTemporaryLimits(ThreeSides side, LimitType type) const;
    std::vector<Overload> checkAllTemporaryLimits(ThreeSides side, double limitReductionValue, LimitType type) const;

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasThreeWindingsTransformerToBeEstimated() const;
    ThreeWindingsTransformerToBeEstimated getThreeWindingsTransformerToBeEstimated() const;
    ThreeWindingsTransformerToBeEstimatedAdder newThreeWindingsTransformerToBeEstimated();
    void removeThreeWindingsTransformerToBeEstimated();

    bool hasThreeWindingsTransformerPhaseAngleClock() const;
    ThreeWindingsTransformerPhaseAngleClock getThreeWindingsTransformerPhaseAngleClock() const;
    ThreeWindingsTransformerPhaseAngleClockAdder newThreeWindingsTransformerPhaseAngleClock();
    void removeThreeWindingsTransformerPhaseAngleClock();

    bool hasThreeWindingsTransformerFortescue() const;
    ThreeWindingsTransformerFortescue getThreeWindingsTransformerFortescue() const;
    ThreeWindingsTransformerFortescueAdder newThreeWindingsTransformerFortescue();
    void removeThreeWindingsTransformerFortescue();
};

} // namespace iidm
