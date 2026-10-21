#pragma once

#include <iidm/Adder.h>
#include <iidm/TwoWindingsTransformer.h>

namespace iidm {

// New two windings transformer of a substation (Substation::newTwoWindingsTransformer()), between two of
// its voltage levels; its tap changers are added to it afterwards
class TwoWindingsTransformerAdder : public detail::BranchAdder<TwoWindingsTransformerAdder> {
public:
    TwoWindingsTransformerAdder(ObjectHandle adder, BackendProvider* backend) : BranchAdder(adder, backend) {}

    TwoWindingsTransformerAdder& setR(double r)             { return doubleField(prop::TWO_WT_R, r); }
    TwoWindingsTransformerAdder& setX(double x)             { return doubleField(prop::TWO_WT_X, x); }
    TwoWindingsTransformerAdder& setG(double g)             { return doubleField(prop::TWO_WT_G, g); }
    TwoWindingsTransformerAdder& setB(double b)             { return doubleField(prop::TWO_WT_B, b); }
    TwoWindingsTransformerAdder& setRatedU1(double ratedU1) { return doubleField(prop::TWO_WT_RATED_U1, ratedU1); }
    TwoWindingsTransformerAdder& setRatedU2(double ratedU2) { return doubleField(prop::TWO_WT_RATED_U2, ratedU2); }
    TwoWindingsTransformerAdder& setRatedS(double ratedS)   { return doubleField(prop::TWO_WT_RATED_S, ratedS); }

    // Sets the values and voltage levels of a transformer on an adder (powsybl
    // TwoWindingsTransformerAdder.fillTwoWindingsTransformerAdder)
    static TwoWindingsTransformerAdder& fillTwoWindingsTransformerAdder(TwoWindingsTransformerAdder& adder,
                                                                         const TwoWindingsTransformer& transformer) {
        return adder.relatedField(prop::FILL_FROM, detail::Access::handle(transformer));
    }

    TwoWindingsTransformer add() { return TwoWindingsTransformer(create(), backend_); }
};

} // namespace iidm
