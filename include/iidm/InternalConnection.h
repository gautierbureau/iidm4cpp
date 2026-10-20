#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

class InternalConnection : public detail::ObjectRef {
public:
    InternalConnection() = default;
    explicit InternalConnection(ObjectHandle handle, BackendProvider* backend);

    int getNode1() const;
    int getNode2() const;
};

} // namespace iidm
