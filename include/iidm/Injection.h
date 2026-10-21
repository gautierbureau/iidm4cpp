#pragma once

#include <iidm/Connectable.h>
#include <iidm/Terminal.h>

namespace iidm {

// Equipment with one terminal (powsybl Injection): generators, loads, batteries, shunts...
class Injection : public Connectable {
public:
    static constexpr int TYPE = prop::INJECTION;

    Injection() = default;
    explicit Injection(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    Terminal getTerminal() const;
};

} // namespace iidm
