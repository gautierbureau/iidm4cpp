#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Bus.h>
#include <string>
#include <optional>

namespace iidm {

class BackendProvider;

class Terminal {
public:
    Terminal() = default;
    explicit Terminal(ObjectHandle handle, BackendProvider* backend);

    double getP() const;
    Terminal& setP(double p);

    double getQ() const;
    Terminal& setQ(double q);

    bool isConnected() const;
    void connect();
    void disconnect();

    // Returns the bus in the bus view (may be invalid if disconnected)
    Bus getBusView() const;

    std::string getBusId() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Terminal& other) const;
    bool operator!=(const Terminal& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
