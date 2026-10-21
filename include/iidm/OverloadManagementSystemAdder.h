#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/OverloadManagementSystem.h>
#include <string>

namespace iidm {

class OverloadManagementSystemAdder;

namespace detail {

// What every new tripping sets (powsybl OverloadManagementSystemAdder.TrippingAdder); add() adds it to
// the new system and returns to the system's adder
template <class Self>
class TrippingAdderBase : public AdderBase<Self> {
public:
    Self& setKey(const std::string& key)             { return this->stringField(prop::TRIPPING_KEY, key); }
    Self& setName(const std::string& name)           { return this->stringField(prop::TRIPPING_NAME, name); }
    Self& setCurrentLimit(double currentLimit)       { return this->doubleField(prop::TRIPPING_CURRENT_LIMIT, currentLimit); }
    Self& setOpenAction(bool openAction)             { return this->boolField(prop::TRIPPING_OPEN_ACTION, openAction); }
    Self& setProperty(const std::string& name, const std::string& value) {
        return this->keyedStringField(prop::PROPERTY, name, value);
    }

    OverloadManagementSystemAdder& add() {
        this->create();
        return parent_;
    }

protected:
    TrippingAdderBase(ObjectHandle adder, BackendProvider* backend, OverloadManagementSystemAdder& parent)
        : AdderBase<Self>(adder, backend), parent_(parent) {}

private:
    OverloadManagementSystemAdder& parent_;
};

} // namespace detail

// New overload management system of a substation (Substation::newOverloadManagementSystem())
class OverloadManagementSystemAdder : public detail::IdentifiableAdder<OverloadManagementSystemAdder> {
public:
    class SwitchTrippingAdder;
    class BranchTrippingAdder;
    class ThreeWindingsTransformerTrippingAdder;

    OverloadManagementSystemAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder(adder, backend) {}

    OverloadManagementSystemAdder& setEnabled(bool enabled) { return boolField(prop::AUTOMATION_SYSTEM_ENABLED, enabled); }
    OverloadManagementSystemAdder& setMonitoredElementId(const std::string& id) { return stringField(prop::OMS_MONITORED_ELEMENT_ID, id); }
    OverloadManagementSystemAdder& setMonitoredElementSide(ThreeSides side) {
        return intField(prop::OMS_MONITORED_SIDE, static_cast<int>(side));
    }

    SwitchTrippingAdder newSwitchTripping();
    BranchTrippingAdder newBranchTripping();
    ThreeWindingsTransformerTrippingAdder newThreeWindingsTransformerTripping();

    OverloadManagementSystem add() { return OverloadManagementSystem(create(), backend_); }
};

class OverloadManagementSystemAdder::SwitchTrippingAdder : public detail::TrippingAdderBase<SwitchTrippingAdder> {
public:
    SwitchTrippingAdder(ObjectHandle adder, BackendProvider* backend, OverloadManagementSystemAdder& parent)
        : TrippingAdderBase(adder, backend, parent) {}

    SwitchTrippingAdder& setSwitchToOperateId(const std::string& switchId) { return stringField(prop::TRIPPING_ELEMENT_ID, switchId); }
};

class OverloadManagementSystemAdder::BranchTrippingAdder : public detail::TrippingAdderBase<BranchTrippingAdder> {
public:
    BranchTrippingAdder(ObjectHandle adder, BackendProvider* backend, OverloadManagementSystemAdder& parent)
        : TrippingAdderBase(adder, backend, parent) {}

    BranchTrippingAdder& setBranchToOperateId(const std::string& branchId) { return stringField(prop::TRIPPING_ELEMENT_ID, branchId); }
    BranchTrippingAdder& setSideToOperate(TwoSides side) { return intField(prop::TRIPPING_SIDE, static_cast<int>(side)); }
};

class OverloadManagementSystemAdder::ThreeWindingsTransformerTrippingAdder
    : public detail::TrippingAdderBase<ThreeWindingsTransformerTrippingAdder> {
public:
    ThreeWindingsTransformerTrippingAdder(ObjectHandle adder, BackendProvider* backend, OverloadManagementSystemAdder& parent)
        : TrippingAdderBase(adder, backend, parent) {}

    ThreeWindingsTransformerTrippingAdder& setThreeWindingsTransformerToOperateId(const std::string& transformerId) {
        return stringField(prop::TRIPPING_ELEMENT_ID, transformerId);
    }
    ThreeWindingsTransformerTrippingAdder& setSideToOperate(ThreeSides side) { return intField(prop::TRIPPING_SIDE, static_cast<int>(side)); }
};

inline OverloadManagementSystemAdder::SwitchTrippingAdder OverloadManagementSystemAdder::newSwitchTripping() {
    return SwitchTrippingAdder(backend_->invoke(adder_, prop::SWITCH_TRIPPING), backend_, *this);
}

inline OverloadManagementSystemAdder::BranchTrippingAdder OverloadManagementSystemAdder::newBranchTripping() {
    return BranchTrippingAdder(backend_->invoke(adder_, prop::BRANCH_TRIPPING), backend_, *this);
}

inline OverloadManagementSystemAdder::ThreeWindingsTransformerTrippingAdder
OverloadManagementSystemAdder::newThreeWindingsTransformerTripping() {
    return ThreeWindingsTransformerTrippingAdder(backend_->invoke(adder_, prop::THREE_WINDINGS_TRANSFORMER_TRIPPING), backend_, *this);
}

} // namespace iidm
