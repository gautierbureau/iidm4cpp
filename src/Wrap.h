#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/Enums.h>
#include <iidm/HandleRef.h>
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

// The objects of a list (prop::ELEMENTS) that a lookup returned; the list is released
template <class T>
std::vector<T> elementsOf(ObjectHandle list, BackendProvider* backend) {
    HandleRef owned(list, backend);
    return wrapAll<T>(backend->getChildren(list, prop::ELEMENTS), backend);
}

// The numbers of a list that a lookup returned; the list is released
inline std::vector<int> intElementsOf(ObjectHandle list, BackendProvider* backend) {
    HandleRef owned(list, backend);
    return backend->getIntList(list, prop::ELEMENTS);
}

// connect (disconnect) through the switches the predicate accepts: invoke(h, prop::CONNECT or
// prop::DISCONNECT), on one side (ThreeSides or TwoSides ordinal; -1: every side). Whether this
// changed the object.
inline bool connectThrough(BackendProvider* backend, ObjectHandle h, int code, SwitchPredicate switches, int side) {
    HandleRef call(backend->invoke(h, code), backend);
    backend->setInt(call, prop::SWITCH_PREDICATE, static_cast<int>(switches));
    if (side >= 0) backend->setInt(call, prop::TERMINAL_SIDE, side);
    // The object itself when this changed it (its handle is released right away)
    return HandleRef(backend->invoke(call, prop::ACT_ADD), backend) != INVALID_HANDLE;
}

} } // namespace iidm::detail
