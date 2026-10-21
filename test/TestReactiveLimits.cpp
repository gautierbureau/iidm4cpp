#include <gtest/gtest.h>
#include <iidm/Generator.h>
#include <iidm/PropertyCodes.h>
#include <iidm/VscConverterStation.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

namespace {

const ObjectHandle GEN = 40;
const ObjectHandle VSC = 41;
const ObjectHandle CURVE = 50;
const ObjectHandle PT0 = 51, PT1 = 52;
const ObjectHandle SHAPE = 60;
const ObjectHandle PLANE = 61;

class ReactiveLimitsTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.related[{GEN, prop::REACTIVE_LIMITS}]           = CURVE;
        backend.related[{GEN, prop::REACTIVE_CAPABILITY_CURVE}] = CURVE;
        backend.ints   [{CURVE, prop::RL_KIND}] = static_cast<int>(ReactiveLimitsKind::CURVE);
        backend.bools  [{CURVE, prop::REACTIVE_CAPABILITY_CURVE}] = true;
        backend.children[{CURVE, prop::REACTIVE_CURVE_POINT}] = {PT0, PT1};
        backend.ints   [{CURVE, prop::CURVE_POINT_COUNT}] = 2;
        backend.doubles[{CURVE, prop::CURVE_MIN_P}] = 0.0;
        backend.doubles[{CURVE, prop::CURVE_MAX_P}] = 100.0;
        backend.doubles[{PT0, prop::POINT_P}]     = 0.0;
        backend.doubles[{PT0, prop::POINT_MIN_Q}] = -10.0;
        backend.doubles[{PT0, prop::POINT_MAX_Q}] = 10.0;
        backend.doubles[{PT1, prop::POINT_P}]     = 100.0;
        backend.doubles[{PT1, prop::POINT_MIN_Q}] = -30.0;
        backend.doubles[{PT1, prop::POINT_MAX_Q}] = 30.0;
        backend.evaluations[std::make_tuple(CURVE, prop::RL_MIN_Q, std::vector<double>{50.0})] = -20.0;
        backend.evaluations[std::make_tuple(CURVE, prop::RL_MAX_Q, std::vector<double>{150.0, 1.0})] = 40.0;

        backend.related[{VSC, prop::REACTIVE_LIMITS}] = SHAPE;
        backend.ints   [{SHAPE, prop::RL_KIND}] = static_cast<int>(ReactiveLimitsKind::SHAPE);
        backend.bools  [{SHAPE, prop::REACTIVE_CAPABILITY_SHAPE}] = true;
        backend.children[{SHAPE, prop::REACTIVE_CAPABILITY_SHAPE_PLANE}] = {PLANE};
        backend.doubles[{PLANE, prop::PLANE_ALPHA}] = 0.0;
        backend.doubles[{PLANE, prop::PLANE_BETA}]  = 0.5;
        backend.doubles[{PLANE, prop::PLANE_GAMMA}] = 150.0;
        backend.doubles[{PLANE, prop::PLANE_DELTA}] = 1.0;
        backend.bools  [{PLANE, prop::PLANE_LESS_OR_EQUAL}] = true;
        backend.evaluations[std::make_tuple(SHAPE, prop::RL_MAX_Q, std::vector<double>{200.0, 400.0})] = 50.0;
        backend.evaluations[std::make_tuple(SHAPE, prop::SHAPE_IS_INSIDE, std::vector<double>{0.0, 0.0, 400.0})] = 1.0;
        backend.evaluations[std::make_tuple(SHAPE, prop::SHAPE_IS_INSIDE, std::vector<double>{200.0, 90.0, 400.0})] = 0.0;
    }
};

} // namespace

TEST_F(ReactiveLimitsTest, KindAndConversions) {
    Generator gen(GEN, &backend);
    ReactiveLimits limits = gen.getReactiveLimits();
    EXPECT_EQ(limits.getKind(), ReactiveLimitsKind::CURVE);
    EXPECT_TRUE(limits.is<ReactiveCapabilityCurve>());
    EXPECT_FALSE(limits.is<MinMaxReactiveLimits>());
    EXPECT_EQ(limits.as<ReactiveCapabilityCurve>(), gen.getReactiveLimits<ReactiveCapabilityCurve>());
    EXPECT_THROW(limits.as<MinMaxReactiveLimits>(), PropertyNotFoundException);
}

TEST_F(ReactiveLimitsTest, LimitsAtAnActivePower) {
    ReactiveLimits limits = Generator(GEN, &backend).getReactiveLimits();
    EXPECT_DOUBLE_EQ(limits.getMinQ(50.0), -20.0);
    ReactiveCapabilityCurve curve = limits.as<ReactiveCapabilityCurve>();
    EXPECT_DOUBLE_EQ(curve.getMinQ(50.0), -20.0);
    EXPECT_DOUBLE_EQ(curve.getMaxQ(150.0, true), 40.0);
}

TEST_F(ReactiveLimitsTest, Curve) {
    ReactiveCapabilityCurve curve = Generator(GEN, &backend).getReactiveLimits<ReactiveCapabilityCurve>();
    EXPECT_EQ(curve.getPointCount(), 2);
    EXPECT_DOUBLE_EQ(curve.getMinP(), 0.0);
    EXPECT_DOUBLE_EQ(curve.getMaxP(), 100.0);
    std::vector<ReactiveCapabilityCurve::Point> points = curve.getPoints();
    ASSERT_EQ(points.size(), 2u);
    EXPECT_DOUBLE_EQ(points[1].getP(), 100.0);
    EXPECT_DOUBLE_EQ(points[1].getMinQ(), -30.0);
    EXPECT_DOUBLE_EQ(points[1].getMaxQ(), 30.0);
    points[0].setProperty("k", "v");
    EXPECT_EQ(*points[0].getProperty("k"), "v");
}

