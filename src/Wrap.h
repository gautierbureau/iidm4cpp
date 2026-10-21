#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <vector>

namespace iidm { namespace detail {

// Wrappers for the handles a backend returned: each wrapper takes over its handle.
template <class T>
std::vector<T> wrapAll(const std::vector<ObjectHandle>& handles, BackendProvider* backend) {
    std::vector<T> objects;
    objects.reserve(handles.size());
    for (ObjectHandle h : handles) objects.emplace_back(h, backend);
    return objects;
}

// Empty for INVALID_HANDLE (no such object)
template <class T>
iidm::optional<T> wrapOptional(ObjectHandle h, BackendProvider* backend) {
    if (h == INVALID_HANDLE) return {};
    return T(h, backend);
}

// The number of objects of a type in an object (a container, a view, a bus)
inline int countOf(BackendProvider* backend, ObjectHandle h, int type) {
    return static_cast<int>(backend->evaluate(h, prop::CHILD_COUNT, {static_cast<double>(type)}));
}

} } // namespace iidm::detail
