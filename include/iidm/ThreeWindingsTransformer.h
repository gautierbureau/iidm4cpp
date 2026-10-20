#pragma once

#include <iidm/Connectable.h>
#include <iidm/Enums.h>
#include <iidm/FlowsLimitsHolder.h>
#include <iidm/TapChangerHolder.h>
#include <iidm/Terminal.h>
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
        ThreeSides getSide() const;
        ThreeWindingsTransformer getTransformer() const;

        double getR() const;
        Leg& setR(double r);

        double getX() const;
        Leg& setX(double x);

        double getG() const;
        Leg& setG(double g);

        double getB() const;
        Leg& setB(double b);

        double getRatedU() const;
        Leg& setRatedU(double ratedU);

        iidm::optional<double> getRatedS() const;
        Leg& setRatedS(double ratedS);
    };

    static constexpr int TYPE = prop::THREE_WINDINGS_TRANSFORMER;

    ThreeWindingsTransformer() = default;
    explicit ThreeWindingsTransformer(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    double     getRatedU0()    const;
    Substation getSubstation() const;

    Leg getLeg1() const;
    Leg getLeg2() const;
    Leg getLeg3() const;
    Leg getLeg(ThreeSides side) const;
    // Leg 1, 2, 3
    std::vector<Leg> getLegs() const;

    Terminal getTerminal(ThreeSides side) const;
    // The terminal in this voltage level
    Terminal getTerminal(const std::string& voltageLevelId) const;

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasThreeWindingsTransformerToBeEstimated() const;
    ThreeWindingsTransformerToBeEstimated getThreeWindingsTransformerToBeEstimated() const;
    ThreeWindingsTransformerToBeEstimatedAdder newThreeWindingsTransformerToBeEstimated();
    void removeThreeWindingsTransformerToBeEstimated();
};

} // namespace iidm
