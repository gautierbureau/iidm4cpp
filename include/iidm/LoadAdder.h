#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/Load.h>

namespace iidm {

class ZipLoadModelAdder;
class ExponentialLoadModelAdder;

// New load of a voltage level (VoltageLevel::newLoad())
class LoadAdder : public detail::InjectionAdder<LoadAdder> {
public:
    LoadAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    LoadAdder& setLoadType(LoadType loadType) { return intField(prop::LOAD_TYPE, static_cast<int>(loadType)); }
    LoadAdder& setP0(double p0)               { return doubleField(prop::LOAD_P0, p0); }
    LoadAdder& setQ0(double q0)               { return doubleField(prop::LOAD_Q0, q0); }

    // Its model; add() returns to this adder:
    //   vl.newLoad().setId("L")...newZipModel().setC0p(0.5)...add().add()
    ZipLoadModelAdder newZipModel();
    ExponentialLoadModelAdder newExponentialModel();

    Load add() { return Load(create(), backend_); }
};

// New ZIP model of a new load
class ZipLoadModelAdder : public detail::AdderBase<ZipLoadModelAdder> {
public:
    ZipLoadModelAdder(ObjectHandle adder, BackendProvider* backend, LoadAdder& load)
        : AdderBase(adder, backend), load_(load) {}

    ZipLoadModelAdder& setC0p(double c0p) { return doubleField(prop::ZIP_C0P, c0p); }
    ZipLoadModelAdder& setC1p(double c1p) { return doubleField(prop::ZIP_C1P, c1p); }
    ZipLoadModelAdder& setC2p(double c2p) { return doubleField(prop::ZIP_C2P, c2p); }
    ZipLoadModelAdder& setC0q(double c0q) { return doubleField(prop::ZIP_C0Q, c0q); }
    ZipLoadModelAdder& setC1q(double c1q) { return doubleField(prop::ZIP_C1Q, c1q); }
    ZipLoadModelAdder& setC2q(double c2q) { return doubleField(prop::ZIP_C2Q, c2q); }
    ZipLoadModelAdder& setProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }

    LoadAdder& add() {
        create();
        return load_;
    }

private:
    LoadAdder& load_;
};

// New exponential model of a new load
class ExponentialLoadModelAdder : public detail::AdderBase<ExponentialLoadModelAdder> {
public:
    ExponentialLoadModelAdder(ObjectHandle adder, BackendProvider* backend, LoadAdder& load)
        : AdderBase(adder, backend), load_(load) {}

    ExponentialLoadModelAdder& setNp(double np) { return doubleField(prop::EXP_NP, np); }
    ExponentialLoadModelAdder& setNq(double nq) { return doubleField(prop::EXP_NQ, nq); }
    ExponentialLoadModelAdder& setProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }

    LoadAdder& add() {
        create();
        return load_;
    }

private:
    LoadAdder& load_;
};

inline ZipLoadModelAdder LoadAdder::newZipModel() {
    return ZipLoadModelAdder(backend_->invoke(adder_, prop::ZIP_LOAD_MODEL), backend_, *this);
}

inline ExponentialLoadModelAdder LoadAdder::newExponentialModel() {
    return ExponentialLoadModelAdder(backend_->invoke(adder_, prop::EXPONENTIAL_LOAD_MODEL), backend_, *this);
}

} // namespace iidm
