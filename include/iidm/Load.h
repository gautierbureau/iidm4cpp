#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/Extension.h>
#include <iidm/Terminal.h>
#include <string>
#include <vector>

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

    void connect();
    void disconnect();
    bool isConnected() const;

    bool hasExtension(const std::string& name) const;
    Extension getExtension(const std::string& name) const;
    std::vector<Extension> getExtensions() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Load& other) const;
    bool operator!=(const Load& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
