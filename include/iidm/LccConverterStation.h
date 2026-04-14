#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Extension.h>
#include <iidm/Terminal.h>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

class LccConverterStation {
public:
    LccConverterStation() = default;
    explicit LccConverterStation(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    float getLossFactor() const;
    float getPowerFactor() const;

    Terminal getTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool hasExtension(const std::string& name) const;
    Extension getExtension(const std::string& name) const;
    std::vector<Extension> getExtensions() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const LccConverterStation& other) const;
    bool operator!=(const LccConverterStation& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
