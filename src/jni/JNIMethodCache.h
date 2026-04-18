#pragma once

#include <jni.h>

namespace iidm {

struct JNIMethodCache {
    // ── Classes ──────────────────────────────────────────────────────────
    jclass networkClass         = nullptr;
    jclass generatorClass       = nullptr;
    jclass loadClass            = nullptr;
    jclass lineClass            = nullptr;
    jclass twoWTClass           = nullptr;
    jclass threeWTClass         = nullptr;
    jclass hvdcLineClass        = nullptr;
    jclass danglingLineClass    = nullptr;
    jclass shuntClass           = nullptr;
    jclass svcClass             = nullptr;
    jclass vscClass             = nullptr;
    jclass lccClass             = nullptr;
    jclass terminalClass        = nullptr;
    jclass busClass             = nullptr;
    jclass substationClass      = nullptr;
    jclass voltageLevelClass    = nullptr;
    jclass switchClass          = nullptr;
    jclass iidmRegistryClass    = nullptr;
    jclass listClass            = nullptr;

    // ── Network methods ──────────────────────────────────────────────────
    jmethodID network_getId             = nullptr;
    jmethodID network_getName           = nullptr;
    jmethodID network_getGenerators     = nullptr;
    jmethodID network_getLoads          = nullptr;
    jmethodID network_getLines          = nullptr;
    jmethodID network_getTwoWT          = nullptr;
    jmethodID network_getThreeWT        = nullptr;
    jmethodID network_getHvdcLines      = nullptr;
    jmethodID network_getDanglingLines  = nullptr;
    jmethodID network_getShunts         = nullptr;
    jmethodID network_getSVCs           = nullptr;
    jmethodID network_getVSCs           = nullptr;
    jmethodID network_getLCCs           = nullptr;
    jmethodID network_getSubstations    = nullptr;
    jmethodID network_getVoltageLevels  = nullptr;
    jmethodID network_getGenerator      = nullptr;
    jmethodID network_getLoad           = nullptr;
    jmethodID network_getLine           = nullptr;

    // ── Generator methods ────────────────────────────────────────────────
    jmethodID generator_getId                 = nullptr;
    jmethodID generator_getName               = nullptr;
    jmethodID generator_getTargetP            = nullptr;
    jmethodID generator_setTargetP            = nullptr;
    jmethodID generator_getTargetQ            = nullptr;
    jmethodID generator_setTargetQ            = nullptr;
    jmethodID generator_getTargetV            = nullptr;
    jmethodID generator_setTargetV            = nullptr;
    jmethodID generator_getMinP               = nullptr;
    jmethodID generator_getMaxP               = nullptr;
    jmethodID generator_isVoltageRegulatorOn  = nullptr;
    jmethodID generator_setVoltageRegulatorOn = nullptr;
    jmethodID generator_getEnergySource       = nullptr;
    jmethodID generator_getTerminal           = nullptr;

    // ── Load methods ─────────────────────────────────────────────────────
    jmethodID load_getId        = nullptr;
    jmethodID load_getName      = nullptr;
    jmethodID load_getP0        = nullptr;
    jmethodID load_setP0        = nullptr;
    jmethodID load_getQ0        = nullptr;
    jmethodID load_setQ0        = nullptr;
    jmethodID load_getLoadType  = nullptr;
    jmethodID load_getTerminal  = nullptr;

    // ── Terminal methods ─────────────────────────────────────────────────
    jmethodID terminal_getP          = nullptr;
    jmethodID terminal_setP          = nullptr;
    jmethodID terminal_getQ          = nullptr;
    jmethodID terminal_setQ          = nullptr;
    jmethodID terminal_isConnected   = nullptr;
    jmethodID terminal_connect       = nullptr;
    jmethodID terminal_disconnect    = nullptr;
    jmethodID terminal_getBusView    = nullptr;

