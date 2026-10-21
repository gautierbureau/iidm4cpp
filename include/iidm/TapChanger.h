#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/stdcxx/optional.hpp>
#include <map>

namespace iidm {
namespace detail {

// What both kinds of tap changer have (powsybl TapChanger): tap positions, and a step for each of
// them, from the low to the high tap position; Replacer replaces the steps.
template <class Self, class Step, class Replacer>
class TapChangerMethods {
public:
    int getLowTapPosition() const;
    Self& setLowTapPosition(int lowTapPosition);
    int getHighTapPosition() const;
    int getStepCount() const;

    // Fails when the tap position is unset; findTapPosition is empty then
    int getTapPosition() const;
    iidm::optional<int> findTapPosition() const;
    Self& setTapPosition(int tapPosition);
    Self& unsetTapPosition();

    // The tap position found by a load flow; empty when unset
    iidm::optional<int> getSolvedTapPosition() const;
    iidm::optional<int> findSolvedTapPosition() const;
    Self& setSolvedTapPosition(int solvedTapPosition);
    Self& unsetSolvedTapPosition();

    // The step at a tap position (ValidationException outside [low, high])
    Step getStep(int tapPosition) const;
    // Every step, by tap position
    std::map<int, Step> getAllSteps() const;
    // Replaces every step at once
    Replacer stepsReplacer();
    // The step at the tap position, at the solved tap position: empty when it is unset
    iidm::optional<Step> getCurrentStep() const;
    iidm::optional<Step> getSolvedCurrentStep() const;
    // The step that changes nothing (rho 1, no phase shift), if there is one
    iidm::optional<int> getNeutralPosition() const;
    iidm::optional<Step> getNeutralStep() const;

    bool isRegulating() const;

    bool hasLoadTapChangingCapabilities() const;
    Self& setLoadTapChangingCapabilities(bool loadTapChangingCapabilities);

    // The solved tap position, when set, becomes the tap position
    void applySolvedValues();
    void setTapPositionToSolvedTapPosition();

    // Removes it from its transformer
    void remove();

protected:
    TapChangerMethods() = default;
    ~TapChangerMethods() = default;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
};

} // namespace detail
} // namespace iidm
