#pragma once

#include <iidm/Identifiable.h>
#include <iidm/Measurement.h>
#include <iidm/Terminal.h>
#include <vector>

namespace iidm {

// Equipment connected to the network through terminals (powsybl Connectable): injections,
// lines, transformers...
class Connectable : public Identifiable {
public:
    static constexpr int TYPE = prop::CONNECTABLE;

    Connectable() = default;
    explicit Connectable(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    std::vector<Terminal> getTerminals() const;

    // Connects (disconnects) every terminal, operating breakers if needed: true when this
    // changed it, false when it could not, or already was
    bool connect();
    bool disconnect();

    // An equivalent, from a network reduction for instance
    bool isEquivalent() const;
    Connectable& setEquivalent(bool equivalent);

    // Removes it from the network
    void remove();

    // ── Extensions ───────────────────────────────────────────────────────
    bool hasMeasurements() const;
    Measurements getMeasurements() const;
};

} // namespace iidm
