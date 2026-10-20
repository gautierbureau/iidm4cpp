#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/VoltageLevel.h>
#include <iidm/VoltageLevelAdder.h>
#include <iidm/stdcxx/optional.hpp>
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

    iidm::optional<Country> getCountry() const;

    // Empty when undefined
    std::string getTso() const;
    Substation& setTso(const std::string& tso);

    std::vector<VoltageLevel> getVoltageLevels() const;
    VoltageLevelAdder newVoltageLevel();

    // Removes the substation, its voltage levels and everything connected to them
    void remove();

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Substation& other) const;
    bool operator!=(const Substation& other) const;

private:
    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
