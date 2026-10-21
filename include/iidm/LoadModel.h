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

    LoadModelType getType() const { return static_cast<LoadModelType>(getInt(prop::LOAD_MODEL_TYPE)); }
};

// Constant power (c0), current (c1) and impedance (c2) parts of P and Q
class ZipLoadModel : public LoadModel {
public:
    static constexpr int TYPE = prop::ZIP_LOAD_MODEL;

    ZipLoadModel() = default;
    explicit ZipLoadModel(ObjectHandle handle, BackendProvider* backend) : LoadModel(handle, backend) {}

    double getC0p() const { return getDouble(prop::ZIP_C0P); }
    ZipLoadModel& setC0p(double c0p) { return setDouble(*this, prop::ZIP_C0P, c0p); }
    double getC1p() const { return getDouble(prop::ZIP_C1P); }
    ZipLoadModel& setC1p(double c1p) { return setDouble(*this, prop::ZIP_C1P, c1p); }
    double getC2p() const { return getDouble(prop::ZIP_C2P); }
    ZipLoadModel& setC2p(double c2p) { return setDouble(*this, prop::ZIP_C2P, c2p); }
    double getC0q() const { return getDouble(prop::ZIP_C0Q); }
    ZipLoadModel& setC0q(double c0q) { return setDouble(*this, prop::ZIP_C0Q, c0q); }
    double getC1q() const { return getDouble(prop::ZIP_C1Q); }
    ZipLoadModel& setC1q(double c1q) { return setDouble(*this, prop::ZIP_C1Q, c1q); }
    double getC2q() const { return getDouble(prop::ZIP_C2Q); }
    ZipLoadModel& setC2q(double c2q) { return setDouble(*this, prop::ZIP_C2Q, c2q); }
};

// P and Q proportional to a power of the voltage
class ExponentialLoadModel : public LoadModel {
public:
    static constexpr int TYPE = prop::EXPONENTIAL_LOAD_MODEL;

    ExponentialLoadModel() = default;
    explicit ExponentialLoadModel(ObjectHandle handle, BackendProvider* backend) : LoadModel(handle, backend) {}

    double getNp() const { return getDouble(prop::EXP_NP); }
    ExponentialLoadModel& setNp(double np) { return setDouble(*this, prop::EXP_NP, np); }
    double getNq() const { return getDouble(prop::EXP_NQ); }
    ExponentialLoadModel& setNq(double nq) { return setDouble(*this, prop::EXP_NQ, nq); }
};

} // namespace iidm
