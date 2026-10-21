#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/PhaseTapChangerAdder.h>
#include <iidm/RatioTapChanger.h>
#include <iidm/RatioTapChangerAdder.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {
namespace detail {

// What may have a ratio and a phase tap changer (powsybl RatioTapChangerHolder and
// PhaseTapChangerHolder): two windings transformers, and the legs of three windings transformers.
template <class Self>
class TapChangerHolderMethods {
public:
    bool hasRatioTapChanger() const;
    // Empty when it has none
    iidm::optional<RatioTapChanger> getRatioTapChanger() const;

    bool hasPhaseTapChanger() const;
    // Empty when it has none
    iidm::optional<PhaseTapChanger> getPhaseTapChanger() const;
    // New tap changers, which replace the current ones on add()
    RatioTapChangerAdder newRatioTapChanger();
    PhaseTapChangerAdder newPhaseTapChanger();
    // The same, with the values, regulation and steps of another one (a copy)
    RatioTapChangerAdder newRatioTapChanger(const RatioTapChanger& copied);
    PhaseTapChangerAdder newPhaseTapChanger(const PhaseTapChanger& copied);

protected:
    TapChangerHolderMethods() = default;
    ~TapChangerHolderMethods() = default;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
};

} // namespace detail
} // namespace iidm
