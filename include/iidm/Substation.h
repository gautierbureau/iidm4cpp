#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/VoltageLevel.h>
#include <optional>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

class Substation {
public:
    Substation() = default;
    explicit Substation(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    std::optional<Country> getCountry() const;

    std::vector<VoltageLevel> getVoltageLevels() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Substation& other) const;
    bool operator!=(const Substation& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
