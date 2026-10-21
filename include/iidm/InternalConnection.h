#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

class InternalConnection : public detail::ObjectRef {
public:
    InternalConnection() = default;
    explicit InternalConnection(ObjectHandle handle, BackendProvider* backend);

    int getNode1() const { return getInt(prop::IC_NODE1); }
    int getNode2() const { return getInt(prop::IC_NODE2); }
};

} // namespace iidm
