#pragma once

#include <iidm/BoundaryLine.h>
#include <iidm/Enums.h>
#include <vector>

namespace iidm { namespace detail {

// The boundary lines that a filter keeps (powsybl BoundaryLineFilter): all of them, the paired
// ones, or the others
std::vector<BoundaryLine> filterBoundaryLines(std::vector<BoundaryLine> lines, BoundaryLineFilter filter);

} } // namespace iidm::detail
