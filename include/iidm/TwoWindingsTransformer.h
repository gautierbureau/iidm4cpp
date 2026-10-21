#pragma once

#include <iidm/Branch.h>
#include <iidm/Connectable.h>
#include <iidm/TapChangerHolder.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class Substation;

class TwoWindingsTransformer : public Connectable,
                               public detail::BranchMethods<TwoWindingsTransformer>,
                               public detail::TapChangerHolderMethods<TwoWindingsTransformer> {
public:
    static constexpr int TYPE = prop::TWO_WINDINGS_TRANSFORMER;

    TwoWindingsTransformer() = default;
    explicit TwoWindingsTransformer(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    // Empty when it is not in a substation
    iidm::optional<Substation> getSubstation() const;

    double getR() const { return getDouble(prop::TWO_WT_R); }
    TwoWindingsTransformer& setR(double r) { return setDouble(*this, prop::TWO_WT_R, r); }
    double getX() const { return getDouble(prop::TWO_WT_X); }
    TwoWindingsTransformer& setX(double x) { return setDouble(*this, prop::TWO_WT_X, x); }
    double getG() const { return getDouble(prop::TWO_WT_G); }
    TwoWindingsTransformer& setG(double g) { return setDouble(*this, prop::TWO_WT_G, g); }
    double getB() const { return getDouble(prop::TWO_WT_B); }
    TwoWindingsTransformer& setB(double b) { return setDouble(*this, prop::TWO_WT_B, b); }
    double getRatedU1() const { return getDouble(prop::TWO_WT_RATED_U1); }
    TwoWindingsTransformer& setRatedU1(double ratedU1) { return setDouble(*this, prop::TWO_WT_RATED_U1, ratedU1); }
    double getRatedU2() const { return getDouble(prop::TWO_WT_RATED_U2); }
    TwoWindingsTransformer& setRatedU2(double ratedU2) { return setDouble(*this, prop::TWO_WT_RATED_U2, ratedU2); }
    iidm::optional<double> getRatedS() const { return getOptDouble(prop::TWO_WT_RATED_S); }
    // NaN: undefined
    TwoWindingsTransformer& setRatedS(double ratedS) { return setDouble(*this, prop::TWO_WT_RATED_S, ratedS); }

    // ── Solved values ────────────────────────────────────────────────────
    // The solved tap positions of its tap changers, when set, become their tap positions: of both
    // of them, of its ratio tap changer, of its phase tap changer
    void applySolvedValues();
    void setRatioTapPositionToSolvedTapPosition();
    void setPhaseTapPositionToSolvedTapPosition();
};

} // namespace iidm
