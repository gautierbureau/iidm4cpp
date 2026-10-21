#pragma once

#include <iidm/Adder.h>
#include <iidm/BranchObservability.h>

namespace iidm {

// New BranchObservability extension of a branch. A quality is defined when its standard deviation
// is given; its redundancy stays undefined unless given.
class BranchObservabilityAdder : public detail::AdderBase<BranchObservabilityAdder> {
public:
    BranchObservabilityAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    BranchObservabilityAdder& withObservable(bool observable) { return boolField(prop::EXT_OBS_OBSERVABLE, observable); }
    BranchObservabilityAdder& withStandardDeviationP1(double standardDeviation) { return doubleField(prop::OBS_QUALITY_P1, standardDeviation); }
    BranchObservabilityAdder& withRedundantP1(bool redundant)                   { return intField(prop::OBS_QUALITY_P1, redundant ? 1 : 0); }
    BranchObservabilityAdder& withStandardDeviationP2(double standardDeviation) { return doubleField(prop::OBS_QUALITY_P2, standardDeviation); }
    BranchObservabilityAdder& withRedundantP2(bool redundant)                   { return intField(prop::OBS_QUALITY_P2, redundant ? 1 : 0); }
    BranchObservabilityAdder& withStandardDeviationQ1(double standardDeviation) { return doubleField(prop::OBS_QUALITY_Q1, standardDeviation); }
    BranchObservabilityAdder& withRedundantQ1(bool redundant)                   { return intField(prop::OBS_QUALITY_Q1, redundant ? 1 : 0); }
    BranchObservabilityAdder& withStandardDeviationQ2(double standardDeviation) { return doubleField(prop::OBS_QUALITY_Q2, standardDeviation); }
    BranchObservabilityAdder& withRedundantQ2(bool redundant)                   { return intField(prop::OBS_QUALITY_Q2, redundant ? 1 : 0); }

    BranchObservability add() { return BranchObservability(create(), backend_); }
};

} // namespace iidm
