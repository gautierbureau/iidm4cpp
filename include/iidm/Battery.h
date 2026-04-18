#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <string>

namespace iidm {

class BackendProvider;

class Battery {
public:
    Battery() = default;
    explicit Battery(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getTargetP() const;
    Battery& setTargetP(double targetP);

    double getTargetQ() const;
    Battery& setTargetQ(double targetQ);

    double getMinP() const;
    double getMaxP() const;

    double getP0() const;
    Battery& setP0(double p0);

    double getQ0() const;
    Battery& setQ0(double q0);

    Terminal getTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Battery& other) const;
    bool operator!=(const Battery& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