    // ── Bus methods ──────────────────────────────────────────────────────
    jmethodID bus_getId    = nullptr;
    jmethodID bus_getName  = nullptr;
    jmethodID bus_getV     = nullptr;
    jmethodID bus_setV     = nullptr;
    jmethodID bus_getAngle = nullptr;
    jmethodID bus_setAngle = nullptr;

    // ── Substation methods ───────────────────────────────────────────────
    jmethodID substation_getId             = nullptr;
    jmethodID substation_getName           = nullptr;
    jmethodID substation_getCountry        = nullptr;
    jmethodID substation_getVoltageLevels  = nullptr;

    // ── VoltageLevel methods ─────────────────────────────────────────────
    jmethodID vl_getId               = nullptr;
    jmethodID vl_getName             = nullptr;
    jmethodID vl_getNominalV         = nullptr;
    jmethodID vl_getLowVoltageLimit  = nullptr;
    jmethodID vl_getHighVoltageLimit = nullptr;
    jmethodID vl_getTopologyKind     = nullptr;

    // ── Line methods ─────────────────────────────────────────────────────
    jmethodID line_getId        = nullptr;
    jmethodID line_getName      = nullptr;
    jmethodID line_getR         = nullptr;
    jmethodID line_getX         = nullptr;
    jmethodID line_getG1        = nullptr;
    jmethodID line_getB1        = nullptr;
    jmethodID line_getG2        = nullptr;
    jmethodID line_getB2        = nullptr;
    jmethodID line_getTerminal1 = nullptr;
    jmethodID line_getTerminal2 = nullptr;

    // ── IidmBridgeRegistry ───────────────────────────────────────────────
    jmethodID registry_get     = nullptr;
    jmethodID registry_save    = nullptr;
    jmethodID registry_listIds = nullptr;

    // ── List ─────────────────────────────────────────────────────────────
    jmethodID list_size       = nullptr;
    jmethodID list_get        = nullptr;
    jmethodID list_iterator   = nullptr;

    // ── Optional (used for getSubstation) ────────────────────────────────────
    jclass    optionalClass       = nullptr;
    jmethodID optional_isPresent  = nullptr;
    jmethodID optional_get        = nullptr;

    // ── VoltageLevel → Substation navigation ─────────────────────────────────
    jmethodID vl_getSubstation    = nullptr;

    // ── Substation country setter ─────────────────────────────────────────────
    jmethodID substation_setCountry = nullptr;
    jmethodID country_valueOf       = nullptr;

    // ── TwoWindingsTransformer tap changers ──────────────────────────────────
    jclass    tapChangerClass              = nullptr;
    jclass    ratioTapChangerClass         = nullptr;
    jclass    phaseTapChangerClass         = nullptr;
    jclass    ptcRegulationModeClass       = nullptr;
    jmethodID twt_getRatioTapChanger       = nullptr;
    jmethodID twt_getPhaseTapChanger       = nullptr;
    jmethodID tc_getTapPosition            = nullptr;
    jmethodID tc_setTapPosition            = nullptr;
    jmethodID tc_getLowTapPosition         = nullptr;
    jmethodID tc_getHighTapPosition        = nullptr;
    jmethodID tc_isRegulating              = nullptr;
    jmethodID tc_setRegulating             = nullptr;
    jmethodID rtc_getTargetV               = nullptr;
    jmethodID rtc_setTargetV               = nullptr;
    jmethodID ptc_getRegulationMode        = nullptr;
    jmethodID ptc_setRegulationMode        = nullptr;
    jmethodID ptc_getRegulationValue       = nullptr;
    jmethodID ptc_setRegulationValue       = nullptr;

    // ── Injection (shared getTerminal for all single-terminal components) ────
    jclass    injectionClass       = nullptr;
    jmethodID injection_getTerminal = nullptr;

    // ── BusbarSection ────────────────────────────────────────────────────────
    jclass    busbarSectionClass          = nullptr;
    jmethodID bbs_getV                    = nullptr;
    jmethodID bbs_getAngle                = nullptr;
    jmethodID bbs_getTerminal             = nullptr;
    jmethodID nbView_getBusbarSections    = nullptr;
    jmethodID network_getBusbarSection    = nullptr;

