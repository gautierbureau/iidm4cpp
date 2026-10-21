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

    LoadConnectionType getConnectionType() const;
    LoadAsymmetrical& setConnectionType(LoadConnectionType connectionType);

    // MW
    double getDeltaPa() const;
    LoadAsymmetrical& setDeltaPa(double deltaPa);

    // MVar
    double getDeltaQa() const;
    LoadAsymmetrical& setDeltaQa(double deltaQa);

    double getDeltaPb() const;
    LoadAsymmetrical& setDeltaPb(double deltaPb);

    double getDeltaQb() const;
    LoadAsymmetrical& setDeltaQb(double deltaQb);

    double getDeltaPc() const;
    LoadAsymmetrical& setDeltaPc(double deltaPc);

    double getDeltaQc() const;
    LoadAsymmetrical& setDeltaQc(double deltaQc);
};

} // namespace iidm
