#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The LoadAsymmetrical extension of a load: P0 and Q0 are then its balanced part, and each
// phase has an unbalanced part on top (phase A: P0 + deltaPa, Q0 + deltaQa). An object of its
// own, from Load::getLoadAsymmetrical().
class LoadAsymmetrical : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::LOAD_ASYMMETRICAL;

    LoadAsymmetrical() = default;
    explicit LoadAsymmetrical(ObjectHandle handle, BackendProvider* backend);

    LoadConnectionType getConnectionType() const {
        return static_cast<LoadConnectionType>(getInt(prop::EXT_LA_CONNECTION_TYPE));
    }
    LoadAsymmetrical& setConnectionType(LoadConnectionType connectionType) {
        return setInt(*this, prop::EXT_LA_CONNECTION_TYPE, static_cast<int>(connectionType));
    }

    // MW
    double getDeltaPa() const { return getDouble(prop::EXT_LA_DELTA_PA); }
    LoadAsymmetrical& setDeltaPa(double deltaPa) { return setDouble(*this, prop::EXT_LA_DELTA_PA, deltaPa); }

    // MVar
    double getDeltaQa() const { return getDouble(prop::EXT_LA_DELTA_QA); }
    LoadAsymmetrical& setDeltaQa(double deltaQa) { return setDouble(*this, prop::EXT_LA_DELTA_QA, deltaQa); }

    double getDeltaPb() const { return getDouble(prop::EXT_LA_DELTA_PB); }
    LoadAsymmetrical& setDeltaPb(double deltaPb) { return setDouble(*this, prop::EXT_LA_DELTA_PB, deltaPb); }

    double getDeltaQb() const { return getDouble(prop::EXT_LA_DELTA_QB); }
    LoadAsymmetrical& setDeltaQb(double deltaQb) { return setDouble(*this, prop::EXT_LA_DELTA_QB, deltaQb); }

    double getDeltaPc() const { return getDouble(prop::EXT_LA_DELTA_PC); }
    LoadAsymmetrical& setDeltaPc(double deltaPc) { return setDouble(*this, prop::EXT_LA_DELTA_PC, deltaPc); }

    double getDeltaQc() const { return getDouble(prop::EXT_LA_DELTA_QC); }
    LoadAsymmetrical& setDeltaQc(double deltaQc) { return setDouble(*this, prop::EXT_LA_DELTA_QC, deltaQc); }
};

} // namespace iidm
