#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The HvdcOperatorActivePowerRange extension of an HVDC line: an object of its own, from
// HvdcLine::getHvdcOperatorActivePowerRange(). Its values are floats in powsybl.
class HvdcOperatorActivePowerRange : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::HVDC_OPERATOR_ACTIVE_POWER_RANGE;

    HvdcOperatorActivePowerRange() = default;
    explicit HvdcOperatorActivePowerRange(ObjectHandle handle, BackendProvider* backend);

    double getOprFromCS1toCS2() const;
    HvdcOperatorActivePowerRange& setOprFromCS1toCS2(double value);

    double getOprFromCS2toCS1() const;
    HvdcOperatorActivePowerRange& setOprFromCS2toCS1(double value);
};

} // namespace iidm
