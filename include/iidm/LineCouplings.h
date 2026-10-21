#pragma once

#include <iidm/Adder.h>
#include <iidm/Line.h>
#include <iidm/LineSegment.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <vector>

namespace iidm {

class BackendProvider;

// A mutual coupling of the LineCouplings extension: the coupling between two lines, on a segment of each
class MutualCoupling : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::MUTUAL_COUPLING;

    MutualCoupling() = default;
    explicit MutualCoupling(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    Line getLine1() const;
    Line getLine2() const;
    // Ω
    double getR() const { return getDouble(prop::EXT_MC_R); }
    MutualCoupling& setR(double r) { return setDouble(*this, prop::EXT_MC_R, r); }
    double getX() const { return getDouble(prop::EXT_MC_X); }
    MutualCoupling& setX(double x) { return setDouble(*this, prop::EXT_MC_X, x); }
    LineSegment getLine1Segment() const;
    LineSegment getLine2Segment() const;
    MutualCoupling& setLine1Segment(const LineSegment& segment);
    MutualCoupling& setLine2Segment(const LineSegment& segment);
};

// New mutual coupling of a LineCouplings extension (LineCouplings::newMutualCoupling()): two different
// lines, not coupled yet (ValidationException from add() otherwise); r and x are 0 and the segments the
// full lines unless set.
class MutualCouplingAdder : public detail::AdderBase<MutualCouplingAdder> {
public:
    MutualCouplingAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    MutualCouplingAdder& withLine1(const Line& line) { return relatedField(prop::EXT_MC_LINE1, detail::Access::handle(line)); }
    MutualCouplingAdder& withLine2(const Line& line) { return relatedField(prop::EXT_MC_LINE2, detail::Access::handle(line)); }
    MutualCouplingAdder& withR(double r) { return doubleField(prop::EXT_MC_R, r); }
    MutualCouplingAdder& withX(double x) { return doubleField(prop::EXT_MC_X, x); }
    MutualCouplingAdder& withLine1Segment(const LineSegment& segment) {
        detail::setLineSegment(backend_, adder_, prop::EXT_MC_LINE1_SEGMENT, segment);
        return *this;
    }
    MutualCouplingAdder& withLine2Segment(const LineSegment& segment) {
        detail::setLineSegment(backend_, adder_, prop::EXT_MC_LINE2_SEGMENT, segment);
        return *this;
    }

    MutualCoupling add() { return MutualCoupling(create(), backend_); }
};

// The LineCouplings extension of a network: the mutual couplings between its lines. An object of
// its own, from Network::getExtension<LineCouplings>(). removeExtension<LineCouplings>() removes
// it, then throws PropertyNotFoundException: on removal, powsybl's
// LineCouplingsImpl.setExtendable(null) adds a listener to the null network (a
// NullPointerException), passed on until powsybl fixes it.
class LineCouplings : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::LINE_COUPLINGS;

    LineCouplings() = default;
    explicit LineCouplings(ObjectHandle handle, BackendProvider* backend);

    // In the order they were added
    std::vector<MutualCoupling> getMutualCouplings() const;
    // A coupling made before (one removed, say); ValidationException when two of its lines are coupled already
    LineCouplings& add(const MutualCoupling& mutualCoupling);
    MutualCouplingAdder newMutualCoupling();
    // The coupling of the two lines, in either order; empty when they are not coupled
    iidm::optional<MutualCoupling> findMutualCoupling(const Line& line1, const Line& line2) const;
    // Whether it was removed
    bool removeMutualCoupling(const MutualCoupling& mutualCoupling);
    bool removeMutualCoupling(const Line& line1, const Line& line2);
};

} // namespace iidm
