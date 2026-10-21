#pragma once

#include <iidm/Adder.h>
#include <iidm/GeneratorEntsoeCategory.h>

namespace iidm {

// New GeneratorEntsoeCategory extension of a generator (newGeneratorEntsoeCategory())
class GeneratorEntsoeCategoryAdder : public detail::AdderBase<GeneratorEntsoeCategoryAdder> {
public:
    GeneratorEntsoeCategoryAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    GeneratorEntsoeCategoryAdder& withCode(int code) { return intField(prop::EXT_GEC_CODE, code); }

    GeneratorEntsoeCategory add() { return GeneratorEntsoeCategory(create(), backend_); }
};

} // namespace iidm