    // ── Switch methods ───────────────────────────────────────────────────────
    jmethodID switch_isOpen      = nullptr;
    jmethodID switch_setOpen     = nullptr;
    jmethodID switch_isRetained  = nullptr;
    jmethodID switch_setRetained = nullptr;
    jmethodID switch_getKind     = nullptr;

    // ── VoltageLevel topology views (for Switch navigation) ──────────────────
    jclass    vlBusBreakerViewClass  = nullptr;
    jclass    vlNodeBreakerViewClass = nullptr;
    jmethodID vl_getBusBreakerView   = nullptr;
    jmethodID vl_getNodeBreakerView  = nullptr;
    jmethodID bbView_getSwitches     = nullptr;
    jmethodID nbView_getSwitches     = nullptr;

    // ── Network getSwitch by ID ───────────────────────────────────────────────
    jmethodID network_getSwitch = nullptr;

    // ── EnergySource / enum ordinal ──────────────────────────────────────
    jmethodID enum_ordinal = nullptr;
    jclass energySourceClass  = nullptr;
    jclass loadTypeClass      = nullptr;
    jclass topologyKindClass  = nullptr;
    jclass switchKindClass    = nullptr;
    jclass convertersModeClass = nullptr;
    jclass regulationModeClass = nullptr;
    jclass countryClass        = nullptr;

    // ── ActivePowerControl extension ─────────────────────────────────────────
    jclass    activePowerControlClass      = nullptr;
    jmethodID generator_getExtensionByName = nullptr;
    jmethodID apc_getDroop                 = nullptr;
    jmethodID apc_setDroop                 = nullptr;
    jmethodID apc_isParticipate            = nullptr;
    jmethodID apc_setParticipate           = nullptr;

    // ── Shared Extendable.getExtensionByName (used for new extensions) ────────
    jmethodID extendable_getExtensionByName = nullptr;

    // ── CoordinatedReactiveControl extension (on Generator) ───────────────────
    jclass    coordinatedReactiveControlClass = nullptr;
    jmethodID crc_getQPercent                 = nullptr;
    jmethodID crc_setQPercent                 = nullptr;

    // ── HvdcAngleDroopActivePowerControl extension (on HvdcLine) ─────────────
    jclass    hvdcAngleDroopActivePowerControlClass = nullptr;
    jmethodID hadapc_getDroop                       = nullptr;
    jmethodID hadapc_setDroop                       = nullptr;
    jmethodID hadapc_getP0                          = nullptr;
    jmethodID hadapc_setP0                          = nullptr;
    jmethodID hadapc_isEnabled                      = nullptr;
    jmethodID hadapc_setEnabled                     = nullptr;

    // ── HvdcOperatorActivePowerRange extension (on HvdcLine) ─────────────────
    jclass    hvdcOperatorActivePowerRangeClass = nullptr;
    jmethodID hoar_getOprFromCS1toCS2           = nullptr;
    jmethodID hoar_setOprFromCS1toCS2           = nullptr;
    jmethodID hoar_getOprFromCS2toCS1           = nullptr;
    jmethodID hoar_setOprFromCS2toCS1           = nullptr;

    // ── VoltagePerReactivePowerControl extension (on StaticVarCompensator) ────
    jclass    voltagePerReactivePowerControlClass = nullptr;
    jmethodID vprc_getSlope                       = nullptr;
    jmethodID vprc_setSlope                       = nullptr;

    // ── SlackTerminal extension (on VoltageLevel) ─────────────────────────────
    jclass    slackTerminalClass   = nullptr;
    jmethodID st_getTerminal       = nullptr;

