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

        Type getType() const { return static_cast<Type>(getInt(prop::TRIPPING_TYPE)); }
        std::string getKey() const { return getString(prop::TRIPPING_KEY); }
        // Its name, or its key without one
        std::string getNameOrKey() const { return getString(prop::TRIPPING_NAME); }
        Tripping& setName(const std::string& name) { return setString(*this, prop::TRIPPING_NAME, name); }
        double getCurrentLimit() const { return getDouble(prop::TRIPPING_CURRENT_LIMIT); }
        Tripping& setCurrentLimit(double currentLimit) {
            return setDouble(*this, prop::TRIPPING_CURRENT_LIMIT, currentLimit);
        }
        // Whether it opens the element, else it closes it
        bool isOpenAction() const { return getBool(prop::TRIPPING_OPEN_ACTION); }
        Tripping& setOpenAction(bool openAction) { return setBool(*this, prop::TRIPPING_OPEN_ACTION, openAction); }
    };

    class SwitchTripping : public Tripping {
    public:
        static constexpr int TYPE = prop::SWITCH_TRIPPING;

        SwitchTripping() = default;
        explicit SwitchTripping(ObjectHandle handle, BackendProvider* backend) : Tripping(handle, backend) {}

        std::string getSwitchToOperateId() const { return getString(prop::TRIPPING_ELEMENT_ID); }
        SwitchTripping& setSwitchToOperateId(const std::string& switchId) {
            return setString(*this, prop::TRIPPING_ELEMENT_ID, switchId);
        }
    };

    class BranchTripping : public Tripping {
    public:
        static constexpr int TYPE = prop::BRANCH_TRIPPING;

        BranchTripping() = default;
        explicit BranchTripping(ObjectHandle handle, BackendProvider* backend) : Tripping(handle, backend) {}

        std::string getBranchToOperateId() const { return getString(prop::TRIPPING_ELEMENT_ID); }
        BranchTripping& setBranchToOperateId(const std::string& branchId) {
            return setString(*this, prop::TRIPPING_ELEMENT_ID, branchId);
        }
        TwoSides getSideToOperate() const { return static_cast<TwoSides>(getInt(prop::TRIPPING_SIDE)); }
        BranchTripping& setSideToOperate(TwoSides side) {
            return setInt(*this, prop::TRIPPING_SIDE, static_cast<int>(side));
        }
    };

    class ThreeWindingsTransformerTripping : public Tripping {
    public:
        static constexpr int TYPE = prop::THREE_WINDINGS_TRANSFORMER_TRIPPING;

        ThreeWindingsTransformerTripping() = default;
        explicit ThreeWindingsTransformerTripping(ObjectHandle handle, BackendProvider* backend) : Tripping(handle, backend) {}

        std::string getThreeWindingsTransformerToOperateId() const { return getString(prop::TRIPPING_ELEMENT_ID); }
        ThreeWindingsTransformerTripping& setThreeWindingsTransformerToOperateId(const std::string& transformerId);
        ThreeSides getSideToOperate() const { return static_cast<ThreeSides>(getInt(prop::TRIPPING_SIDE)); }
        ThreeWindingsTransformerTripping& setSideToOperate(ThreeSides side);
    };

    static constexpr int TYPE = prop::OVERLOAD_MANAGEMENT_SYSTEM;

    OverloadManagementSystem() = default;
    explicit OverloadManagementSystem(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    bool isEnabled() const { return getBool(prop::AUTOMATION_SYSTEM_ENABLED); }
    OverloadManagementSystem& setEnabled(bool enabled) {
        return setBool(*this, prop::AUTOMATION_SYSTEM_ENABLED, enabled);
    }
    // Include <iidm/Substation.h> to use it
    Substation getSubstation() const;
    std::string getMonitoredElementId() const { return getString(prop::OMS_MONITORED_ELEMENT_ID); }
    ThreeSides getMonitoredSide() const { return static_cast<ThreeSides>(getInt(prop::OMS_MONITORED_SIDE)); }

    // In order
    std::vector<Tripping> getTrippings() const;
    void addTripping(const Tripping& tripping);

    void remove();
};

} // namespace iidm
