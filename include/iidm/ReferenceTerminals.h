#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Terminal.h>
#include <vector>

namespace iidm {

class BackendProvider;
class Network;

// The ReferenceTerminals extension of a network: the terminals chosen as the angle reference (by a
// load flow, for each synchronous component). An object of its own, from
// Network::getExtension<ReferenceTerminals>().
class ReferenceTerminals : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::REFERENCE_TERMINALS;

    ReferenceTerminals() = default;
    explicit ReferenceTerminals(ObjectHandle handle, BackendProvider* backend);

    // In the order they were added
    std::vector<Terminal> getReferenceTerminals() const;
    // Terminals of the network (ValidationException otherwise), in place of the others
    ReferenceTerminals& setReferenceTerminals(const std::vector<Terminal>& terminals);
    // No terminal
    ReferenceTerminals& reset();
    ReferenceTerminals& addReferenceTerminal(const Terminal& terminal);

    // The powsybl helpers: the reference terminals of a network (none without the extension); their
    // reset, in its subnetworks too (the extension is created when absent); a terminal added to the
    // extension of its root network (created when absent).
    static std::vector<Terminal> getTerminals(const Network& network);
    static void reset(const Network& network);
    static void addTerminal(const Terminal& terminal);
};

} // namespace iidm
