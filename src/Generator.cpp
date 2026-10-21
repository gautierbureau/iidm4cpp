#include <iidm/Generator.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

// ── Solved values ────────────────────────────────────────────────────────────

void Generator::applySolvedValues() {
    backend_->invoke(handle_, prop::ACT_APPLY_SOLVED_VALUES);
}

void Generator::setTargetPToP() {
    backend_->invoke(handle_, prop::ACT_SET_TARGET_P_TO_P);
}

} // namespace iidm
