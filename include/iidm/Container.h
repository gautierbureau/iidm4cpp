#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>

namespace iidm {
namespace detail {

// What holds network objects (powsybl Container): networks, substations, voltage levels
template <class Self>
class ContainerMethods {
public:
    ContainerType getContainerType() const;

protected:
    ContainerMethods() = default;
    ~ContainerMethods() = default;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
};

} // namespace detail
} // namespace iidm
