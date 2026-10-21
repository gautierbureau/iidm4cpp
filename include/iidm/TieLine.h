#pragma once

#include <iidm/BoundaryLine.h>
#include <iidm/Branch.h>
#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {

// Two boundary lines of the network paired by their pairing key, making a branch (powsybl TieLine).
// Its characteristics are those of its boundary lines; its terminals and limits too.
class TieLine : public Identifiable, public detail::BranchMethods<TieLine> {
public:
    static constexpr int TYPE = prop::TIE_LINE;

    TieLine() = default;
    explicit TieLine(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    // Empty when its boundary lines have none
    iidm::optional<std::string> getPairingKey() const;

    // From those of its boundary lines
    double getR() const;
    double getX() const;
    double getG1() const;
    double getB1() const;
    double getG2() const;
    double getB2() const;

    BoundaryLine getBoundaryLine1() const;
    BoundaryLine getBoundaryLine2() const;
    BoundaryLine getBoundaryLine(TwoSides side) const;
    // The one in this voltage level (empty when neither is)
    iidm::optional<BoundaryLine> getBoundaryLine(const std::string& voltageLevelId) const;

    // Connects (disconnects) its boundary lines: true when this changed them, false when it could
    // not, or they already were
    bool connectBoundaryLines();
    bool disconnectBoundaryLines();

    // Removes it: its boundary lines stay, unpaired. With updateBoundaryLines, the flows at the
    // boundary become their P0 and Q0 (and their generations stop)
    void remove();
    void remove(bool updateBoundaryLines);
};

} // namespace iidm
