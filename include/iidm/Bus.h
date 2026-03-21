#pragma once

#include <iidm/ObjectHandle.h>
#include <string>

namespace iidm {

class BackendProvider;

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

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Bus& other) const;
    bool operator!=(const Bus& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
