#pragma once

#include <iidm/Enums.h>
#include <iidm/Injection.h>
#include <iidm/LoadAsymmetrical.h>
#include <iidm/LoadAsymmetricalAdder.h>
#include <iidm/LoadDetail.h>
#include <iidm/LoadDetailAdder.h>
#include <iidm/LoadModel.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class Load : public Injection {
public:
    static constexpr int TYPE = prop::LOAD;

    Load() = default;
    explicit Load(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getP0() const { return getDouble(prop::LOAD_P0); }
    Load& setP0(double p0) { return setDouble(*this, prop::LOAD_P0, p0); }

    double getQ0() const { return getDouble(prop::LOAD_Q0); }
    Load& setQ0(double q0) { return setDouble(*this, prop::LOAD_Q0, q0); }

    LoadType getLoadType() const { return static_cast<LoadType>(getInt(prop::LOAD_TYPE)); }
    Load& setLoadType(LoadType loadType) { return setInt(*this, prop::LOAD_TYPE, static_cast<int>(loadType)); }

    // How its power depends on the voltage: empty when it has no model
    iidm::optional<LoadModel> getModel() const;

    // ── Solved values ────────────────────────────────────────────────────
    // P0 and Q0 from the solved flows: both, then each
    void applySolvedValues();
    void setP0ToP();
    void setQ0ToQ();

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasLoadDetail() const;
    LoadDetail getLoadDetail() const;
    LoadDetailAdder newLoadDetail();
    void removeLoadDetail();

    bool hasLoadAsymmetrical() const;
    LoadAsymmetrical getLoadAsymmetrical() const;
    LoadAsymmetricalAdder newLoadAsymmetrical();
    void removeLoadAsymmetrical();
};

} // namespace iidm
