#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/Terminal.h>
#include <string>

namespace iidm {

class BackendProvider;

class Load {
public:
    Load() = default;
    explicit Load(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getP0() const;
    Load& setP0(double p0);

    double getQ0() const;
    Load& setQ0(double q0);

    LoadType getLoadType() const;

    Terminal getTerminal() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Load& other) const;
    bool operator!=(const Load& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
