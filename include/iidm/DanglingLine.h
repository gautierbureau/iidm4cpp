#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <string>

namespace iidm {

class BackendProvider;

class DanglingLine {
public:
    DanglingLine() = default;
    explicit DanglingLine(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getP0() const;
    DanglingLine& setP0(double p0);

    double getQ0() const;
    DanglingLine& setQ0(double q0);

    double getR() const;
    double getX() const;
    double getG() const;
    double getB() const;

    Terminal getTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const DanglingLine& other) const;
    bool operator!=(const DanglingLine& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
