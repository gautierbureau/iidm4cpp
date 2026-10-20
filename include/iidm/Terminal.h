#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Bus.h>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;
class SlackTerminal;
class VoltageLevel;

class Terminal {
public:
    // ── BusBreakerView ───────────────────────────────────────────────────────
    class BusBreakerView {
    public:
        BusBreakerView() = default;
        explicit BusBreakerView(ObjectHandle termHandle, BackendProvider* backend);

        Bus getBus() const;            // connected bus (invalid if disconnected)
        Bus getConnectableBus() const; // bus-breaker topology bus

        bool isValid() const { return handle_ != INVALID_HANDLE; }

    private:
        HandleRef  handle_;
        BackendProvider* backend_ = detail::nullBackend();
    };

    // ── NodeBreakerView ──────────────────────────────────────────────────────
    class NodeBreakerView {
    public:
        NodeBreakerView() = default;
        explicit NodeBreakerView(ObjectHandle termHandle, BackendProvider* backend);

        int getNode() const;

        bool isValid() const { return handle_ != INVALID_HANDLE; }

    private:
        HandleRef  handle_;
        BackendProvider* backend_ = detail::nullBackend();
    };

    // ── Terminal ─────────────────────────────────────────────────────────────
    Terminal() = default;
    explicit Terminal(ObjectHandle handle, BackendProvider* backend);

    double getP() const;
    Terminal& setP(double p);

    double getQ() const;
    Terminal& setQ(double q);

    bool isConnected() const;
    void connect();
    void disconnect();

    Bus getBusView() const;
    std::string getBusId() const;

    std::string           getConnectableId() const;
    std::vector<Terminal> getTerminals()     const; // all terminals of this terminal's connectable

    VoltageLevel getVoltageLevel() const;

    BusBreakerView  getBusBreakerView()  const;
    NodeBreakerView getNodeBreakerView() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Terminal& other) const;
    bool operator!=(const Terminal& other) const;

private:
    friend class SlackTerminal; // passes the terminal handle to setRelated

    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
