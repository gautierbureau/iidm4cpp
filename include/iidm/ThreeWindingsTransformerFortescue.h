#pragma once

#include <iidm/LegFortescue.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The ThreeWindingsTransformerFortescue extension of a three windings transformer: the
// zero-sequence data of its legs, for asymmetrical studies. An object of its own, from
// ThreeWindingsTransformer::getExtension<ThreeWindingsTransformerFortescue>().
class ThreeWindingsTransformerFortescue : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::THREE_WINDINGS_TRANSFORMER_FORTESCUE;

    ThreeWindingsTransformerFortescue() = default;
    explicit ThreeWindingsTransformerFortescue(ObjectHandle handle, BackendProvider* backend);

    LegFortescue getLeg1() const;
    LegFortescue getLeg2() const;
    LegFortescue getLeg3() const;
};

} // namespace iidm
