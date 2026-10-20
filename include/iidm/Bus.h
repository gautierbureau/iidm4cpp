#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <string>

namespace iidm {

class BackendProvider;
class VoltageLevel;

class Bus {
public:
    Bus() = default;
    explicit Bus(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getV() const;
    Bus& setV(double v);

    double getAngle() const;
    Bus& setAngle(double angle);

    VoltageLevel getVoltageLevel() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Bus& other) const;
    bool operator!=(const Bus& other) const;

private:
    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
