#pragma once

#include <iidm/Identifiable.h>

namespace iidm {

class VoltageLevel;

class Bus : public Identifiable {
public:
    static constexpr int TYPE = prop::BUS;

    Bus() = default;
    explicit Bus(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    double getV() const;
    Bus& setV(double v);

    double getAngle() const;
    Bus& setAngle(double angle);

    VoltageLevel getVoltageLevel() const;
};

} // namespace iidm
