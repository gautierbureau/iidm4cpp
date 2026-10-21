#pragma once

#include <iidm/Adder.h>
#include <iidm/ReferencePriorities.h>

namespace iidm {

// New ReferencePriorities extension of a connectable (newReferencePriorities()): empty, its
// priorities come with newReferencePriority()
class ReferencePrioritiesAdder : public detail::AdderBase<ReferencePrioritiesAdder> {
public:
    ReferencePrioritiesAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    ReferencePriorities add() { return ReferencePriorities(create(), backend_); }
};

} // namespace iidm
