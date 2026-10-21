#pragma once

#include <iidm/Adder.h>
#include <iidm/InjectionObservability.h>

namespace iidm {

// New InjectionObservability extension of an injection. A quality is defined when its standard
// deviation is given; its redundancy stays undefined unless given.
class InjectionObservabilityAdder : public detail::AdderBase<InjectionObservabilityAdder> {
public:
    InjectionObservabilityAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    InjectionObservabilityAdder& withObservable(bool observable) { return boolField(prop::EXT_OBS_OBSERVABLE, observable); }
    InjectionObservabilityAdder& withStandardDeviationP(double standardDeviation) { return doubleField(prop::OBS_QUALITY_P, standardDeviation); }
    InjectionObservabilityAdder& withRedundantP(bool redundant)                   { return intField(prop::OBS_QUALITY_P, redundant ? 1 : 0); }
    InjectionObservabilityAdder& withStandardDeviationQ(double standardDeviation) { return doubleField(prop::OBS_QUALITY_Q, standardDeviation); }
    InjectionObservabilityAdder& withRedundantQ(bool redundant)                   { return intField(prop::OBS_QUALITY_Q, redundant ? 1 : 0); }
    InjectionObservabilityAdder& withStandardDeviationV(double standardDeviation) { return doubleField(prop::OBS_QUALITY_V, standardDeviation); }
    InjectionObservabilityAdder& withRedundantV(bool redundant)                   { return intField(prop::OBS_QUALITY_V, redundant ? 1 : 0); }

    InjectionObservability add() { return InjectionObservability(create(), backend_); }
};

} // namespace iidm
