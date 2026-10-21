#pragma once

#include <iidm/ConnectablePosition.h>
#include <iidm/ConnectablePositionAdder.h>
#include <iidm/Identifiable.h>
#include <iidm/Measurement.h>
#include <iidm/MeasurementsAdder.h>
#include <iidm/ReferencePriorities.h>
#include <iidm/ReferencePrioritiesAdder.h>
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

    // Unsets its solved values: the flows at its terminals, and those of its own (a shunt
    // compensator's solved section count, a transformer's solved tap positions)
    void unsetSolvedValues();

    // An equivalent, from a network reduction for instance
    bool isEquivalent() const { return getBool(prop::EQUIVALENT); }
    Connectable& setEquivalent(bool equivalent) { return setBool(*this, prop::EQUIVALENT, equivalent); }

    // Removes it from the network
    void remove();

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasMeasurements() const;
    Measurements getMeasurements() const;
    MeasurementsAdder newMeasurements();
    void removeMeasurements();

    bool hasConnectablePosition() const;
    ConnectablePosition getConnectablePosition() const;
    ConnectablePositionAdder newConnectablePosition();
    void removeConnectablePosition();

    bool hasReferencePriorities() const;
    ReferencePriorities getReferencePriorities() const;
    ReferencePrioritiesAdder newReferencePriorities();
    void removeReferencePriorities();
};

} // namespace iidm
