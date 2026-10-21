#pragma once

// The network that the C++ adders build on a backend: every type of object and every extension, with
// known values (buildNetwork); and a generator with a reactive capability shape, which powsybl does not
// save yet (buildShapeNetwork). Shared by the conformance test and the accessor table.

#include <iidm/iidm.h>

namespace iidm {
namespace test {

// The rest of the network built from C++: every other type of object, and every extension
inline void addEverything(Network n, Substation s, VoltageLevel vl1, VoltageLevel vl2, Generator g1, Load load) {
    s.addGeographicalTag("north");
    s.newExtension<SubstationPositionAdder>().withCoordinate(Coordinate(48.85, 2.35)).add();
    vl1.getBusBreakerView().newSwitch().setId("CPL1").setBus1("B1").setBus2("B1b").setOpen(false).add();
    vl1.newExtension<IdentifiableShortCircuitAdder>().withIpMin(1000).withIpMax(5000).add();
    vl1.newExtension<SlackTerminalAdder>().withTerminal(g1.getTerminal()).add();
    vl1.newExtension<ObservabilityAreaAdder>().withObservabilityAreaByBusBreakerViewBuses({"B1", "B1b"}, 1, ObservabilityStatus::OBSERVABLE).add();

    // Generator: its reactive limits and extensions
    g1.newReactiveCapabilityCurve().setProperty("source", "datasheet")
        .beginPoint().setP(0).setMinQ(-100).setMaxQ(100).endPoint()
        .beginPoint().setP(200).setMinQ(-80).setMaxQ(80).endPoint()
        .add();
    g1.newExtension<CoordinatedReactiveControlAdder>().withQPercent(50).add();
    g1.newExtension<GeneratorShortCircuitAdder>().withDirectSubtransX(0.2).withDirectTransX(0.3).withStepUpTransformerX(0.1).add();
    g1.newExtension<GeneratorEntsoeCategoryAdder>().withCode(1).add();
    g1.newExtension<GeneratorStartupAdder>().withPlannedActivePowerSetpoint(90).withStartupCost(5).withMarginalCost(10)
        .withPlannedOutageRate(0.01).withForcedOutageRate(0.02).add();
    g1.newExtension<GeneratorFortescueAdder>().withGrounded(true).withRz(0.1).withXz(0.2).withRn(0.3).withXn(0.4).withGroundingR(0.5)
        .withGroundingX(0.6).add();
    g1.newExtension<InjectionObservabilityAdder>().withObservable(true).withStandardDeviationP(0.1).withRedundantP(true).add();
    g1.newExtension<ManualFrequencyRestorationReserveAdder>().withParticipate(true).add();
    g1.newExtension<ConnectablePositionAdder>().newFeeder().withName("G1 feeder").withOrder(10)
        .withDirection(ConnectablePosition::Direction::TOP).add().add();
    g1.newExtension<DynamicModelInfoAdder>().setModelName("GeneratorSynchronous").add();
    g1.newExtension<MeasurementsAdder>().add().newMeasurement().setId("M1").setType(MeasurementType::ACTIVE_POWER).setValue(100)
        .setStandardDeviation(1).setValid(true).add();
    ReferencePriority::set(g1, 1);

    // Loads: details, asymmetry, models
    load.newExtension<LoadDetailAdder>().withFixedActivePower(40).withFixedReactivePower(5).withVariableActivePower(40)
        .withVariableReactivePower(5).add();
    load.newExtension<LoadAsymmetricalAdder>().withConnectionType(LoadConnectionType::Y).withDeltaPa(1).withDeltaQa(0.5).withDeltaPb(0)
        .withDeltaQb(0).withDeltaPc(-1).withDeltaQc(-0.5).add();
    vl2.newLoad().setId("LZIP").setBus("B2").setP0(10).setQ0(2)
        .newZipModel().setC0p(0.5).setC1p(0.3).setC2p(0.2).setC0q(0.5).setC1q(0.3).setC2q(0.2).add()
        .add();
    vl2.newLoad().setId("LEXP").setBus("B2").setP0(10).setQ0(2).newExponentialModel().setNp(1.5).setNq(2).add().add();

    // Batteries, shunt compensators, static var compensators, converter stations, HVDC lines
    Battery battery = vl1.newBattery().setId("BAT").setBus("B1").setMinP(-50).setMaxP(50).setTargetP(10)
        .setLocalTargetQ(0).add();
    battery.newMinMaxReactiveLimits().setMinQ(-20).setMaxQ(20).add();
    battery.newExtension<BatteryShortCircuitAdder>().withDirectSubtransX(0.2).withDirectTransX(0.3).withStepUpTransformerX(0.1).add();
    vl1.newShuntCompensator().setId("SC1").setBus("B1").setSectionCount(1)
        .newLinearModel().setBPerSection(1e-5).setGPerSection(0).setMaximumSectionCount(2).add()
        .add();
    vl2.newShuntCompensator().setId("SC2").setBus("B2").setSectionCount(2).setSolvedSectionCount(1)
        .newNonLinearModel()
            .beginSection().setB(1e-5).setG(0).endSection()
            .beginSection().setB(2e-5).setG(1e-7).endSection()
            .add()
        .add();
    StaticVarCompensator svc = vl1.newStaticVarCompensator().setId("SVC").setBus("B1").setBmin(-1e-3).setBmax(1e-3)
        .setLocalTargetV(400)
        .newVoltageRegulation().withMode(RegulationMode::VOLTAGE).withRegulating(true).add()
        .add();
    svc.newExtension<StandbyAutomatonAdder>().withStandbyStatus(false).withB0(1e-4).withLowVoltageSetpoint(390).withHighVoltageSetpoint(410)
        .withLowVoltageThreshold(385).withHighVoltageThreshold(415).add();
    VscConverterStation vsc1 = vl1.newVscConverterStation().setId("VSC1").setBus("B1").setLossFactor(1.1)
        .setLocalTargetQ(0).add();
    // A copy of the generator's curve (a shape would prevent the export: see buildShapeNetwork)
    vsc1.newReactiveCapabilityCurve(g1.getReactiveLimits<ReactiveCapabilityCurve>()).add();
    vl2.newVscConverterStation().setId("VSC2").setBus("B2").setLossFactor(1.1).setLocalTargetQ(0).add();
    vl1.newLccConverterStation().setId("LCC1").setBus("B1").setLossFactor(1).setPowerFactor(0.8).add();
    vl2.newLccConverterStation().setId("LCC2").setBus("B2").setLossFactor(1).setPowerFactor(0.8).add();
    HvdcLine hvdc = n.newHvdcLine().setId("HVDC").setR(1).setNominalV(400).setActivePowerSetpoint(100).setMaxP(200)
        .setConvertersMode(HvdcConverterStationMode::RECTIFIER).setConverterStationId1("VSC1").setConverterStationId2("VSC2")
        .add();
    hvdc.newExtension<HvdcAngleDroopActivePowerControlAdder>().withP0(100).withDroop(1).withEnabled(true).add();
    hvdc.newExtension<HvdcOperatorActivePowerRangeAdder>().withOprFromCS1toCS2(150).withOprFromCS2toCS1(120).add();
    n.newHvdcLine().setId("HVDC_LCC").setR(1).setNominalV(400).setActivePowerSetpoint(50).setMaxP(100)
        .setConvertersMode(HvdcConverterStationMode::INVERTER).setConverterStationId1("LCC1").setConverterStationId2("LCC2")
        .add();

    // A node-breaker voltage level, and a third one for a three windings transformer
    VoltageLevel vl3 = s.newVoltageLevel().setId("VL3").setNominalV(225).setTopologyKind(TopologyKind::NODE_BREAKER).add();
    VoltageLevel::NodeBreakerView nbv = vl3.getNodeBreakerView();
    BusbarSection bbs = nbv.newBusbarSection().setId("BBS3").setNode(0).add();
    bbs.newExtension<BusbarSectionPositionAdder>().withBusbarIndex(1).withSectionIndex(1).add();
    bbs.newExtension<OperatingStatusAdder>().withStatus(OperatingStatus::Status::IN_OPERATION).add();
    nbv.newBreaker().setId("BR3").setNode1(0).setNode2(1).setOpen(false).add();
    nbv.newDisconnector().setId("DIS3").setNode1(0).setNode2(2).setOpen(false).add();
    nbv.newSwitch().setId("LBS3").setKind(SwitchKind::LOAD_BREAK_SWITCH).setNode1(0).setNode2(3).setOpen(false)
        .setRetained(true).add();
    nbv.newInternalConnection().setNode1(3).setNode2(4).add();
    nbv.newBreaker().setId("BR3T2").setNode1(0).setNode2(5).setOpen(false).add();
    nbv.newBreaker().setId("BR3T3").setNode1(0).setNode2(6).setOpen(false).add();
    vl3.newLoad().setId("L3").setNode(1).setP0(30).setQ0(3).add()
        .newExtension<ConnectablePositionAdder>().newFeeder().withName("L3").withOrder(1).withDirection(ConnectablePosition::Direction::BOTTOM)
        .add().add();
    vl3.newGenerator().setId("G4").setNode(2).setMinP(0).setMaxP(50).setTargetP(10).setLocalTargetQ(0).add();
    vl3.newGround().setId("GROUND3").setNode(4).add();
    vl3.newExtension<ObservabilityAreaAdder>().withObservabilityAreaByNodes({0, 1}, 1, ObservabilityStatus::BORDER).add();
    VoltageLevel vl4 = s.newVoltageLevel().setId("VL4").setNominalV(63).setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl4.getBusBreakerView().newBus().setId("B4").add();
    vl4.newLoad().setId("L4").setBus("B4").setP0(20).setQ0(2).add();

    // Transformers, their tap changers, limits and extensions
    TwoWindingsTransformer t2 = s.newTwoWindingsTransformer().setId("T2").setVoltageLevel1("VL1").setBus1("B1")
        .setVoltageLevel2("VL3").setNode2(5).setR(0.5).setX(20).setG(0).setB(0).setRatedU1(400).setRatedU2(225)
        .setRatedS(300).add();
    t2.newRatioTapChanger().setLowTapPosition(-1).setTapPosition(0).setLoadTapChangingCapabilities(true)
        .beginStep().setRho(0.95).setR(0).setX(0).setG(0).setB(0).endStep()
        .beginStep().setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .beginStep().setRho(1.05).setR(0).setX(0).setG(0).setB(0).setProperty("note", "high").endStep()
        .newVoltageRegulation().withMode(RegulationMode::VOLTAGE).withTargetValue(225).withTargetDeadband(1)
            .withTerminal(t2.getTerminal2()).withRegulating(true).add()
        .add();
    t2.newPhaseTapChanger().setLowTapPosition(0).setTapPosition(1)
        .setRegulationMode(PhaseTapChangerRegulationMode::CURRENT_LIMITER).setRegulationValue(500).setTargetDeadband(5)
        .setRegulating(false).setRegulationTerminal(t2.getTerminal1())
        .beginStep().setAlpha(-5).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .beginStep().setAlpha(5).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .add();
    t2.newOperationalLimitsGroup1("DEFAULT").newCurrentLimits().setPermanentLimit(800)
        .beginTemporaryLimit().setName("TL10").setAcceptableDuration(600).setValue(1000).endTemporaryLimit()
        .add();
    t2.setSelectedOperationalLimitsGroup1("DEFAULT");
    t2.newExtension<TwoWindingsTransformerToBeEstimatedAdder>().withRatioTapChangerStatus(true).withPhaseTapChangerStatus(false).add();
    t2.newExtension<TwoWindingsTransformerFortescueAdder>().withRz(0.1).withXz(0.2).withFreeFluxes(true).withXm(1)
        .withConnectionType1(WindingConnectionType::Y_GROUNDED).withConnectionType2(WindingConnectionType::DELTA)
        .withGroundingR1(0).withGroundingX1(0).withGroundingR2(0).withGroundingX2(0).add();
    t2.newExtension<TwoWindingsTransformerPhaseAngleClockAdder>().withPhaseAngleClock(11).add();
    t2.newExtension<DiscreteMeasurementsAdder>().add().newDiscreteMeasurement().setId("DM1").setType(DiscreteMeasurementType::TAP_POSITION)
        .setTapChanger(DiscreteMeasurementTapChanger::RATIO_TAP_CHANGER).setValue(0).setValid(true).add();
    ThreeWindingsTransformer t3 = s.newThreeWindingsTransformer().setId("T3").setRatedU0(400)
        .newLeg1().setR(0.1).setX(5).setG(0).setB(0).setRatedU(400).setRatedS(200).setVoltageLevel("VL1").setBus("B1").add()
        .newLeg2().setR(0.2).setX(6).setG(0).setB(0).setRatedU(225).setVoltageLevel("VL3").setNode(6).add()
        .newLeg3().setR(0.3).setX(7).setG(0).setB(0).setRatedU(63).setVoltageLevel("VL4").setBus("B4").add()
        .add();
    t3.getLeg2().newRatioTapChanger().setLowTapPosition(0).setTapPosition(0)
        .beginStep().setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .add();
    t3.getLeg3().newPhaseTapChanger().setLowTapPosition(0).setTapPosition(0)
        .setRegulationMode(PhaseTapChangerRegulationMode::ACTIVE_POWER_CONTROL).setRegulationValue(10).setRegulating(false)
        .beginStep().setAlpha(0).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .add();
    t3.getLeg1().newOperationalLimitsGroup("WINTER").newCurrentLimits().setPermanentLimit(500).add();
    t3.newExtension<ThreeWindingsTransformerToBeEstimatedAdder>().withRatioTapChanger2Status(true).add();
    t3.newExtension<ThreeWindingsTransformerFortescueAdder>().leg1().withRz(0.1).withXz(0.2).withFreeFluxes(false)
        .withConnectionType(WindingConnectionType::Y).withGroundingR(0).withGroundingX(0)
        .leg2().withRz(0.1).withXz(0.2).withConnectionType(WindingConnectionType::DELTA)
        .leg3().withRz(0.1).withXz(0.2).withConnectionType(WindingConnectionType::Y_GROUNDED)
        .add();
    t3.newExtension<ThreeWindingsTransformerPhaseAngleClockAdder>().withPhaseAngleClockLeg2(1).withPhaseAngleClockLeg3(2).add();

    // Lines, boundary lines and a tie line
    Line l12 = n.newLine().setId("L12").setVoltageLevel1("VL1").setBus1("B1").setVoltageLevel2("VL2").setBus2("B2")
        .setR(1).setX(10).setG1(0).setB1(1e-6).setG2(0).setB2(1e-6).add();
    Line l12b = n.newLine().setId("L12b").setVoltageLevel1("VL1").setBus1("B1").setVoltageLevel2("VL2").setBus2("B2")
        .setR(1.5).setX(12).setG1(0).setB1(1e-6).setG2(0).setB2(1e-6).add();
    l12.newOperationalLimitsGroup1("DEFAULT").newCurrentLimits().setPermanentLimit(1000)
        .beginTemporaryLimit().setName("TL20").setAcceptableDuration(1200).setValue(1200).endTemporaryLimit()
        .add();
    l12.setSelectedOperationalLimitsGroup1("DEFAULT");
    l12.newOperationalLimitsGroup2("DEFAULT").newActivePowerLimits().setPermanentLimit(400).add();
    l12.newExtension<LineFortescueAdder>().withRz(0.1).withXz(0.3).withG1z(0).withB1z(0).withG2z(0).withB2z(0).add();
    l12.newExtension<LinePositionAdder>().withCoordinates({Coordinate(48.85, 2.35), Coordinate(49, 3)}).add();
    l12.newExtension<BranchObservabilityAdder>().withObservable(true).withStandardDeviationP1(0.1).withRedundantP1(false).add();
    l12.newExtension<OperatingStatusAdder>().withStatus(OperatingStatus::Status::PLANNED_OUTAGE).add();
    l12.newExtension<ConnectablePositionAdder>()
        .newFeeder1().withName("L12 1").withOrder(1).withDirection(ConnectablePosition::Direction::TOP).add()
        .newFeeder2().withName("L12 2").withOrder(2).withDirection(ConnectablePosition::Direction::BOTTOM).add()
        .add();
    l12.newExtension<MeasurementsAdder>().add().newMeasurement().setId("M12").setType(MeasurementType::CURRENT).setSide(ThreeSides::ONE)
        .setValue(100).setStandardDeviation(1).setValid(true).add();
    ReferencePriority::set(l12, TwoSides::TWO, 2);
    BoundaryLine bl = vl1.newBoundaryLine().setId("BL1").setBus("B1").setP0(10).setQ0(5).setR(1).setX(10).setG(0).setB(0)
        .setPairingKey("XNODE1")
        .newGeneration().setMinP(0).setMaxP(100).setTargetP(10).setTargetQ(0).setTargetV(400).setVoltageRegulationOn(false).add()
        .add();
    bl.getGeneration()->newMinMaxReactiveLimits().setMinQ(-50).setMaxQ(50).add();
    bl.newExtension<LinePositionAdder>().withCoordinates({Coordinate(48.85, 2.35), Coordinate(50, 4)}).add();
    vl1.newBoundaryLine().setId("BLT1").setBus("B1").setP0(0).setQ0(0).setR(1).setX(10).setG(0).setB(0)
        .setPairingKey("XNODE2").add();
    vl2.newBoundaryLine().setId("BLT2").setBus("B2").setP0(0).setQ0(0).setR(1).setX(10).setG(0).setB(0)
        .setPairingKey("XNODE2").add();
    n.newTieLine().setId("TL").setBoundaryLine1("BLT1").setBoundaryLine2("BLT2").add();

    // Areas, voltage angle limits, overload management systems
    // One voltage level and one boundary through the adder, the others added to the area: powsybl's area
    // adder keeps them in hash sets, so they would be saved in an order of its own on each backend
    Area area = n.newArea().setId("A1").setName("area").setAreaType("ControlArea").setInterchangeTarget(100)
        .addVoltageLevel(vl1).addAreaBoundary(l12.getTerminal1(), true).add();
    area.addVoltageLevel(vl2);
    area.newAreaBoundary().setBoundary(bl.getBoundary()).setAc(true).add();
    n.newVoltageAngleLimit().setId("VAL").from(g1.getTerminal()).to(load.getTerminal()).setLowLimit(-10).setHighLimit(10).add();
    s.newOverloadManagementSystem().setId("OMS").setEnabled(true).setMonitoredElementId("T2")
        .setMonitoredElementSide(ThreeSides::ONE)
        .newSwitchTripping().setKey("sw").setCurrentLimit(1000).setOpenAction(true).setSwitchToOperateId("BR3").add()
        .newBranchTripping().setKey("br").setCurrentLimit(1100).setOpenAction(true).setBranchToOperateId("T2")
            .setSideToOperate(TwoSides::ONE).add()
        .newThreeWindingsTransformerTripping().setKey("t3").setCurrentLimit(1200).setOpenAction(true)
            .setThreeWindingsTransformerToOperateId("T3").setSideToOperate(ThreeSides::TWO).add()
        .add();

    // Network extensions, and a subnetwork
    n.newExtension<ReferenceTerminalsAdder>().withTerminals({g1.getTerminal()}).add();
    n.newExtension<SecondaryVoltageControlAdder>()
        .newControlZone().withName("z1")
            .newPilotPoint().withBusbarSectionsOrBusesIds({"B2"}).withTargetV(225).add()
            .newControlUnit().withId("G2").add()
            .add()
        .add();
    n.newExtension<LineCouplingsAdder>().add().newMutualCoupling().withLine1(l12).withLine2(l12b).withR(0.1).withX(0.5)
        .withLine1Segment(LineSegment(0, 0.5)).add();
    Network sub = n.createSubnetwork("SUB", "sub", "test");
    sub.setCaseDate("2026-01-31T12:00Z"); // else the creation time
    sub.newSubstation().setId("SUB_S").add().newVoltageLevel().setId("SUB_VL").setNominalV(63)
        .setTopologyKind(TopologyKind::BUS_BREAKER).add().getBusBreakerView().newBus().setId("SUB_B").add();
}

// A generator with a reactive capability shape: powsybl does not export those yet (both backends then
// fail to save alike), so they have a network of their own
inline void buildShapeNetwork(BackendProvider& b) {
    b.createNetwork("shape");
    const ObjectHandle network = b.getNetworkHandle();
    b.setString(network, prop::NETWORK_CASE_DATE, "2026-01-31T12:00Z");
    VoltageLevel vl = VoltageLevelAdder(b.invoke(network, prop::VOLTAGE_LEVEL), &b)
                        .setId("VL").setNominalV(400).setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl.getBusBreakerView().newBus().setId("B").add();
    vl.newLoad().setId("L").setBus("B").setP0(50).setQ0(5).add();
    vl.newGenerator().setId("G").setBus("B").setMinP(0).setMaxP(200).setTargetP(100).setLocalTargetQ(0).add()
        .newReactiveCapabilityShape().setProperty("source", "c++")
        .addPlane(0, 0, 1, true, -100)
        .addPlane(0, 0, 1, false, 100)
        .addPlane(0, 0.5, 1, false, 150)
        .add();
}

// The same network built through the C++ adders on each backend: every type of object, and every
// extension (addEverything)
inline void buildNetwork(BackendProvider& b) {
    b.createNetwork("created");
    const ObjectHandle network = b.getNetworkHandle();
    b.setString(network, prop::NETWORK_CASE_DATE, "2026-01-31T12:00Z"); // else the creation time
    b.setInt(network, prop::NETWORK_FORECAST_DISTANCE, 60);
    Substation s = SubstationAdder(b.invoke(network, prop::SUBSTATION), &b)
                     .setId("S").setCountry(Country::FR).setTso("RTE").add();
    VoltageLevel vl1 = s.newVoltageLevel().setId("VL1").setNominalV(400).setLowVoltageLimit(380)
                         .setHighVoltageLimit(420).setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl1.getBusBreakerView().newBus().setId("B1").add();
    vl1.getBusBreakerView().newBus().setId("B1b").setName("spare").add();
    // Without substation
    VoltageLevel vl2 = VoltageLevelAdder(b.invoke(network, prop::VOLTAGE_LEVEL), &b)
                         .setId("VL2").setNominalV(225).setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl2.getBusBreakerView().newBus().setId("B2").add();
    Load load = vl2.newLoad().setId("L").setName("load").setBus("B2").setConnectableBus("B2")
                  .setP0(80).setQ0(10).setLoadType(LoadType::AUXILIARY).add();
    vl2.newLoad().setId("L2").setConnectableBus("B2").setP0(5).setQ0(1).setFictitious(true).add(); // disconnected
    Generator g1 = vl1.newGenerator().setId("G1").setBus("B1").setMinP(0).setMaxP(200).setTargetP(100)
                     .setRatedS(250).setEnergySource(EnergySource::NUCLEAR).setLocalTargetV(405).setLocalTargetQ(0)
                     .newVoltageRegulation().withMode(RegulationMode::VOLTAGE).withRegulating(true).withTargetDeadband(1).add()
                     .add();
    g1.newExtension<ActivePowerControlAdder>().withDroop(4).withParticipate(true).withParticipationFactor(0.5).withMaxTargetP(150).add();
    // Properties, aliases, names out of ASCII (a 4-byte UTF-8 character too)
    g1.setProperty("owner", "EDF").setProperty("unit", "1");
    g1.addAlias("G1-alias").addAlias("G1-code", "code");
    s.setProperty("region", "\xc3\x8ele-de-France");
    vl1.setName("Poste \xe2\x82\xac \xf0\x9d\x84\x9e");
    Identifiable(network, &b).setProperty("study", "conformance");
    vl2.newGenerator().setId("G2").setBus("B2").setMinP(-50).setMaxP(50).setTargetP(10).setLocalTargetQ(5)
        .newVoltageRegulation().withMode(RegulationMode::REACTIVE_POWER).withTerminal(load.getTerminal())
            .withTargetValue(-10).withRegulating(true).add()
        .add();
    vl2.newGenerator().setId("G3").setBus("B2").setMinP(0).setMaxP(10).setTargetP(1).setLocalTargetQ(0).add();
    vl1.newGround().setId("GROUND").setBus("B1b").add();
    addEverything(s.getNetwork(), s, vl1, vl2, g1, load);
    // A second variant, for the walk of the variant manager
    s.getNetwork().getVariantManager().cloneVariant(VariantManager::INITIAL_VARIANT_ID, "SECOND");
}

} // namespace test
} // namespace iidm
