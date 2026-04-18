#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Bus.h>
#include <string>

namespace iidm {

class BackendProvider;

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
        ObjectHandle     handle_  = INVALID_HANDLE;
        BackendProvider* backend_ = nullptr;
    };

    // ── NodeBreakerView ──────────────────────────────────────────────────────
    class NodeBreakerView {
    public:
        NodeBreakerView() = default;
        explicit NodeBreakerView(ObjectHandle termHandle, BackendProvider* backend);

        int getNode() const;

        bool isValid() const { return handle_ != INVALID_HANDLE; }

    private:
        ObjectHandle     handle_  = INVALID_HANDLE;
        BackendProvider* backend_ = nullptr;
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

    BusBreakerView  getBusBreakerView()  const;
    NodeBreakerView getNodeBreakerView() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Terminal& other) const;
    bool operator!=(const Terminal& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
