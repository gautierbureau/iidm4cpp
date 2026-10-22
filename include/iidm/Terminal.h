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

        // The bus it is connected to (empty when disconnected), the one it can connect to
        iidm::optional<Bus> getBus() const;
        iidm::optional<Bus> getConnectableBus() const;
        // Another bus of its voltage level to connect to, connected or not as it was (bus-breaker
        // topology only: ValidationException otherwise)
        BusBreakerView& setConnectableBus(const std::string& busId) {
            return setString(*this, prop::TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, busId);
        }
        // Moves its end of the equipment to a bus of a voltage level with a bus-breaker topology:
        // the equipment gets a new terminal there, and this one is no longer its terminal
        void moveConnectable(const std::string& busId, bool connected);
    };

    // ── BusView ──────────────────────────────────────────────────────────────
    class BusView : public detail::ObjectRef {
    public:
        BusView() = default;
        explicit BusView(ObjectHandle termHandle, BackendProvider* backend) : ObjectRef(termHandle, backend) {}

        // The bus it is connected to (empty when disconnected), the one it can connect to
        iidm::optional<Bus> getBus() const;
        iidm::optional<Bus> getConnectableBus() const;
    };

    // ── NodeBreakerView ──────────────────────────────────────────────────────
    class NodeBreakerView : public detail::ObjectRef {
    public:
        NodeBreakerView() = default;
        explicit NodeBreakerView(ObjectHandle termHandle, BackendProvider* backend) : ObjectRef(termHandle, backend) {}

        int getNode() const { return getInt(prop::TERMINAL_NODE); }
        // Moves its end of the equipment to a node of a voltage level with a node-breaker
        // topology: the equipment gets a new terminal there, and this one is no longer its terminal
        void moveConnectable(int node, const std::string& voltageLevelId);
    };

    // ── Terminal ─────────────────────────────────────────────────────────────
    Terminal() = default;
    explicit Terminal(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    double getP() const { return getDouble(prop::TERMINAL_P); }
    Terminal& setP(double p) { return setDouble(*this, prop::TERMINAL_P, p); }

    double getQ() const { return getDouble(prop::TERMINAL_Q); }
    Terminal& setQ(double q) { return setDouble(*this, prop::TERMINAL_Q, q); }

    // Current (A), from its flows and the voltage of its bus
    double getI() const { return getDouble(prop::TERMINAL_I); }
    // Unsets its flows
    void unsetSolvedValues();

    bool isConnected() const { return getBool(prop::TERMINAL_CONNECTED); }
    // Connects (disconnects) it, operating breakers if needed: true when this changed it, false
    // when it could not, or already was
    bool connect();
    bool disconnect();
    // The same, operating only the switches the predicate accepts (connect() operates the
    // NONFICTIONAL_BREAKER ones, disconnect() the NONFICTIONAL_CLOSED_BREAKER ones)
    bool connect(SwitchPredicate switches);
    bool disconnect(SwitchPredicate switches);

    // The equipment it belongs to (include <iidm/Connectable.h> to use it)
    Connectable getConnectable() const;
    // Its side on a branch or three windings transformer; empty for an injection
    iidm::optional<ThreeSides> getSide() const;

    VoltageLevel getVoltageLevel() const;

    BusBreakerView  getBusBreakerView()  const;
    BusView         getBusView()         const;
    NodeBreakerView getNodeBreakerView() const;
};

} // namespace iidm
