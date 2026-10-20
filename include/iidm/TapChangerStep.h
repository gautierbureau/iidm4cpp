#pragma once

#include <iidm/ObjectRef.h>

namespace iidm {
namespace detail {

// What the steps of both kinds of tap changer have (powsybl TapChangerStep): the ratio, and the
// deviations (%) of the transformer's resistance, reactance, conductance and susceptance at that
// tap position.
template <class Self>
class TapChangerStepMethods {
public:
    double getRho() const;
    Self& setRho(double rho);

    double getR() const;
    Self& setR(double r);

    double getX() const;
    Self& setX(double x);

    double getG() const;
    Self& setG(double g);

    double getB() const;
    Self& setB(double b);

protected:
    TapChangerStepMethods() = default;
    ~TapChangerStepMethods() = default;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
};

} // namespace detail
} // namespace iidm
