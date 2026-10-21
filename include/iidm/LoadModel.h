#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

// How the power of a load depends on the voltage, in one of two types: ZipLoadModel or
// ExponentialLoadModel. getType() or is<T>() tell which, as<T>() gives the model as that type.
class LoadModel : public detail::ObjectRef, public detail::PropertiesHolder<LoadModel> {
public:
    static constexpr int TYPE = prop::LOAD_MODEL;

    LoadModel() = default;
    explicit LoadModel(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    LoadModelType getType() const;
};

// Constant power (c0), current (c1) and impedance (c2) parts of P and Q
class ZipLoadModel : public LoadModel {
public:
    static constexpr int TYPE = prop::ZIP_LOAD_MODEL;

    ZipLoadModel() = default;
    explicit ZipLoadModel(ObjectHandle handle, BackendProvider* backend) : LoadModel(handle, backend) {}

    double getC0p() const;
    ZipLoadModel& setC0p(double c0p);
    double getC1p() const;
    ZipLoadModel& setC1p(double c1p);
    double getC2p() const;
    ZipLoadModel& setC2p(double c2p);
    double getC0q() const;
    ZipLoadModel& setC0q(double c0q);
    double getC1q() const;
    ZipLoadModel& setC1q(double c1q);
    double getC2q() const;
    ZipLoadModel& setC2q(double c2q);
};

// P and Q proportional to a power of the voltage
class ExponentialLoadModel : public LoadModel {
public:
    static constexpr int TYPE = prop::EXPONENTIAL_LOAD_MODEL;

    ExponentialLoadModel() = default;
    explicit ExponentialLoadModel(ObjectHandle handle, BackendProvider* backend) : LoadModel(handle, backend) {}

    double getNp() const;
    ExponentialLoadModel& setNp(double np);
    double getNq() const;
    ExponentialLoadModel& setNq(double nq);
};

} // namespace iidm
