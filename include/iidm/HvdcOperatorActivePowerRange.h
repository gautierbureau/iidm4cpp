#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl HvdcOperatorActivePowerRange extension attached to an HvdcLine.
class HvdcOperatorActivePowerRange : public detail::ObjectRef {
public:
    HvdcOperatorActivePowerRange() = default;
    explicit HvdcOperatorActivePowerRange(ObjectHandle handle, BackendProvider* backend);

    double getOprFromCS1toCS2() const;
    HvdcOperatorActivePowerRange& setOprFromCS1toCS2(double value);

    double getOprFromCS2toCS1() const;
    HvdcOperatorActivePowerRange& setOprFromCS2toCS1(double value);
};

} // namespace iidm
