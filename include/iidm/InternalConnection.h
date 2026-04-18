#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

class InternalConnection {
public:
    InternalConnection() = default;
    explicit InternalConnection(ObjectHandle handle, BackendProvider* backend);

    int getNode1() const;
    int getNode2() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
