#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <string>

namespace iidm {

class BackendProvider;

class Line {
public:
    Line() = default;
    explicit Line(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getR() const;
    double getX() const;
    double getG1() const;
    double getB1() const;
    double getG2() const;
    double getB2() const;

    Terminal getTerminal1() const;
    Terminal getTerminal2() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Line& other) const;
    bool operator!=(const Line& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
