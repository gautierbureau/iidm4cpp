#pragma once

#include <iidm/Bus.h>
#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {

class Connectable;
class VoltageLevel;

class Terminal : public detail::ObjectRef {
public:
    // ── BusBreakerView ───────────────────────────────────────────────────────
    class BusBreakerView : public detail::ObjectRef {
    public:
        BusBreakerView() = default;
        explicit BusBreakerView(ObjectHandle termHandle, BackendProvider* backend) : ObjectRef(termHandle, backend) {}

        Bus getBus() const;            // connected bus (invalid if disconnected)
        Bus getConnectableBus() const; // bus-breaker topology bus
    };

    // ── NodeBreakerView ──────────────────────────────────────────────────────
    class NodeBreakerView : public detail::ObjectRef {
    public:
        NodeBreakerView() = default;
        explicit NodeBreakerView(ObjectHandle termHandle, BackendProvider* backend) : ObjectRef(termHandle, backend) {}

        int getNode() const;
    };

    // ── Terminal ─────────────────────────────────────────────────────────────
    Terminal() = default;
    explicit Terminal(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    double getP() const;
    Terminal& setP(double p);

    double getQ() const;
    Terminal& setQ(double q);

    bool isConnected() const;
    // Connects (disconnects) it, operating breakers if needed: true when this changed it, false
    // when it could not, or already was
    bool connect();
    bool disconnect();

    Bus getBusView() const;
    std::string getBusId() const;

    // The equipment it belongs to (include <iidm/Connectable.h> to use it)
    Connectable getConnectable() const;
    // Its side on a branch or three windings transformer; empty for an injection
    iidm::optional<ThreeSides> getSide() const;

    VoltageLevel getVoltageLevel() const;

    BusBreakerView  getBusBreakerView()  const;
    NodeBreakerView getNodeBreakerView() const;
};

} // namespace iidm
