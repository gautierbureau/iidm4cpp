#pragma once

#include <iidm/CurrentLimits.h>
#include <iidm/Injection.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class DanglingLine : public Injection {
public:
    static constexpr int TYPE = prop::DANGLING_LINE;

    DanglingLine() = default;
    explicit DanglingLine(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getP0() const;
    DanglingLine& setP0(double p0);

    double getQ0() const;
    DanglingLine& setQ0(double q0);

    double getR() const;
    double getX() const;
    double getG() const;
    double getB() const;

    iidm::optional<CurrentLimits> getCurrentLimits() const;
};

} // namespace iidm
