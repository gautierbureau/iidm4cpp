#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <string>

namespace iidm {

class BackendProvider;

class BusbarSection {
public:
    BusbarSection() = default;
    explicit BusbarSection(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getV() const;
    double getAngle() const;

    Terminal getTerminal() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const BusbarSection& other) const;
    bool operator!=(const BusbarSection& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
