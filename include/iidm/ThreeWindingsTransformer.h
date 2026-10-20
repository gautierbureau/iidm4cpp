#pragma once

#include <iidm/Connectable.h>
#include <iidm/CurrentLimits.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/RatioTapChanger.h>
#include <iidm/Terminal.h>
#include <iidm/ThreeWindingsTransformerToBeEstimated.h>
#include <iidm/stdcxx/optional.hpp>
#include <vector>

namespace iidm {

class Substation;

class ThreeWindingsTransformer : public Connectable {
public:
    // ── Leg ──────────────────────────────────────────────────────────────────
    class Leg : public detail::ObjectRef {
    public:
        Leg() = default;
        explicit Leg(ObjectHandle twtHandle, BackendProvider* backend,
                     int legBase, int relTerminal,
                     int rtcStepChildType, int ptcStepChildType);

        double getR() const;
        double getX() const;
        double getG() const;
        double getB() const;
        double getRatedU() const;
        iidm::optional<double> getRatedS() const;

        bool hasRatioTapChanger() const;
        RatioTapChanger getRatioTapChanger() const;

        bool hasPhaseTapChanger() const;
        PhaseTapChanger getPhaseTapChanger() const;

        Terminal getTerminal() const;

        iidm::optional<CurrentLimits> getCurrentLimits() const;

        std::vector<OperationalLimitsGroup> getOperationalLimitsGroups() const;
        iidm::optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup() const;

    private:
        int              legBase_         = 0;
        int              relTerminal_     = 0;
        int              rtcStepChildType_ = 0;
        int              ptcStepChildType_ = 0;
    };

    // ── ThreeWindingsTransformer ──────────────────────────────────────────────
    static constexpr int TYPE = prop::THREE_WINDINGS_TRANSFORMER;

    ThreeWindingsTransformer() = default;
    explicit ThreeWindingsTransformer(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    double     getRatedU0()    const;
    Substation getSubstation() const;

    Leg getLeg1() const;
    Leg getLeg2() const;
    Leg getLeg3() const;

    Terminal getTerminal1() const;
    Terminal getTerminal2() const;
    Terminal getTerminal3() const;

    // ── Extensions ───────────────────────────────────────────────────────
    bool hasThreeWindingsTransformerToBeEstimated() const;
    ThreeWindingsTransformerToBeEstimated getThreeWindingsTransformerToBeEstimated() const;
};

} // namespace iidm
