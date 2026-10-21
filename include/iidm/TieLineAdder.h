#pragma once

#include <iidm/Adder.h>
#include <iidm/TieLine.h>

namespace iidm {

// New tie line of a network (Network::newTieLine()): two of its boundary lines, unpaired, with the
// same pairing key
class TieLineAdder : public detail::IdentifiableAdder<TieLineAdder> {
public:
    TieLineAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder(adder, backend) {}

    TieLineAdder& setBoundaryLine1(const std::string& boundaryLineId) { return stringField(prop::BOUNDARY_LINE1_ID, boundaryLineId); }
    TieLineAdder& setBoundaryLine2(const std::string& boundaryLineId) { return stringField(prop::BOUNDARY_LINE2_ID, boundaryLineId); }

    TieLine add() { return TieLine(create(), backend_); }
};

} // namespace iidm
