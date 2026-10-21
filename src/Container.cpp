#include <iidm/Container.h>
#include <iidm/BackendProvider.h>
#include <iidm/Network.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Substation.h>
#include <iidm/VoltageLevel.h>

namespace iidm { namespace detail {

template <class Self>
ContainerType ContainerMethods<Self>::getContainerType() const {
    return static_cast<ContainerType>(backend()->getInt(handle(), prop::CONTAINER_TYPE));
}

template class ContainerMethods<Network>;
template class ContainerMethods<Substation>;
template class ContainerMethods<VoltageLevel>;

} } // namespace iidm::detail
