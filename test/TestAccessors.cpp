#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "CompleteNetwork.h"
#include "RealBackends.h"
#include <cmath>
#include <string>
#include <type_traits>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Every accessor that is one property code (the one-line getters and setters of the headers), on the
// real backends and the network built from C++ (CompleteNetwork.h): a getter reads the value of the
// network, a setter changes it and its getter reads the change back. An accessor with the code of
// another property reads another value, fails as a code of another type of object, or writes what
// its getter does not read. The values are those buildNetwork gives, or powsybl's (NaN where no load
// flow ran); float values are those powsybl stores as floats (1.1f).
//
// A new accessor of that kind gets its line here, after the others of its class (and its object in
// buildNetwork when the network has none).

using namespace iidm;

namespace {

// A double: NaN is the value of a NaN, the others compare as with EXPECT_DOUBLE_EQ
template <class A, class E>
void expectValue(const A& actual, const E& expected, const char* accessor, std::true_type) {
    const double value = static_cast<double>(expected);
    if (std::isnan(value)) {
        EXPECT_TRUE(std::isnan(actual)) << accessor << " = " << actual;
    } else {
        EXPECT_DOUBLE_EQ(actual, value) << accessor;
    }
}

template <class A, class E>
void expectValue(const A& actual, const E& expected, const char* accessor, std::false_type) {
    EXPECT_EQ(actual, expected) << accessor;
}

template <class A, class E>
void expectValue(const A& actual, const E& expected, const char* accessor) {
    expectValue(actual, expected, accessor, std::is_floating_point<A>());
}

} // namespace

