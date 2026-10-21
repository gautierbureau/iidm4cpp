#include <iidm/LineCouplings.h>
#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

namespace {

// findMutualCoupling or removeMutualCoupling(line1, line2): the query's result
ObjectHandle queryByLines(BackendProvider* backend, ObjectHandle lineCouplings, int query, const Line& line1,
                          const Line& line2) {
    HandleRef pending(backend->invoke(lineCouplings, query), backend);
    backend->setRelated(pending, prop::EXT_MC_LINE1, detail::Access::handle(line1));
    backend->setRelated(pending, prop::EXT_MC_LINE2, detail::Access::handle(line2));
    return backend->invoke(pending, prop::ACT_ADD);
}

} // namespace

Line MutualCoupling::getLine1() const {
    return Line(backend_->getRelated(handle_, prop::EXT_MC_LINE1), backend_);
}

Line MutualCoupling::getLine2() const {
    return Line(backend_->getRelated(handle_, prop::EXT_MC_LINE2), backend_);
}

LineSegment MutualCoupling::getLine1Segment() const {
    return detail::readLineSegment(backend_, backend_->getRelated(handle_, prop::EXT_MC_LINE1_SEGMENT));
}

LineSegment MutualCoupling::getLine2Segment() const {
    return detail::readLineSegment(backend_, backend_->getRelated(handle_, prop::EXT_MC_LINE2_SEGMENT));
}

MutualCoupling& MutualCoupling::setLine1Segment(const LineSegment& segment) {
    detail::setLineSegment(backend_, handle_, prop::EXT_MC_LINE1_SEGMENT, segment);
    return *this;
}

MutualCoupling& MutualCoupling::setLine2Segment(const LineSegment& segment) {
    detail::setLineSegment(backend_, handle_, prop::EXT_MC_LINE2_SEGMENT, segment);
    return *this;
}

LineCouplings::LineCouplings(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::vector<MutualCoupling> LineCouplings::getMutualCouplings() const {
    return detail::wrapAll<MutualCoupling>(backend_->getChildren(handle_, prop::MUTUAL_COUPLING), backend_);
}

LineCouplings& LineCouplings::add(const MutualCoupling& mutualCoupling) {
    backend_->setRelated(handle_, prop::ADD_MUTUAL_COUPLING, detail::Access::handle(mutualCoupling));
    return *this;
}

MutualCouplingAdder LineCouplings::newMutualCoupling() {
    return MutualCouplingAdder(backend_->invoke(handle_, prop::MUTUAL_COUPLING), backend_);
}

iidm::optional<MutualCoupling> LineCouplings::findMutualCoupling(const Line& line1, const Line& line2) const {
    return detail::wrapOptional<MutualCoupling>(queryByLines(backend_, handle_, prop::FIND_MUTUAL_COUPLING, line1, line2),
                                                backend_);
}

bool LineCouplings::removeMutualCoupling(const MutualCoupling& mutualCoupling) {
    HandleRef pending(backend_->invoke(handle_, prop::REMOVE_MUTUAL_COUPLING), backend_);
    backend_->setRelated(pending, prop::MUTUAL_COUPLING, detail::Access::handle(mutualCoupling));
    HandleRef removed(backend_->invoke(pending, prop::ACT_ADD), backend_);
    return removed != INVALID_HANDLE;
}

bool LineCouplings::removeMutualCoupling(const Line& line1, const Line& line2) {
    HandleRef removed(queryByLines(backend_, handle_, prop::REMOVE_MUTUAL_COUPLING, line1, line2), backend_);
    return removed != INVALID_HANDLE;
}

} // namespace iidm
