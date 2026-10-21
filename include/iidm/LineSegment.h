#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// The part of a line a mutual coupling is on (powsybl LineSegment), from start to end: fractions of
// the line's length. A value; ValidationException where it is used unless 0 <= start <= end <= 1.
struct LineSegment {
    double start;
    double end;

    // The full line
    constexpr LineSegment() : start(0), end(1) {}
    constexpr LineSegment(double start, double end) : start(start), end(end) {}

    static const LineSegment FULL_LINE;

    bool operator==(const LineSegment& other) const { return start == other.start && end == other.end; }
    bool operator!=(const LineSegment& other) const { return !(*this == other); }
};

#ifdef __cpp_inline_variables
// In the header rather than the library: a Windows DLL does not export data
inline const LineSegment LineSegment::FULL_LINE(0, 1);
#endif

namespace detail {

// A segment object of a mutual coupling, read; its handle is released
LineSegment readLineSegment(BackendProvider* backend, ObjectHandle segment);
// Sets a segment of a mutual coupling, or of its adder: which is EXT_MC_LINE1_SEGMENT or EXT_MC_LINE2_SEGMENT
void setLineSegment(BackendProvider* backend, ObjectHandle target, int which, const LineSegment& segment);

} // namespace detail
} // namespace iidm
