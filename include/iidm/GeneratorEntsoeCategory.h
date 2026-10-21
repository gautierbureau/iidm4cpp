#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The GeneratorEntsoeCategory extension of a generator: an object of its own, from
// Generator::getExtension<GeneratorEntsoeCategory>().
class GeneratorEntsoeCategory : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::GENERATOR_ENTSOE_CATEGORY;

    GeneratorEntsoeCategory() = default;
    explicit GeneratorEntsoeCategory(ObjectHandle handle, BackendProvider* backend);

    // The ENTSO-E category code of the generator (from 1)
    int getCode() const { return getInt(prop::EXT_GEC_CODE); }
    GeneratorEntsoeCategory& setCode(int code) { return setInt(*this, prop::EXT_GEC_CODE, code); }
};

} // namespace iidm
