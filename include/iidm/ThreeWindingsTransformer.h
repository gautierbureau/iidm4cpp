#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/CurrentLimits.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/Enums.h>
#include <iidm/RatioTapChanger.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/Substation.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

class ThreeWindingsTransformer {
public:
    // ── Leg ──────────────────────────────────────────────────────────────────
    class Leg {
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

        bool isValid() const { return handle_ != INVALID_HANDLE; }

    private:
        ObjectHandle     handle_          = INVALID_HANDLE;
        BackendProvider* backend_         = nullptr;
        int              legBase_         = 0;
        int              relTerminal_     = 0;
        int              rtcStepChildType_ = 0;
        int              ptcStepChildType_ = 0;
    };

    // ── ThreeWindingsTransformer ──────────────────────────────────────────────
    ThreeWindingsTransformer() = default;
    explicit ThreeWindingsTransformer(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double     getRatedU0()    const;
    Substation getSubstation() const;

    Leg getLeg1() const;
    Leg getLeg2() const;
    Leg getLeg3() const;

    Terminal getTerminal1() const;
    Terminal getTerminal2() const;
    Terminal getTerminal3() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ThreeWindingsTransformer& other) const;
    bool operator!=(const ThreeWindingsTransformer& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
