#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Terminal.h>

namespace iidm {

class BackendProvider;
class Branch;
class Injection;
class ThreeWindingsTransformer;

// A priority of the ReferencePriorities extension of a connectable: whether one of its terminals is
// to be chosen as the angle reference: 0 not to be, else 1 is the first choice, then 2...
class ReferencePriority : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::REFERENCE_PRIORITY;

    ReferencePriority() = default;
    explicit ReferencePriority(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    Terminal getTerminal() const;
    int getPriority() const { return getInt(prop::EXT_RP_PRIORITY); }

    // The powsybl helpers: the priority of an injection, of a side of a branch or of a three windings
    // transformer (0 when it has none); set: the extension is created when absent, and the priority
    // replaces that of the same terminal.
    static int get(const Injection& injection);
    static int get(const Branch& branch, TwoSides side);
    static int get(const ThreeWindingsTransformer& transformer, ThreeSides side);
    static void set(const Injection& injection, int priority);
    static void set(const Branch& branch, TwoSides side, int priority);
    static void set(const ThreeWindingsTransformer& transformer, ThreeSides side, int priority);
};

} // namespace iidm
