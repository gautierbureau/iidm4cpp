#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Extension.h>
#include <iidm/Terminal.h>
#include <optional>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

class TwoWindingsTransformer {
public:
    TwoWindingsTransformer() = default;
    explicit TwoWindingsTransformer(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getR() const;
    double getX() const;
    double getG() const;
    double getB() const;
    double getRatedU1() const;
    double getRatedU2() const;
    std::optional<double> getRatedS() const;

    Terminal getTerminal1() const;
    Terminal getTerminal2() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool hasExtension(const std::string& name) const;
    Extension getExtension(const std::string& name) const;
    std::vector<Extension> getExtensions() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const TwoWindingsTransformer& other) const;
    bool operator!=(const TwoWindingsTransformer& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