TEST_F(ReactiveLimitsTest, Shape) {
    VscConverterStation vsc(VSC, &backend);
    ReactiveLimits limits = vsc.getReactiveLimits();
    EXPECT_EQ(limits.getKind(), ReactiveLimitsKind::SHAPE);
    ReactiveCapabilityShape shape = limits.as<ReactiveCapabilityShape>();
    EXPECT_DOUBLE_EQ(shape.getMaxQ(200.0, 400.0), 50.0);
    EXPECT_TRUE(shape.isInside(0.0, 0.0, 400.0));
    EXPECT_FALSE(shape.isInside(200.0, 90.0, 400.0));
    std::vector<ReactiveCapabilityShapePlane> planes = shape.getPlanes();
    ASSERT_EQ(planes.size(), 1u);
    EXPECT_DOUBLE_EQ(planes[0].getBeta(), 0.5);
    EXPECT_DOUBLE_EQ(planes[0].getGamma(), 150.0);
    EXPECT_DOUBLE_EQ(planes[0].getDelta(), 1.0);
    EXPECT_TRUE(planes[0].isLessOrEqual());
    EXPECT_FALSE(planes[0].isGreaterOrEqual());
}

TEST_F(ReactiveLimitsTest, Invalid) {
    EXPECT_THROW(ReactiveLimits().getKind(), InvalidHandleException);
    EXPECT_THROW(MinMaxReactiveLimits().getMinQ(0.0), InvalidHandleException);
    EXPECT_THROW(Generator().getReactiveLimits(), InvalidHandleException);
}

// New limits: started on their holder; a curve's points and a shape's planes as nested adders
TEST_F(ReactiveLimitsTest, Adders) {
    backend.nextAdder = 500;
    Generator gen(GEN, &backend);
    gen.newMinMaxReactiveLimits().setMinQ(-40).setMaxQ(45).add();
    EXPECT_EQ(backend.invocations[0], std::make_pair(GEN, prop::MIN_MAX_REACTIVE_LIMITS));
    EXPECT_DOUBLE_EQ((backend.doubles[{500, prop::RL_MIN_Q}]), -40.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{500, prop::RL_MAX_Q}]), 45.0);
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(500), prop::ACT_ADD));

    gen.newReactiveCapabilityCurve().setProperty("source", "datasheet")
        .beginPoint().setP(0).setMinQ(-10).setMaxQ(10).endPoint()
        .beginPoint().setP(100).setMinQ(-30).setMaxQ(30).setProperty("k", "v").endPoint()
        .add();
    EXPECT_EQ(backend.invocations[2], std::make_pair(GEN, prop::REACTIVE_CAPABILITY_CURVE));
    EXPECT_EQ((backend.keyedStrings[std::make_tuple(ObjectHandle(501), prop::PROPERTY, std::string("source"))]), "datasheet");
    EXPECT_EQ(backend.invocations[3], std::make_pair(ObjectHandle(501), prop::REACTIVE_CURVE_POINT));
    EXPECT_EQ(backend.invocations[4], std::make_pair(ObjectHandle(502), prop::ACT_ADD));
    EXPECT_DOUBLE_EQ((backend.doubles[{503, prop::POINT_MAX_Q}]), 30.0);
    EXPECT_EQ((backend.keyedStrings[std::make_tuple(ObjectHandle(503), prop::PROPERTY, std::string("k"))]), "v");
    EXPECT_EQ(backend.invocations[7], std::make_pair(ObjectHandle(501), prop::ACT_ADD));

    VscConverterStation(VSC, &backend).newReactiveCapabilityShape().addPlane(0, 0.5, 1, false, 150).add();
    EXPECT_EQ(backend.invocations[8], std::make_pair(VSC, prop::REACTIVE_CAPABILITY_SHAPE));
    EXPECT_EQ(backend.invocations[9], std::make_pair(ObjectHandle(504), prop::REACTIVE_CAPABILITY_SHAPE_PLANE));
    EXPECT_DOUBLE_EQ((backend.doubles[{505, prop::PLANE_BETA}]), 0.5);
    EXPECT_DOUBLE_EQ((backend.doubles[{505, prop::PLANE_DELTA}]), 1.0);
    EXPECT_FALSE((backend.bools[{505, prop::PLANE_GREATER_OR_EQUAL}]));
    EXPECT_DOUBLE_EQ((backend.doubles[{505, prop::PLANE_GAMMA}]), 150.0);
    EXPECT_EQ(backend.invocations[10], std::make_pair(ObjectHandle(505), prop::ACT_ADD));
    EXPECT_EQ(backend.invocations[11], std::make_pair(ObjectHandle(504), prop::ACT_ADD));
}

TEST_F(ReactiveLimitsTest, PlaneInequality) {
    ReactiveCapabilityShapePlane plane = ReactiveCapabilityShape(SHAPE, &backend).getPlanes()[0];
    plane.greaterOrEqual(-120).lessOrEqual(130);
    EXPECT_DOUBLE_EQ((backend.doubles[{PLANE, prop::PLANE_GREATER_OR_EQUAL}]), -120.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{PLANE, prop::PLANE_LESS_OR_EQUAL}]), 130.0);
}
