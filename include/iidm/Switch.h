#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <string>
#include <iidm/DiscreteMeasurement.h>

namespace iidm {

class BackendProvider;

class Switch {
public:
    Switch() = default;
    explicit Switch(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    bool isOpen() const;
    Switch& setOpen(bool open);

    bool isRetained() const;
    Switch& setRetained(bool retained);

    SwitchKind getKind() const;

    int getNode1() const;
    int getNode2() const;

    // ── State estimation extensions ──────────────────────────────────────
    bool hasDiscreteMeasurements() const;
    DiscreteMeasurements getDiscreteMeasurements() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Switch& other) const;
    bool operator!=(const Switch& other) const;

private:
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
