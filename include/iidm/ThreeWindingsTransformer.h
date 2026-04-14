#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <optional>
#include <string>

namespace iidm {

class BackendProvider;

class ThreeWindingsTransformer {
public:
    ThreeWindingsTransformer() = default;
    explicit ThreeWindingsTransformer(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    Terminal getTerminal1() const;
    Terminal getTerminal2() const;
    Terminal getTerminal3() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ThreeWindingsTransformer& other) const;
    bool operator!=(const ThreeWindingsTransformer& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