    // ── Battery ────────────────────────────────────────────────────────────────
    jclass    batteryClass          = nullptr;
    jmethodID battery_getTargetP    = nullptr;
    jmethodID battery_setTargetP    = nullptr;
    jmethodID battery_getTargetQ    = nullptr;
    jmethodID battery_setTargetQ    = nullptr;
    jmethodID battery_getMinP       = nullptr;
    jmethodID battery_getMaxP       = nullptr;
    jmethodID network_getBatteries  = nullptr;
    jmethodID network_getBattery    = nullptr;

    // ── Reactive limits ────────────────────────────────────────────────────────
    jclass    reactiveLimitsClass   = nullptr;
    jclass    minMaxRLClass         = nullptr;
    jclass    rcCurveClass          = nullptr;
    jclass    rcPointClass          = nullptr;
    jmethodID gen_getReactiveLimits = nullptr;
    jmethodID vsc_getReactiveLimits = nullptr;
    jmethodID minMaxRL_getMinQ      = nullptr;
    jmethodID minMaxRL_getMaxQ      = nullptr;
    jmethodID rcCurve_getPoints     = nullptr;
    jmethodID rcPoint_getP          = nullptr;
    jmethodID rcPoint_getMinQ       = nullptr;
    jmethodID rcPoint_getMaxQ       = nullptr;

    // ── TapChangerStep ────────────────────────────────────────────────────────
    jclass    rtcStepClass          = nullptr;
    jclass    ptcStepClass          = nullptr;
    jmethodID tc_getAllSteps         = nullptr;
    jmethodID rtcStep_getRho        = nullptr;
    jmethodID rtcStep_getR          = nullptr;
    jmethodID rtcStep_getX          = nullptr;
    jmethodID rtcStep_getG          = nullptr;
    jmethodID rtcStep_getB          = nullptr;
    jmethodID ptcStep_getAlpha      = nullptr;
    jmethodID ptcStep_getRho        = nullptr;
    jmethodID ptcStep_getR          = nullptr;
    jmethodID ptcStep_getX          = nullptr;
    jmethodID ptcStep_getG          = nullptr;
    jmethodID ptcStep_getB          = nullptr;

    // ── ThreeWindingsTransformer Leg ──────────────────────────────────────────
    jclass    threeWTLegClass           = nullptr;
    jmethodID threeWT_getLeg1           = nullptr;
    jmethodID threeWT_getLeg2           = nullptr;
    jmethodID threeWT_getLeg3           = nullptr;
    jmethodID leg_getR                  = nullptr;
    jmethodID leg_getX                  = nullptr;
    jmethodID leg_getG                  = nullptr;
    jmethodID leg_getB                  = nullptr;
    jmethodID leg_getRatedU             = nullptr;
    jmethodID leg_getRatedS             = nullptr;
    jmethodID leg_getRatioTapChanger    = nullptr;
    jmethodID leg_getPhaseTapChanger    = nullptr;
    // ── ShuntCompensator non-linear model ─────────────────────────────────────
    jclass    shuntNLModelClass          = nullptr;
    jclass    shuntSectionClass          = nullptr;
    jmethodID shunt_getModelType         = nullptr;
    jmethodID shunt_getNonLinearModel    = nullptr;
    jmethodID shuntNLModel_getAllSections = nullptr;
    jmethodID shuntSection_getB          = nullptr;
    jmethodID shuntSection_getG          = nullptr;

    // ── VoltageLevel BusBreakerView.getBuses() ────────────────────────────────
    jmethodID bbView_getBuses            = nullptr;

    // ── Terminal BusBreakerView / NodeBreakerView ─────────────────────────────
    jclass    termBBViewClass                = nullptr;
    jclass    termNBViewClass                = nullptr;
    jmethodID terminal_getBusBreakerView     = nullptr;
    jmethodID terminal_getNodeBreakerView    = nullptr;
    jmethodID termBBView_getConnectableBus   = nullptr;
    jmethodID termNBView_getNode             = nullptr;

    // ── ThreeWT/TwoWT findById ────────────────────────────────────────────────
    jmethodID network_getThreeWTById    = nullptr;
    jmethodID network_getTwoWTById      = nullptr;
};

} // namespace iidm
