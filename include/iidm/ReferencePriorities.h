#pragma once

#include <iidm/Adder.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ReferencePriority.h>
#include <vector>

namespace iidm {

class BackendProvider;
class Network;

// New priority of a ReferencePriorities extension (ReferencePriorities::newReferencePriority())
class ReferencePriorityAdder : public detail::AdderBase<ReferencePriorityAdder> {
public:
    ReferencePriorityAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    // A terminal of the connectable (ValidationException from add() otherwise)
    ReferencePriorityAdder& setTerminal(const Terminal& terminal) {
        return relatedField(prop::REL_TERMINAL, detail::Access::handle(terminal));
    }
    // 0 or more
    ReferencePriorityAdder& setPriority(int priority) { return intField(prop::EXT_RP_PRIORITY, priority); }

    // It replaces the priority of the same terminal
    ReferencePriority add() { return ReferencePriority(create(), backend_); }
};

// The ReferencePriorities extension of a connectable: the priorities of its terminals as the angle
// reference, one per terminal. An object of its own, from Connectable::getReferencePriorities().
class ReferencePriorities : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::REFERENCE_PRIORITIES;

    ReferencePriorities() = default;
    explicit ReferencePriorities(ObjectHandle handle, BackendProvider* backend);

    std::vector<ReferencePriority> getReferencePriorities() const;
    ReferencePriorityAdder newReferencePriority();
    void deleteReferencePriorities();

    // The powsybl helpers: the priorities above 0 of all the connectables of the network, 1 first
    // (equal ones: generators, busbar sections, loads, then the others); their deletion.
    static std::vector<ReferencePriority> get(const Network& network);
    static void deleteReferencePriorities(const Network& network);
};

} // namespace iidm
