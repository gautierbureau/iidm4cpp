#include <iidm/InternalConnection.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

InternalConnection::InternalConnection(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

int InternalConnection::getNode1() const {
    return backend_->getInt(handle_, prop::IC_NODE1);
}

int InternalConnection::getNode2() const {
    return backend_->getInt(handle_, prop::IC_NODE2);
}

} // namespace iidm
