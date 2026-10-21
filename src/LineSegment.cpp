#include <iidm/LineSegment.h>
#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

#ifndef __cpp_inline_variables
// From C++17 on, the header defines it
const LineSegment LineSegment::FULL_LINE(0, 1);
#endif

namespace detail {

LineSegment readLineSegment(BackendProvider* backend, ObjectHandle segment) {
    HandleRef owned(segment, backend);
    return LineSegment(backend->getDouble(segment, prop::EXT_MC_SEGMENT_START), backend->getDouble(segment, prop::EXT_MC_SEGMENT_END));
}

void setLineSegment(BackendProvider* backend, ObjectHandle target, int which, const LineSegment& segment) {
    HandleRef change(backend->invoke(target, which), backend);
    backend->setDouble(change, prop::EXT_MC_SEGMENT_START, segment.start);
    backend->setDouble(change, prop::EXT_MC_SEGMENT_END, segment.end);
    backend->invoke(change, prop::ACT_ADD);
}

} // namespace detail
} // namespace iidm
