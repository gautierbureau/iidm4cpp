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
    jmethodID registry_listIds = nullptr;

    // ── List ─────────────────────────────────────────────────────────────
    jmethodID list_size       = nullptr;
    jmethodID list_get        = nullptr;
    jmethodID list_iterator   = nullptr;

    // ── EnergySource / enum ordinal ──────────────────────────────────────
    jmethodID enum_ordinal = nullptr;
    jclass energySourceClass  = nullptr;
    jclass loadTypeClass      = nullptr;
    jclass topologyKindClass  = nullptr;
    jclass convertersModeClass = nullptr;
    jclass regulationModeClass = nullptr;
    jclass countryClass        = nullptr;
};

} // namespace iidm
