#pragma once

#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <string>
#include <vector>

namespace iidm {

class Substation;

// An automation system of a substation: when the current of its monitored element exceeds a limit,
// it opens or closes a switch, or a side of a branch or three windings transformer (powsybl
// OverloadManagementSystem)
class OverloadManagementSystem : public Identifiable {
public:
    // One of its actions (powsybl OverloadManagementSystem.Tripping): at a current limit, open or
    // close an element. It is one of the three kinds below: as<SwitchTripping>()...
    class Tripping : public detail::ObjectRef, public detail::PropertiesHolder<Tripping> {
    public:
        // powsybl OverloadManagementSystem.Tripping.Type, same order
        enum class Type { BRANCH_TRIPPING, THREE_WINDINGS_TRANSFORMER_TRIPPING, SWITCH_TRIPPING };

        static constexpr int TYPE = prop::TRIPPING;

        Tripping() = default;
        explicit Tripping(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

        Type getType() const;
        std::string getKey() const;
        // Its name, or its key without one
        std::string getNameOrKey() const;
        Tripping& setName(const std::string& name);
        double getCurrentLimit() const;
        Tripping& setCurrentLimit(double currentLimit);
        // Whether it opens the element, else it closes it
        bool isOpenAction() const;
        Tripping& setOpenAction(bool openAction);
    };

    class SwitchTripping : public Tripping {
    public:
        static constexpr int TYPE = prop::SWITCH_TRIPPING;

        SwitchTripping() = default;
        explicit SwitchTripping(ObjectHandle handle, BackendProvider* backend) : Tripping(handle, backend) {}

        std::string getSwitchToOperateId() const;
        SwitchTripping& setSwitchToOperateId(const std::string& switchId);
    };

    class BranchTripping : public Tripping {
    public:
        static constexpr int TYPE = prop::BRANCH_TRIPPING;

        BranchTripping() = default;
        explicit BranchTripping(ObjectHandle handle, BackendProvider* backend) : Tripping(handle, backend) {}

        std::string getBranchToOperateId() const;
        BranchTripping& setBranchToOperateId(const std::string& branchId);
        TwoSides getSideToOperate() const;
        BranchTripping& setSideToOperate(TwoSides side);
    };

    class ThreeWindingsTransformerTripping : public Tripping {
    public:
        static constexpr int TYPE = prop::THREE_WINDINGS_TRANSFORMER_TRIPPING;

        ThreeWindingsTransformerTripping() = default;
        explicit ThreeWindingsTransformerTripping(ObjectHandle handle, BackendProvider* backend) : Tripping(handle, backend) {}

        std::string getThreeWindingsTransformerToOperateId() const;
        ThreeWindingsTransformerTripping& setThreeWindingsTransformerToOperateId(const std::string& transformerId);
        ThreeSides getSideToOperate() const;
        ThreeWindingsTransformerTripping& setSideToOperate(ThreeSides side);
    };

    static constexpr int TYPE = prop::OVERLOAD_MANAGEMENT_SYSTEM;

    OverloadManagementSystem() = default;
    explicit OverloadManagementSystem(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    bool isEnabled() const;
    void setEnabled(bool enabled);
    // Include <iidm/Substation.h> to use it
    Substation getSubstation() const;
    std::string getMonitoredElementId() const;
    ThreeSides getMonitoredSide() const;

    // In order
    std::vector<Tripping> getTrippings() const;
    void addTripping(const Tripping& tripping);

    void remove();
};

} // namespace iidm
