#pragma once

#include <iidm/Enums.h>
#include <iidm/Injection.h>

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
};

} // namespace iidm
