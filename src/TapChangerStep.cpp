#include <iidm/TapChangerStep.h>
#include <iidm/BackendProvider.h>
#include <iidm/PhaseTapChangerStep.h>
#include <iidm/PropertyCodes.h>
#include <iidm/RatioTapChangerStep.h>

namespace iidm { namespace detail {

template <class Self>
double TapChangerStepMethods<Self>::getRho() const {
    return backend()->getDouble(handle(), prop::STEP_RHO);
}

template <class Self>
Self& TapChangerStepMethods<Self>::setRho(double rho) {
    backend()->setDouble(handle(), prop::STEP_RHO, rho);
    return static_cast<Self&>(*this);
}

template <class Self>
double TapChangerStepMethods<Self>::getR() const {
    return backend()->getDouble(handle(), prop::STEP_R);
}

template <class Self>
Self& TapChangerStepMethods<Self>::setR(double r) {
    backend()->setDouble(handle(), prop::STEP_R, r);
    return static_cast<Self&>(*this);
}

template <class Self>
double TapChangerStepMethods<Self>::getX() const {
    return backend()->getDouble(handle(), prop::STEP_X);
}

template <class Self>
Self& TapChangerStepMethods<Self>::setX(double x) {
    backend()->setDouble(handle(), prop::STEP_X, x);
    return static_cast<Self&>(*this);
}

template <class Self>
double TapChangerStepMethods<Self>::getG() const {
    return backend()->getDouble(handle(), prop::STEP_G);
}

template <class Self>
Self& TapChangerStepMethods<Self>::setG(double g) {
    backend()->setDouble(handle(), prop::STEP_G, g);
    return static_cast<Self&>(*this);
}

template <class Self>
double TapChangerStepMethods<Self>::getB() const {
    return backend()->getDouble(handle(), prop::STEP_B);
}

template <class Self>
Self& TapChangerStepMethods<Self>::setB(double b) {
    backend()->setDouble(handle(), prop::STEP_B, b);
    return static_cast<Self&>(*this);
}

template class TapChangerStepMethods<RatioTapChangerStep>;
template class TapChangerStepMethods<PhaseTapChangerStep>;

} } // namespace iidm::detail
