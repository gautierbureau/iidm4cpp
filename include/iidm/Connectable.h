#pragma once

#include <iidm/Identifiable.h>
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
    // The same, operating only the switches the predicate accepts (see Terminal), on every side or
    // on one (of a branch or a three windings transformer)
    bool connect(SwitchPredicate switches);
    bool connect(SwitchPredicate switches, ThreeSides side);
    bool disconnect(SwitchPredicate switches);
    bool disconnect(SwitchPredicate switches, ThreeSides side);

    // Unsets its solved values: the flows at its terminals, and those of its own (a shunt
    // compensator's solved section count, a transformer's solved tap positions)
    void unsetSolvedValues();

    // An equivalent, from a network reduction for instance
    bool isEquivalent() const { return getBool(prop::EQUIVALENT); }
    Connectable& setEquivalent(bool equivalent) { return setBool(*this, prop::EQUIVALENT, equivalent); }

    // Removes it from the network
    void remove();
};

} // namespace iidm
