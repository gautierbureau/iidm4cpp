#pragma once

#include <iidm/Enums.h>
#include <iidm/Injection.h>
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

    double getP0() const;
    Load& setP0(double p0);

    double getQ0() const;
    Load& setQ0(double q0);

    LoadType getLoadType() const;
    Load& setLoadType(LoadType loadType);

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
};

} // namespace iidm
