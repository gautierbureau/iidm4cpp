#include <iidm/ReferencePriorities.h>
#include <iidm/BackendProvider.h>
#include <iidm/Branch.h>
#include <iidm/HandleRef.h>
#include <iidm/Injection.h>
#include <iidm/Network.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ThreeWindingsTransformer.h>
#include "Wrap.h"

namespace iidm {

namespace {

int priorityOf(const detail::ObjectRef& connectable, const std::vector<double>& side) {
    return static_cast<int>(detail::Access::backend(connectable)->evaluate(detail::Access::handle(connectable),
                                                                           prop::EXT_RP_PRIORITY, side));
}

// ReferencePriority.set: side < 0 for an injection
void setPriority(const detail::ObjectRef& connectable, int side, int priority) {
    BackendProvider* backend = detail::Access::backend(connectable);
    HandleRef change(backend->invoke(detail::Access::handle(connectable), prop::EXT_RP_PRIORITY), backend);
    if (side >= 0) backend->setInt(change, prop::TERMINAL_SIDE, side);
    backend->setInt(change, prop::EXT_RP_PRIORITY, priority);
    backend->invoke(change, prop::ACT_ADD);
}

} // namespace

Terminal ReferencePriority::getTerminal() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL), backend_);
}

int ReferencePriority::getPriority() const {
    return backend_->getInt(handle_, prop::EXT_RP_PRIORITY);
}

int ReferencePriority::get(const Injection& injection) {
    return priorityOf(injection, std::vector<double>());
}

int ReferencePriority::get(const Branch& branch, TwoSides side) {
    return priorityOf(branch, std::vector<double>(1, static_cast<double>(side)));
}

int ReferencePriority::get(const ThreeWindingsTransformer& transformer, ThreeSides side) {
    return priorityOf(transformer, std::vector<double>(1, static_cast<double>(side)));
}

void ReferencePriority::set(const Injection& injection, int priority) {
    setPriority(injection, -1, priority);
}

void ReferencePriority::set(const Branch& branch, TwoSides side, int priority) {
    setPriority(branch, static_cast<int>(side), priority);
}

void ReferencePriority::set(const ThreeWindingsTransformer& transformer, ThreeSides side, int priority) {
    setPriority(transformer, static_cast<int>(side), priority);
}

ReferencePriorities::ReferencePriorities(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::vector<ReferencePriority> ReferencePriorities::getReferencePriorities() const {
    return detail::wrapAll<ReferencePriority>(backend_->getChildren(handle_, prop::REFERENCE_PRIORITY), backend_);
}

ReferencePriorityAdder ReferencePriorities::newReferencePriority() {
    return ReferencePriorityAdder(backend_->invoke(handle_, prop::REFERENCE_PRIORITY), backend_);
}

void ReferencePriorities::deleteReferencePriorities() {
    backend_->invoke(handle_, prop::ACT_DELETE_REFERENCE_PRIORITIES);
}

std::vector<ReferencePriority> ReferencePriorities::get(const Network& network) {
    BackendProvider* backend = detail::Access::backend(network);
    return detail::wrapAll<ReferencePriority>(backend->getChildren(detail::Access::handle(network), prop::REFERENCE_PRIORITY),
                                              backend);
}

void ReferencePriorities::deleteReferencePriorities(const Network& network) {
    detail::Access::backend(network)->invoke(detail::Access::handle(network), prop::ACT_DELETE_REFERENCE_PRIORITIES);
}

} // namespace iidm
