#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include <string>

namespace iidm { namespace detail {

// The has / get / new / remove accessors of an extension E on its host (the extension codes of
// PropertyCodes.def): E::TYPE is the extension's type code.

template <class E>
bool hasExtension(BackendProvider* backend, ObjectHandle host) {
    HandleRef extension(backend->getRelated(host, E::TYPE), backend);
    return extension != INVALID_HANDLE;
}

// ExtensionNotFoundException when absent
template <class E>
E getExtension(BackendProvider* backend, ObjectHandle host, const char* name) {
    E extension(backend->getRelated(host, E::TYPE), backend);
    if (!extension.isValid()) {
        throw ExtensionNotFoundException(std::string(name) + " extension not present on "
                                         + backend->getString(host, prop::ID));
    }
    return extension;
}

template <class E, class Adder>
Adder newExtension(BackendProvider* backend, ObjectHandle host) {
    return Adder(backend->invoke(host, E::TYPE), backend);
}

template <class E>
void removeExtension(BackendProvider* backend, ObjectHandle host) {
    backend->setRelated(host, E::TYPE, INVALID_HANDLE);
}

} } // namespace iidm::detail