// A getter: the value in the network
#define READ(object, get, value) \
    do { \
        try { \
            expectValue((object).get(), value, #object "." #get "()"); \
        } catch (const std::exception& e) { \
            ADD_FAILURE() << #object "." #get "(): " << e.what(); \
        } \
    } while (false)
// A getter and its setter: the value in the network, then the value set, read back; the setter returns
// the object, for chaining
#define WRITE(object, get, set, value, changed) WRITE_READ(object, get, set, value, changed, changed)
#define WRITE_READ(object, get, set, value, changed, readBack) \
    do { \
        READ(object, get, value); \
        try { \
            EXPECT_EQ(&(object).set(changed), &(object)) << #object "." #set "() returns another object"; \
            expectValue((object).get(), readBack, #object "." #set "()"); \
        } catch (const std::exception& e) { \
            ADD_FAILURE() << #object "." #set "(): " << e.what(); \
        } \
    } while (false)

namespace {

void checkAccessors(BackendProvider& b) {
    test::buildNetwork(b);
    Network n = Identifiable(b.getNetworkHandle(), &b).getNetwork();

    WRITE(n, getCaseDate, setCaseDate, "2026-01-31T12:00Z", "2026-02-01T12:00Z");
    WRITE(n, getForecastDistance, setForecastDistance, 60, 61);
    READ(n, getSourceFormat, "iidm4cpp");
    READ(n, getValidationLevel, ValidationLevel::STEADY_STATE_HYPOTHESIS);
    READ(n, isDetachable, false);
    n.setMinimumAcceptableValidationLevel(ValidationLevel::EQUIPMENT);

    Substation s = *n.getSubstation("S");
    WRITE_READ(s, getTso, setTso, iidm::optional<std::string>("RTE"), "RTE2", iidm::optional<std::string>("RTE2"));
    s.addGeographicalTag("south");
    EXPECT_EQ(s.getGeographicalTags(), (std::vector<std::string>{"north", "south"}));

    VoltageLevel vl1 = *n.getVoltageLevel("VL1");
    WRITE(vl1, getNominalV, setNominalV, 400.0, 401.0);
    WRITE_READ(vl1, getLowVoltageLimit, setLowVoltageLimit, iidm::optional<double>(380.0), 370, iidm::optional<double>(370.0));
    WRITE_READ(vl1, getHighVoltageLimit, setHighVoltageLimit, iidm::optional<double>(420.0), 430, iidm::optional<double>(430.0));
    READ(vl1, getTopologyKind, TopologyKind::BUS_BREAKER);

    VoltageLevel::NodeBreakerView nbv = n.getVoltageLevel("VL3")->getNodeBreakerView();
    READ(nbv, getMaximumNodeIndex, 6);

    auto internalConnection = nbv.getInternalConnections().at(0);
    READ(internalConnection, getNode1, 3);
    READ(internalConnection, getNode2, 4);

    Bus bus = *n.getBusBreakerView().getBus("B1");
    WRITE(bus, getV, setV, NAN, 1.5);
    WRITE(bus, getAngle, setAngle, NAN, 1.5);
    READ(bus, getP, 0.0);
    READ(bus, getQ, 0.0);
    WRITE(bus, getFictitiousP0, setFictitiousP0, 0.0, 1.0);
    WRITE(bus, getFictitiousQ0, setFictitiousQ0, 0.0, 1.0);
    READ(bus, isInMainConnectedComponent, true);
    READ(bus, isInMainSynchronousComponent, true);

    Component component = *n.getBusView().getBuses().at(0).getConnectedComponent();
    READ(component, getNum, 0);
    READ(component, getSize, 4);

    BusbarSection bbs = *n.getBusbarSection("BBS3");
    READ(bbs, getV, NAN);
    READ(bbs, getAngle, NAN);

    BusbarSectionPosition bbsPosition = bbs.getExtension<BusbarSectionPosition>();
    WRITE(bbsPosition, getBusbarIndex, setBusbarIndex, 1, 2);
    WRITE(bbsPosition, getSectionIndex, setSectionIndex, 1, 2);

    OperatingStatus operatingStatus = bbs.getExtension<OperatingStatus>();
    WRITE(operatingStatus, getStatus, setStatus, OperatingStatus::Status::IN_OPERATION, OperatingStatus::Status::PLANNED_OUTAGE);

    Switch breaker = *n.getSwitch("BR3");
    WRITE(breaker, isOpen, setOpen, false, true);
    WRITE(breaker, isRetained, setRetained, false, true);
    READ(breaker, getKind, SwitchKind::BREAKER);
    READ(breaker, getNode1, 0);
    READ(breaker, getNode2, 1);

    Generator g1 = *n.getGenerator("G1");
    WRITE(g1, getTargetP, setTargetP, 100.0, 101.0);
    WRITE(g1, getMinP, setMinP, 0.0, 1.0);
    WRITE(g1, getMaxP, setMaxP, 200.0, 201.0);
    WRITE(g1, getRatedS, setRatedS, 250.0, 251.0);
    READ(g1, isCondenser, false);
    WRITE(g1, getEnergySource, setEnergySource, EnergySource::NUCLEAR, EnergySource::HYDRO);

    VoltageRegulation regulation = *g1.getVoltageRegulation();
    WRITE(regulation, getTargetValue, setTargetValue, NAN, 1.5);
    WRITE(regulation, getTargetDeadband, setTargetDeadband, 1.0, 2.0);
    WRITE(regulation, getSlope, setSlope, NAN, 1.5);
    WRITE(regulation, isRegulating, setRegulating, true, false);
    READ(regulation, isWithTerminal, false);
    regulation.setMode(RegulationMode::VOLTAGE);
    EXPECT_EQ(regulation.getMode().value_or(RegulationMode::REACTIVE_POWER), RegulationMode::VOLTAGE);

    Terminal terminal = g1.getTerminal();
    WRITE(terminal, getP, setP, NAN, 1.5);
    WRITE(terminal, getQ, setQ, NAN, 1.5);
    READ(terminal, getI, 816.496580927726);
    READ(terminal, isConnected, true);

    Terminal::NodeBreakerView nodeView = n.getLoad("L3")->getTerminal().getNodeBreakerView();
    READ(nodeView, getNode, 1);

    Terminal::BusBreakerView busView = n.getLoad("L2")->getTerminal().getBusBreakerView();
    busView.setConnectableBus("B2");
    EXPECT_EQ(busView.getConnectableBus()->getId(), "B2");

    ReactiveLimits limits = g1.getReactiveLimits();
    READ(limits, getKind, ReactiveLimitsKind::CURVE);

    ReactiveCapabilityCurve curve = g1.getReactiveLimits<ReactiveCapabilityCurve>();
    READ(curve, getPointCount, 2);
    READ(curve, getMinP, 0.0);
    READ(curve, getMaxP, 200.0);

    ReactiveCapabilityCurve::Point point = curve.getPoints().at(1);
    READ(point, getP, 200.0);
    READ(point, getMinQ, -80.0);
    READ(point, getMaxQ, 80.0);

    ActivePowerControl apc = g1.getExtension<ActivePowerControl>();
    WRITE(apc, isParticipate, setParticipate, true, false);
    WRITE(apc, getDroop, setDroop, 4.0, 5.0);
    WRITE(apc, getParticipationFactor, setParticipationFactor, 0.5, 1.5);
    WRITE_READ(apc, getMinTargetP, setMinTargetP, iidm::optional<double>(), 1.5, iidm::optional<double>(1.5));
    WRITE_READ(apc, getMaxTargetP, setMaxTargetP, iidm::optional<double>(150.0), 151.0, iidm::optional<double>(151.0));

    CoordinatedReactiveControl crc = g1.getExtension<CoordinatedReactiveControl>();
    WRITE(crc, getQPercent, setQPercent, 50.0, 51.0);

    GeneratorShortCircuit gsc = g1.getExtension<GeneratorShortCircuit>();
    WRITE(gsc, getDirectSubtransX, setDirectSubtransX, 0.2, 1.2);
    WRITE(gsc, getDirectTransX, setDirectTransX, 0.3, 1.3);
    WRITE(gsc, getStepUpTransformerX, setStepUpTransformerX, 0.1, 1.1);

    GeneratorEntsoeCategory category = g1.getExtension<GeneratorEntsoeCategory>();
    WRITE(category, getCode, setCode, 1, 2);

    GeneratorStartup startup = g1.getExtension<GeneratorStartup>();
    WRITE(startup, getPlannedActivePowerSetpoint, setPlannedActivePowerSetpoint, 90.0, 91.0);
    WRITE(startup, getStartupCost, setStartupCost, 5.0, 6.0);
    WRITE(startup, getMarginalCost, setMarginalCost, 10.0, 11.0);
    WRITE(startup, getPlannedOutageRate, setPlannedOutageRate, 0.01, 0.02);
    WRITE(startup, getForcedOutageRate, setForcedOutageRate, 0.02, 0.03);

    GeneratorFortescue gf = g1.getExtension<GeneratorFortescue>();
    WRITE(gf, isGrounded, setGrounded, true, false);
    WRITE(gf, getRz, setRz, 0.1, 1.1);
    WRITE(gf, getXz, setXz, 0.2, 1.2);
    WRITE(gf, getRn, setRn, 0.3, 1.3);
    WRITE(gf, getXn, setXn, 0.4, 1.4);
    WRITE(gf, getGroundingR, setGroundingR, 0.5, 1.5);
    WRITE(gf, getGroundingX, setGroundingX, 0.6, 1.6);

    InjectionObservability injectionObs = g1.getExtension<InjectionObservability>();
    WRITE(injectionObs, isObservable, setObservable, true, false);

    ObservabilityQuality quality = *injectionObs.getQualityP();
    WRITE(quality, getStandardDeviation, setStandardDeviation, 0.1, 1.1);

    ManualFrequencyRestorationReserve mfrr = g1.getExtension<ManualFrequencyRestorationReserve>();
    WRITE(mfrr, isParticipate, setParticipate, true, false);

    ConnectablePosition::Feeder feeder = *g1.getExtension<ConnectablePosition>().getFeeder();
    WRITE_READ(feeder, getName, setName, iidm::optional<std::string>("G1 feeder"), "G1 feeder2", iidm::optional<std::string>("G1 feeder2"));
    WRITE_READ(feeder, getOrder, setOrder, iidm::optional<int>(10), 11, iidm::optional<int>(11));
    WRITE(feeder, getDirection, setDirection, ConnectablePosition::Direction::TOP, ConnectablePosition::Direction::BOTTOM);

    DynamicModelInfo modelInfo = g1.getExtension<DynamicModelInfo>();
    WRITE(modelInfo, getModelName, setModelName, "GeneratorSynchronous", "GeneratorSynchronous2");

    Measurement measurement = g1.getExtension<Measurements>().getMeasurements().at(0);
    READ(measurement, getId, iidm::optional<std::string>("M1"));
    READ(measurement, getType, MeasurementType::ACTIVE_POWER);
    WRITE(measurement, getValue, setValue, 100.0, 101.0);
    WRITE(measurement, getStandardDeviation, setStandardDeviation, 1.0, 2.0);
    WRITE(measurement, isValueValid, setValueValid, true, false);

    ReferencePriority priority = g1.getExtension<ReferencePriorities>().getReferencePriorities().at(0);
    READ(priority, getPriority, 1);

    Load load = *n.getLoad("L");
    WRITE(load, getP0, setP0, 80.0, 81.0);
    WRITE(load, getQ0, setQ0, 10.0, 11.0);
    WRITE(load, getLoadType, setLoadType, LoadType::AUXILIARY, LoadType::FICTITIOUS);

    Connectable connectable = load;
    WRITE(connectable, isEquivalent, setEquivalent, false, true);

    LoadDetail detail = load.getExtension<LoadDetail>();
    WRITE(detail, getFixedActivePower, setFixedActivePower, 40.0, 41.0);
    WRITE(detail, getFixedReactivePower, setFixedReactivePower, 5.0, 6.0);
    WRITE(detail, getVariableActivePower, setVariableActivePower, 40.0, 41.0);
    WRITE(detail, getVariableReactivePower, setVariableReactivePower, 5.0, 6.0);

    LoadAsymmetrical asymmetrical = load.getExtension<LoadAsymmetrical>();
    WRITE(asymmetrical, getConnectionType, setConnectionType, LoadConnectionType::Y, LoadConnectionType::DELTA);
    WRITE(asymmetrical, getDeltaPa, setDeltaPa, 1.0, 2.0);
    WRITE(asymmetrical, getDeltaQa, setDeltaQa, 0.5, 1.5);
    WRITE(asymmetrical, getDeltaPb, setDeltaPb, 0.0, 1.0);
    WRITE(asymmetrical, getDeltaQb, setDeltaQb, 0.0, 1.0);
    WRITE(asymmetrical, getDeltaPc, setDeltaPc, -1.0, 0.0);
    WRITE(asymmetrical, getDeltaQc, setDeltaQc, -0.5, 0.5);

    LoadModel model = *n.getLoad("LZIP")->getModel();
    READ(model, getType, LoadModelType::ZIP);

    ZipLoadModel zip = model.as<ZipLoadModel>();
    WRITE(zip, getC0p, setC0p, 0.5, 1.5);
    WRITE(zip, getC1p, setC1p, 0.3, 1.3);
    WRITE(zip, getC2p, setC2p, 0.2, 1.2);
    WRITE(zip, getC0q, setC0q, 0.5, 1.5);
    WRITE(zip, getC1q, setC1q, 0.3, 1.3);
    WRITE(zip, getC2q, setC2q, 0.2, 1.2);

    ExponentialLoadModel exponential = n.getLoad("LEXP")->getModel()->as<ExponentialLoadModel>();
    WRITE(exponential, getNp, setNp, 1.5, 2.5);
    WRITE(exponential, getNq, setNq, 2.0, 3.0);

    Battery battery = *n.getBattery("BAT");
    WRITE(battery, getTargetP, setTargetP, 10.0, 11.0);
    WRITE(battery, getMinP, setMinP, -50.0, -49.0);
    WRITE(battery, getMaxP, setMaxP, 50.0, 51.0);

    MinMaxReactiveLimits minMax = battery.getReactiveLimits<MinMaxReactiveLimits>();
    READ(minMax, getMinQ, -20.0);
    READ(minMax, getMaxQ, 20.0);

    BatteryShortCircuit bsc = battery.getExtension<BatteryShortCircuit>();
    WRITE(bsc, getDirectSubtransX, setDirectSubtransX, 0.2, 1.2);
    WRITE(bsc, getDirectTransX, setDirectTransX, 0.3, 1.3);
    WRITE(bsc, getStepUpTransformerX, setStepUpTransformerX, 0.1, 1.1);

    ShuntCompensator shunt = *n.getShuntCompensator("SC1");
    READ(shunt, getSectionCount, 1);
    WRITE_READ(shunt, findSectionCount, setSectionCount, iidm::optional<int>(1), 2, iidm::optional<int>(2));
    WRITE_READ(shunt, getSolvedSectionCount, setSolvedSectionCount, iidm::optional<int>(), 1, iidm::optional<int>(1));
    READ(shunt, getMaximumSectionCount, 2);
    READ(shunt, getB, 2e-05);
    READ(shunt, getG, 0.0);
    READ(shunt, getModelType, ShuntCompensatorModelType::LINEAR);

    ShuntCompensatorLinearModel linear = shunt.getModel<ShuntCompensatorLinearModel>();
    WRITE(linear, getBPerSection, setBPerSection, 1e-05, 1.00001);
    WRITE(linear, getGPerSection, setGPerSection, 0.0, 1.0);
    linear.setMaximumSectionCount(3);
    EXPECT_EQ(shunt.getMaximumSectionCount(), 3);

    ShuntCompensatorNonLinearModel::Section section = n.getShuntCompensator("SC2")->getModel<ShuntCompensatorNonLinearModel>().getAllSections().at(1);
    WRITE(section, getB, setB, 2e-05, 1.00002);
    WRITE(section, getG, setG, 1e-07, 1.0000001);

    StaticVarCompensator svc = *n.getStaticVarCompensator("SVC");
    WRITE(svc, getBmin, setBmin, -0.001, 0.999);
    WRITE(svc, getBmax, setBmax, 0.001, 1.001);

    StandbyAutomaton standby = svc.getExtension<StandbyAutomaton>();
    WRITE(standby, isStandby, setStandby, false, true);
    WRITE(standby, getB0, setB0, 0.0001, 1.0001);
    WRITE(standby, getLowVoltageSetpoint, setLowVoltageSetpoint, 390.0, 391.0);
    WRITE(standby, getHighVoltageSetpoint, setHighVoltageSetpoint, 410.0, 411.0);
    WRITE(standby, getLowVoltageThreshold, setLowVoltageThreshold, 385.0, 386.0);
    WRITE(standby, getHighVoltageThreshold, setHighVoltageThreshold, 415.0, 416.0);

    HvdcConverterStation vsc = *n.getHvdcConverterStation("VSC1");
    READ(vsc, getHvdcType, HvdcType::VSC);
    WRITE(vsc, getLossFactor, setLossFactor, 1.1f, 2.5);

    LccConverterStation lcc = *n.getLccConverterStation("LCC1");
    WRITE_READ(lcc, getPowerFactor, setPowerFactor, 0.8f, 0.9, 0.9f);

    HvdcLine hvdc = *n.getHvdcLine("HVDC");
    WRITE(hvdc, getR, setR, 1.0, 2.0);
    WRITE(hvdc, getNominalV, setNominalV, 400.0, 401.0);
    WRITE(hvdc, getActivePowerSetpoint, setActivePowerSetpoint, 100.0, 101.0);
    WRITE(hvdc, getMaxP, setMaxP, 200.0, 201.0);
    WRITE(hvdc, getConvertersMode, setConvertersMode, HvdcConverterStationMode::RECTIFIER, HvdcConverterStationMode::INVERTER);

    HvdcAngleDroopActivePowerControl droop = hvdc.getExtension<HvdcAngleDroopActivePowerControl>();
    WRITE(droop, getDroop, setDroop, 1.0, 2.0);
    WRITE(droop, getP0, setP0, 100.0, 101.0);
    WRITE(droop, isEnabled, setEnabled, true, false);

    HvdcOperatorActivePowerRange range = hvdc.getExtension<HvdcOperatorActivePowerRange>();
    WRITE(range, getOprFromCS1toCS2, setOprFromCS1toCS2, 150.0, 151.0);
    WRITE(range, getOprFromCS2toCS1, setOprFromCS2toCS1, 120.0, 121.0);

    TwoWindingsTransformer t2 = *n.getTwoWindingsTransformer("T2");
    WRITE(t2, getR, setR, 0.5, 1.5);
    WRITE(t2, getX, setX, 20.0, 21.0);
    WRITE(t2, getG, setG, 0.0, 1.0);
    WRITE(t2, getB, setB, 0.0, 1.0);
    WRITE(t2, getRatedU1, setRatedU1, 400.0, 401.0);
    WRITE(t2, getRatedU2, setRatedU2, 225.0, 226.0);
    WRITE_READ(t2, getRatedS, setRatedS, iidm::optional<double>(300.0), 301.0, iidm::optional<double>(301.0));

    PhaseTapChanger ptc = *t2.getPhaseTapChanger();
    WRITE(ptc, getRegulationMode, setRegulationMode, PhaseTapChangerRegulationMode::CURRENT_LIMITER, PhaseTapChangerRegulationMode::ACTIVE_POWER_CONTROL);
    WRITE(ptc, getRegulationValue, setRegulationValue, 500.0, 501.0);
    WRITE(ptc, getTargetDeadband, setTargetDeadband, 5.0, 6.0);
    ptc.setRegulating(true);
    EXPECT_TRUE(ptc.isRegulating());

    PhaseTapChangerStep phaseStep = ptc.getStep(0);
    WRITE(phaseStep, getAlpha, setAlpha, -5.0, -4.0);

    OperationalLimitsGroup group = t2.getOperationalLimitsGroups1().at(0);
    READ(group, getId, "DEFAULT");
    READ(group, isEmpty, false);

    LoadingLimits currentLimits = *t2.getCurrentLimits1();
    READ(currentLimits, getLimitType, LimitType::CURRENT);
    READ(currentLimits, getDetectionKind, DetectionKind::HIGH);
    WRITE(currentLimits, getPermanentLimit, setPermanentLimit, 800.0, 801.0);
    WRITE(currentLimits, getPermanentLimitName, setPermanentLimitName, "permanent", "permanent2");

    TemporaryLimit temporaryLimit = currentLimits.getTemporaryLimits().at(0);
    READ(temporaryLimit, getName, "TL10");
    READ(temporaryLimit, getValue, 1000.0);
    READ(temporaryLimit, getAcceptableDuration, 600);
    READ(temporaryLimit, isFictitious, false);

    TwoWindingsTransformerToBeEstimated t2Estimated = t2.getExtension<TwoWindingsTransformerToBeEstimated>();
    WRITE(t2Estimated, shouldEstimateRatioTapChanger, shouldEstimateRatioTapChanger, true, false);
    WRITE(t2Estimated, shouldEstimatePhaseTapChanger, shouldEstimatePhaseTapChanger, false, true);

    TwoWindingsTransformerFortescue t2Fortescue = t2.getExtension<TwoWindingsTransformerFortescue>();
    WRITE(t2Fortescue, getRz, setRz, 0.1, 1.1);
    WRITE(t2Fortescue, getXz, setXz, 0.2, 1.2);
    WRITE(t2Fortescue, isFreeFluxes, setFreeFluxes, true, false);
    WRITE(t2Fortescue, getXm, setXm, 1.0, 2.0);
    WRITE(t2Fortescue, getConnectionType1, setConnectionType1, WindingConnectionType::Y_GROUNDED, WindingConnectionType::DELTA);
    WRITE(t2Fortescue, getConnectionType2, setConnectionType2, WindingConnectionType::DELTA, WindingConnectionType::Y);
    WRITE(t2Fortescue, getGroundingR1, setGroundingR1, 0.0, 1.0);
    WRITE(t2Fortescue, getGroundingX1, setGroundingX1, 0.0, 1.0);
    WRITE(t2Fortescue, getGroundingR2, setGroundingR2, 0.0, 1.0);
    WRITE(t2Fortescue, getGroundingX2, setGroundingX2, 0.0, 1.0);

    TwoWindingsTransformerPhaseAngleClock t2Clock = t2.getExtension<TwoWindingsTransformerPhaseAngleClock>();
    WRITE(t2Clock, getPhaseAngleClock, setPhaseAngleClock, 11, 5);

    DiscreteMeasurement discrete = t2.getExtension<DiscreteMeasurements>().getDiscreteMeasurements().at(0);
    READ(discrete, getId, iidm::optional<std::string>("DM1"));
    READ(discrete, getType, DiscreteMeasurementType::TAP_POSITION);
    READ(discrete, getValueType, DiscreteMeasurementValueType::INT);
    READ(discrete, getValueAsInt, 0);
    WRITE(discrete, isValueValid, setValueValid, true, false);
    discrete.setValue(1);
    EXPECT_EQ(discrete.getValueAsInt(), 1);
    discrete.setValue(true);
    EXPECT_TRUE(discrete.getValueAsBoolean());
    discrete.setValue(std::string("up"));
    EXPECT_EQ(discrete.getValueAsString(), "up");

    ThreeWindingsTransformer t3 = *n.getThreeWindingsTransformer("T3");
    WRITE(t3, getRatedU0, setRatedU0, 400.0, 401.0);
    READ(t3, isOverloaded, false);
    READ(t3, getOverloadDuration, 2147483647);

    ThreeWindingsTransformer::Leg leg = t3.getLeg1();
    READ(leg, getSide, ThreeSides::ONE);
    WRITE(leg, getR, setR, 0.1, 1.1);
    WRITE(leg, getX, setX, 5.0, 6.0);
    WRITE(leg, getG, setG, 0.0, 1.0);
    WRITE(leg, getB, setB, 0.0, 1.0);
    WRITE(leg, getRatedU, setRatedU, 400.0, 401.0);
    WRITE_READ(leg, getRatedS, setRatedS, iidm::optional<double>(200.0), 201.0, iidm::optional<double>(201.0));

    ThreeWindingsTransformerPhaseAngleClock t3Clock = t3.getExtension<ThreeWindingsTransformerPhaseAngleClock>();
    WRITE(t3Clock, getPhaseAngleClockLeg2, setPhaseAngleClockLeg2, 1, 2);
    WRITE(t3Clock, getPhaseAngleClockLeg3, setPhaseAngleClockLeg3, 2, 3);

    LegFortescue legFortescue = t3.getExtension<ThreeWindingsTransformerFortescue>().getLeg1();
    WRITE(legFortescue, getRz, setRz, 0.1, 1.1);
    WRITE(legFortescue, getXz, setXz, 0.2, 1.2);
    WRITE(legFortescue, isFreeFluxes, setFreeFluxes, false, true);
    WRITE(legFortescue, getConnectionType, setConnectionType, WindingConnectionType::Y, WindingConnectionType::DELTA);
    WRITE(legFortescue, getGroundingR, setGroundingR, 0.0, 1.0);
    WRITE(legFortescue, getGroundingX, setGroundingX, 0.0, 1.0);

    Line l12 = *n.getLine("L12");
    WRITE(l12, getR, setR, 1.0, 2.0);
    WRITE(l12, getX, setX, 10.0, 11.0);
    WRITE(l12, getG1, setG1, 0.0, 1.0);
    WRITE(l12, getB1, setB1, 1e-06, 1.000001);
    WRITE(l12, getG2, setG2, 0.0, 1.0);
    WRITE(l12, getB2, setB2, 1e-06, 1.000001);

    LineFortescue lineFortescue = l12.getExtension<LineFortescue>();
    WRITE(lineFortescue, getRz, setRz, 0.1, 1.1);
    WRITE(lineFortescue, getXz, setXz, 0.3, 1.3);
    WRITE(lineFortescue, getG1z, setG1z, 0.0, 1.0);
    WRITE(lineFortescue, getB1z, setB1z, 0.0, 1.0);
    WRITE(lineFortescue, getG2z, setG2z, 0.0, 1.0);
    WRITE(lineFortescue, getB2z, setB2z, 0.0, 1.0);
    WRITE(lineFortescue, isOpenPhaseA, setOpenPhaseA, false, true);
    WRITE(lineFortescue, isOpenPhaseB, setOpenPhaseB, false, true);
    WRITE(lineFortescue, isOpenPhaseC, setOpenPhaseC, false, true);

    BranchObservability branchObs = l12.getExtension<BranchObservability>();
    WRITE(branchObs, isObservable, setObservable, true, false);

    MutualCoupling coupling = n.getExtension<LineCouplings>().getMutualCouplings().at(0);
    WRITE(coupling, getR, setR, 0.1, 1.1);
    WRITE(coupling, getX, setX, 0.5, 1.5);

    BoundaryLine bl = *n.getBoundaryLine("BL1");
    WRITE(bl, getP0, setP0, 10.0, 11.0);
    WRITE(bl, getQ0, setQ0, 5.0, 6.0);
    WRITE(bl, getR, setR, 1.0, 2.0);
    WRITE(bl, getX, setX, 10.0, 11.0);
    WRITE(bl, getG, setG, 0.0, 1.0);
    WRITE(bl, getB, setB, 0.0, 1.0);
    READ(bl, isPaired, false);
    WRITE_READ(bl, getPairingKey, setPairingKey, iidm::optional<std::string>("XNODE1"), "XNODE9", iidm::optional<std::string>("XNODE9"));

    BoundaryLine::Generation generation = *bl.getGeneration();
    WRITE(generation, getTargetP, setTargetP, 10.0, 11.0);
    WRITE(generation, getMinP, setMinP, 0.0, 1.0);
    WRITE(generation, getMaxP, setMaxP, 100.0, 101.0);
    WRITE(generation, getTargetQ, setTargetQ, 0.0, 1.0);
    WRITE(generation, isVoltageRegulationOn, setVoltageRegulationOn, false, true);
    WRITE(generation, getTargetV, setTargetV, 400.0, 401);

    Boundary boundary = bl.getBoundary();
    READ(boundary, getV, NAN);
    READ(boundary, getAngle, NAN);
    READ(boundary, getP, NAN);
    READ(boundary, getQ, NAN);
    READ(boundary, getI, NAN);

    TieLine tieLine = *n.getTieLine("TL");
    READ(tieLine, getPairingKey, iidm::optional<std::string>("XNODE2"));
    READ(tieLine, getR, 1.9999999999999991);
    READ(tieLine, getX, 19.999999999999996);
    READ(tieLine, getG1, 1.7347235e-18f);
    READ(tieLine, getB1, 1.3877788e-17f);
    READ(tieLine, getG2, 1.7347235e-18f);
    READ(tieLine, getB2, 1.3877788e-17f);

    Area area = *n.getArea("A1");
    READ(area, getAreaType, "ControlArea");
    WRITE_READ(area, getInterchangeTarget, setInterchangeTarget, iidm::optional<double>(100.0), 101.0, iidm::optional<double>(101.0));
    READ(area, getAcInterchange, 0.0);
    READ(area, getDcInterchange, 0.0);
    READ(area, getInterchange, 0.0);

    AreaBoundary areaBoundary = area.getAreaBoundaries().at(0);
    READ(areaBoundary, isAc, true);
    READ(areaBoundary, getP, NAN);
    READ(areaBoundary, getQ, NAN);

    VoltageAngleLimit angleLimit = *n.getVoltageAngleLimit("VAL");
    READ(angleLimit, getId, "VAL");
    READ(angleLimit, getLimitType, LimitType::VOLTAGE_ANGLE);
    READ(angleLimit, getLowLimit, iidm::optional<double>(-10.0));
    READ(angleLimit, getHighLimit, iidm::optional<double>(10.0));

    OverloadManagementSystem oms = *n.getOverloadManagementSystem("OMS");
    WRITE(oms, isEnabled, setEnabled, true, false);
    READ(oms, getMonitoredElementId, "T2");
    READ(oms, getMonitoredSide, ThreeSides::ONE);

    OverloadManagementSystem::Tripping tripping = oms.getTrippings().at(0);
    READ(tripping, getType, OverloadManagementSystem::Tripping::Type::SWITCH_TRIPPING);
    READ(tripping, getKey, "sw");
    WRITE(tripping, getNameOrKey, setName, "sw", "sw2");
    WRITE(tripping, getCurrentLimit, setCurrentLimit, 1000.0, 1001.0);
    WRITE(tripping, isOpenAction, setOpenAction, true, false);

    OverloadManagementSystem::SwitchTripping switchTripping = tripping.as<OverloadManagementSystem::SwitchTripping>();
    WRITE(switchTripping, getSwitchToOperateId, setSwitchToOperateId, "BR3", "DIS3");

    OverloadManagementSystem::BranchTripping branchTripping = oms.getTrippings().at(1).as<OverloadManagementSystem::BranchTripping>();
    WRITE(branchTripping, getBranchToOperateId, setBranchToOperateId, "T2", "L12");
    WRITE(branchTripping, getSideToOperate, setSideToOperate, TwoSides::ONE, TwoSides::TWO);

    OverloadManagementSystem::ThreeWindingsTransformerTripping t3Tripping = oms.getTrippings().at(2).as<OverloadManagementSystem::ThreeWindingsTransformerTripping>();
    READ(t3Tripping, getThreeWindingsTransformerToOperateId, "T3");
    READ(t3Tripping, getSideToOperate, ThreeSides::TWO);

    IdentifiableShortCircuit isc = vl1.getExtension<IdentifiableShortCircuit>();
    WRITE(isc, getIpMin, setIpMin, 1000.0, 1001.0);
    WRITE(isc, getIpMax, setIpMax, 5000.0, 5001.0);

    SlackTerminal slack = vl1.getExtension<SlackTerminal>();
    READ(slack, isEmpty, false);

    ObservabilityArea observabilityArea = vl1.getExtension<ObservabilityArea>();
    READ(observabilityArea, isConsistentWithTopology, true);

    ObservabilityArea::AreaCharacteristics areaCharacteristics = observabilityArea.getObservabilityAreas().at(0);
    READ(areaCharacteristics, getAreaNumber, 1);
    READ(areaCharacteristics, getStatus, ObservabilityStatus::OBSERVABLE);

    ControlZone zone = n.getExtension<SecondaryVoltageControl>().getControlZones().at(0);
    READ(zone, getName, "z1");

    PilotPoint pilotPoint = zone.getPilotPoint();
    WRITE(pilotPoint, getTargetV, setTargetV, 225.0, 226.0);

    ControlUnit unit = zone.getControlUnits().at(0);
    READ(unit, getId, "G2");
    WRITE(unit, isParticipate, setParticipate, true, false);

    Identifiable l4 = *n.getIdentifiable("L4");
    WRITE(l4, getId, setId, "L4", "L9");
    READ(l4, getNameOrId, "L9");
    WRITE_READ(l4, getOptionalName, setName, iidm::optional<std::string>(), "x", iidm::optional<std::string>("x"));
    WRITE(l4, isFictitious, setFictitious, false, true);
    READ(l4, getType, IdentifiableType::LOAD);

    VariantManager variants = n.getVariantManager();
    WRITE(variants, getWorkingVariantId, setWorkingVariant, "InitialState", "SECOND");
    WRITE(variants, isVariantMultiThreadAccessAllowed, allowVariantMultiThreadAccess, false, true);
}

void checkShapeAccessors(BackendProvider& b) {
    test::buildShapeNetwork(b);
    Network n = Identifiable(b.getNetworkHandle(), &b).getNetwork();

    ReactiveCapabilityShapePlane plane = n.getGenerator("G")->getReactiveLimits<ReactiveCapabilityShape>().getPlanes().at(2);
    READ(plane, getAlpha, 0.0);
    READ(plane, getBeta, 0.5);
    READ(plane, getGamma, 150.0);
    READ(plane, getDelta, 1.0);
    READ(plane, isLessOrEqual, true);
    READ(plane, isGreaterOrEqual, false);
    plane.greaterOrEqual(120);
    EXPECT_TRUE(plane.isGreaterOrEqual());
    EXPECT_DOUBLE_EQ(plane.getGamma(), 120);
    plane.lessOrEqual(160);
    EXPECT_TRUE(plane.isLessOrEqual());
    EXPECT_DOUBLE_EQ(plane.getGamma(), 160);
}
} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(AccessorsTest, GraalVM) {
    const char* lib = iidm::test::envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    {
        GraalVMBackend backend(lib);
        checkAccessors(backend);
    }
    GraalVMBackend backend(lib);
    checkShapeAccessors(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(AccessorsTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    {
        JNIBackend backend("accessors");
        checkAccessors(backend);
    }
    JNIBackend backend("accessors_shape");
    checkShapeAccessors(backend);
}
#endif
