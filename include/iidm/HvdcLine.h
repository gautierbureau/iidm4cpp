#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/Extension.h>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

class HvdcLine {
public:
    HvdcLine() = default;
    explicit HvdcLine(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getR() const;
    double getNominalV() const;
    double getActivePowerSetpoint() const;
    HvdcLine& setActivePowerSetpoint(double value);
    double getMaxP() const;

    HvdcConverterStationMode getConvertersMode() const;
    HvdcLine& setConvertersMode(HvdcConverterStationMode mode);

    bool hasExtension(const std::string& name) const;
    Extension getExtension(const std::string& name) const;
    std::vector<Extension> getExtensions() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const HvdcLine& other) const;
    bool operator!=(const HvdcLine& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
