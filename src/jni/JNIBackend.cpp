#include "JNIBackend.h"
#include "JNIExceptionBridge.h"
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include <cmath>
#include <string>

namespace iidm {

// ── Lifecycle ─────────────────────────────────────────────────────────────────

JNIBackend::JNIBackend(const std::string& networkId)
    : networkId_(networkId.empty() ? "default" : networkId) {
    attachToJVM();
    cacheMethodIds();
}

JNIBackend::~JNIBackend() {
    close();
}

void JNIBackend::attachToJVM() {
    JavaVM* jvms[1];
    jsize   count = 0;
    jint rc = JNI_GetCreatedJavaVMs(jvms, 1, &count);
    if (rc != JNI_OK || count == 0) {
        throw BackendNotAvailableException("No JVM found; cannot use JNI backend");
    }
    jvm_ = jvms[0];

    JNIEnv* env = nullptr;
    rc = jvm_->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
    if (rc != JNI_OK) {
        throw IidmException("Failed to attach to JVM thread");
    }
    env_ = env;
}

void JNIBackend::cacheClass(jclass& out, const char* name) {
    jclass local = env_->FindClass(name);
    checkJNIException(env_);
    if (!local) throw IidmException(std::string("Cannot find class: ") + name);
    out = static_cast<jclass>(env_->NewGlobalRef(local));
    env_->DeleteLocalRef(local);
}

void JNIBackend::cacheMethodIds() {
    cacheClass(cache_.networkClass,      "com/powsybl/iidm/network/Network");
    cacheClass(cache_.generatorClass,    "com/powsybl/iidm/network/Generator");
    cacheClass(cache_.loadClass,         "com/powsybl/iidm/network/Load");
    cacheClass(cache_.lineClass,         "com/powsybl/iidm/network/Line");
    cacheClass(cache_.terminalClass,     "com/powsybl/iidm/network/Terminal");
    cacheClass(cache_.busClass,          "com/powsybl/iidm/network/Bus");
    cacheClass(cache_.substationClass,   "com/powsybl/iidm/network/Substation");
    cacheClass(cache_.voltageLevelClass, "com/powsybl/iidm/network/VoltageLevel");
    cacheClass(cache_.switchClass,       "com/powsybl/iidm/network/Switch");
    cacheClass(cache_.iidmRegistryClass, "com/powsybl/iidmbridge/jni/IidmBridgeRegistry");
    cacheClass(cache_.listClass,         "java/util/List");
    cacheClass(cache_.energySourceClass, "com/powsybl/iidm/network/EnergySource");
    cacheClass(cache_.loadTypeClass,     "com/powsybl/iidm/network/LoadType");
    cacheClass(cache_.topologyKindClass, "com/powsybl/iidm/network/TopologyKind");
    cacheClass(cache_.switchKindClass,   "com/powsybl/iidm/network/SwitchKind");
    cacheClass(cache_.countryClass,      "com/powsybl/iidm/network/Country");

    // Enum ordinal (shared across all enums)
    jclass enumClass = env_->FindClass("java/lang/Enum");
    cache_.enum_ordinal = env_->GetMethodID(enumClass, "ordinal", "()I");
    env_->DeleteLocalRef(enumClass);

    // IidmBridgeRegistry
    cache_.registry_get = env_->GetStaticMethodID(cache_.iidmRegistryClass,
        "get", "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Network;");
    cache_.registry_save = env_->GetStaticMethodID(cache_.iidmRegistryClass,
        "save", "(Ljava/lang/String;Ljava/lang/String;)V");
    checkJNIException(env_);

    // Network
    cache_.network_getId            = env_->GetMethodID(cache_.networkClass, "getId",   "()Ljava/lang/String;");
    cache_.network_getName          = env_->GetMethodID(cache_.networkClass, "getNameOrId", "()Ljava/lang/String;");
    cache_.network_getGenerators    = env_->GetMethodID(cache_.networkClass, "getGenerators",    "()Ljava/lang/Iterable;");
    cache_.network_getLoads         = env_->GetMethodID(cache_.networkClass, "getLoads",         "()Ljava/lang/Iterable;");
    cache_.network_getLines         = env_->GetMethodID(cache_.networkClass, "getLines",         "()Ljava/lang/Iterable;");
    cache_.network_getSubstations   = env_->GetMethodID(cache_.networkClass, "getSubstations",   "()Ljava/lang/Iterable;");
    cache_.network_getVoltageLevels = env_->GetMethodID(cache_.networkClass, "getVoltageLevels", "()Ljava/lang/Iterable;");
    cache_.network_getGenerator     = env_->GetMethodID(cache_.networkClass, "getGenerator",
        "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Generator;");
    cache_.network_getLoad          = env_->GetMethodID(cache_.networkClass, "getLoad",
        "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Load;");
    cache_.network_getLine          = env_->GetMethodID(cache_.networkClass, "getLine",
        "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Line;");
    checkJNIException(env_);

    // Generator
    cache_.generator_getId                 = env_->GetMethodID(cache_.generatorClass, "getId",   "()Ljava/lang/String;");
    cache_.generator_getName               = env_->GetMethodID(cache_.generatorClass, "getNameOrId", "()Ljava/lang/String;");
    cache_.generator_getTargetP            = env_->GetMethodID(cache_.generatorClass, "getTargetP", "()D");
    cache_.generator_setTargetP            = env_->GetMethodID(cache_.generatorClass, "setTargetP", "(D)Lcom/powsybl/iidm/network/Generator;");
    cache_.generator_getTargetQ            = env_->GetMethodID(cache_.generatorClass, "getTargetQ", "()D");
    cache_.generator_setTargetQ            = env_->GetMethodID(cache_.generatorClass, "setTargetQ", "(D)Lcom/powsybl/iidm/network/Generator;");
    cache_.generator_getTargetV            = env_->GetMethodID(cache_.generatorClass, "getTargetV", "()D");
    cache_.generator_setTargetV            = env_->GetMethodID(cache_.generatorClass, "setTargetV", "(D)Lcom/powsybl/iidm/network/Generator;");
    cache_.generator_getMinP               = env_->GetMethodID(cache_.generatorClass, "getMinP", "()D");
    cache_.generator_getMaxP               = env_->GetMethodID(cache_.generatorClass, "getMaxP", "()D");
    cache_.generator_isVoltageRegulatorOn  = env_->GetMethodID(cache_.generatorClass, "isVoltageRegulatorOn", "()Z");
    cache_.generator_setVoltageRegulatorOn = env_->GetMethodID(cache_.generatorClass, "setVoltageRegulatorOn", "(Z)Lcom/powsybl/iidm/network/Generator;");
    cache_.generator_getEnergySource       = env_->GetMethodID(cache_.generatorClass, "getEnergySource",
        "()Lcom/powsybl/iidm/network/EnergySource;");
    cache_.generator_getTerminal           = env_->GetMethodID(cache_.generatorClass, "getTerminal",
        "()Lcom/powsybl/iidm/network/Terminal;");
    checkJNIException(env_);

    // Load
    cache_.load_getId       = env_->GetMethodID(cache_.loadClass, "getId",   "()Ljava/lang/String;");
    cache_.load_getName     = env_->GetMethodID(cache_.loadClass, "getNameOrId", "()Ljava/lang/String;");
    cache_.load_getP0       = env_->GetMethodID(cache_.loadClass, "getP0", "()D");
    cache_.load_setP0       = env_->GetMethodID(cache_.loadClass, "setP0", "(D)Lcom/powsybl/iidm/network/Load;");
    cache_.load_getQ0       = env_->GetMethodID(cache_.loadClass, "getQ0", "()D");
    cache_.load_setQ0       = env_->GetMethodID(cache_.loadClass, "setQ0", "(D)Lcom/powsybl/iidm/network/Load;");
    cache_.load_getLoadType = env_->GetMethodID(cache_.loadClass, "getLoadType",
        "()Lcom/powsybl/iidm/network/LoadType;");
    cache_.load_getTerminal = env_->GetMethodID(cache_.loadClass, "getTerminal",
        "()Lcom/powsybl/iidm/network/Terminal;");
    checkJNIException(env_);

    // Terminal
    cache_.terminal_getP        = env_->GetMethodID(cache_.terminalClass, "getP", "()D");
    cache_.terminal_setP        = env_->GetMethodID(cache_.terminalClass, "setP", "(D)Lcom/powsybl/iidm/network/Terminal;");
    cache_.terminal_getQ        = env_->GetMethodID(cache_.terminalClass, "getQ", "()D");
    cache_.terminal_setQ        = env_->GetMethodID(cache_.terminalClass, "setQ", "(D)Lcom/powsybl/iidm/network/Terminal;");
    cache_.terminal_isConnected = env_->GetMethodID(cache_.terminalClass, "isConnected", "()Z");
    cache_.terminal_connect     = env_->GetMethodID(cache_.terminalClass, "connect",    "()Z");
    cache_.terminal_disconnect  = env_->GetMethodID(cache_.terminalClass, "disconnect", "()Z");
    checkJNIException(env_);

    // Bus
    cache_.bus_getId    = env_->GetMethodID(cache_.busClass, "getId",    "()Ljava/lang/String;");
    cache_.bus_getName  = env_->GetMethodID(cache_.busClass, "getNameOrId",  "()Ljava/lang/String;");
    cache_.bus_getV     = env_->GetMethodID(cache_.busClass, "getV",     "()D");
    cache_.bus_setV     = env_->GetMethodID(cache_.busClass, "setV",     "(D)Lcom/powsybl/iidm/network/Bus;");
    cache_.bus_getAngle = env_->GetMethodID(cache_.busClass, "getAngle", "()D");
    cache_.bus_setAngle = env_->GetMethodID(cache_.busClass, "setAngle", "(D)Lcom/powsybl/iidm/network/Bus;");
    checkJNIException(env_);

    // Line
    cache_.line_getId        = env_->GetMethodID(cache_.lineClass, "getId",   "()Ljava/lang/String;");
    cache_.line_getName      = env_->GetMethodID(cache_.lineClass, "getNameOrId", "()Ljava/lang/String;");
    cache_.line_getR         = env_->GetMethodID(cache_.lineClass, "getR",  "()D");
    cache_.line_getX         = env_->GetMethodID(cache_.lineClass, "getX",  "()D");
    cache_.line_getG1        = env_->GetMethodID(cache_.lineClass, "getG1", "()D");
    cache_.line_getB1        = env_->GetMethodID(cache_.lineClass, "getB1", "()D");
    cache_.line_getG2        = env_->GetMethodID(cache_.lineClass, "getG2", "()D");
    cache_.line_getB2        = env_->GetMethodID(cache_.lineClass, "getB2", "()D");
    cache_.line_getTerminal1 = env_->GetMethodID(cache_.lineClass, "getTerminal1",
        "()Lcom/powsybl/iidm/network/Terminal;");
    cache_.line_getTerminal2 = env_->GetMethodID(cache_.lineClass, "getTerminal2",
        "()Lcom/powsybl/iidm/network/Terminal;");
    checkJNIException(env_);

    // Substation
    cache_.substation_getId            = env_->GetMethodID(cache_.substationClass, "getId",   "()Ljava/lang/String;");
    cache_.substation_getName          = env_->GetMethodID(cache_.substationClass, "getNameOrId", "()Ljava/lang/String;");
    cache_.substation_getCountry       = env_->GetMethodID(cache_.substationClass, "getCountry",
        "()Ljava/util/Optional;");
    cache_.substation_getVoltageLevels = env_->GetMethodID(cache_.substationClass, "getVoltageLevels",
        "()Ljava/lang/Iterable;");
    checkJNIException(env_);

    // VoltageLevel
    cache_.vl_getId               = env_->GetMethodID(cache_.voltageLevelClass, "getId",   "()Ljava/lang/String;");
    cache_.vl_getName             = env_->GetMethodID(cache_.voltageLevelClass, "getNameOrId", "()Ljava/lang/String;");
    cache_.vl_getNominalV         = env_->GetMethodID(cache_.voltageLevelClass, "getNominalV", "()D");
    cache_.vl_getLowVoltageLimit  = env_->GetMethodID(cache_.voltageLevelClass, "getLowVoltageLimit",  "()D");
    cache_.vl_getHighVoltageLimit = env_->GetMethodID(cache_.voltageLevelClass, "getHighVoltageLimit", "()D");
    cache_.vl_getTopologyKind     = env_->GetMethodID(cache_.voltageLevelClass, "getTopologyKind",
        "()Lcom/powsybl/iidm/network/TopologyKind;");
    checkJNIException(env_);

    // Optional (for getSubstation)
    cacheClass(cache_.optionalClass, "java/util/Optional");
    cache_.optional_isPresent = env_->GetMethodID(cache_.optionalClass, "isPresent", "()Z");
    cache_.optional_get       = env_->GetMethodID(cache_.optionalClass, "get",       "()Ljava/lang/Object;");
    checkJNIException(env_);

    // VoltageLevel → Substation
    cache_.vl_getSubstation = env_->GetMethodID(cache_.voltageLevelClass, "getSubstation",
        "()Ljava/util/Optional;");
    checkJNIException(env_);

    // Substation country setter
    cache_.substation_setCountry = env_->GetMethodID(cache_.substationClass, "setCountry",
        "(Lcom/powsybl/iidm/network/Country;)Lcom/powsybl/iidm/network/Substation;");
    cache_.country_valueOf = env_->GetStaticMethodID(cache_.countryClass, "valueOf",
        "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Country;");
    checkJNIException(env_);

    // Switch
    cache_.switch_isOpen      = env_->GetMethodID(cache_.switchClass, "isOpen",      "()Z");
    cache_.switch_setOpen     = env_->GetMethodID(cache_.switchClass, "setOpen",     "(Z)V");
    cache_.switch_isRetained  = env_->GetMethodID(cache_.switchClass, "isRetained",  "()Z");
    cache_.switch_setRetained = env_->GetMethodID(cache_.switchClass, "setRetained", "(Z)V");
    cache_.switch_getKind     = env_->GetMethodID(cache_.switchClass, "getKind",
        "()Lcom/powsybl/iidm/network/SwitchKind;");
    checkJNIException(env_);

    // VoltageLevel topology views (for Switch child navigation)
    cacheClass(cache_.vlBusBreakerViewClass,
        "com/powsybl/iidm/network/VoltageLevel$BusBreakerView");
    cacheClass(cache_.vlNodeBreakerViewClass,
        "com/powsybl/iidm/network/VoltageLevel$NodeBreakerView");
    cache_.vl_getBusBreakerView  = env_->GetMethodID(cache_.voltageLevelClass,
        "getBusBreakerView",  "()Lcom/powsybl/iidm/network/VoltageLevel$BusBreakerView;");
    cache_.vl_getNodeBreakerView = env_->GetMethodID(cache_.voltageLevelClass,
        "getNodeBreakerView", "()Lcom/powsybl/iidm/network/VoltageLevel$NodeBreakerView;");
    cache_.bbView_getSwitches = env_->GetMethodID(cache_.vlBusBreakerViewClass,
        "getSwitches", "()Ljava/lang/Iterable;");
    cache_.nbView_getSwitches = env_->GetMethodID(cache_.vlNodeBreakerViewClass,
        "getSwitches", "()Ljava/lang/Iterable;");
    checkJNIException(env_);

    // Network.getSwitch(String id)
    cache_.network_getSwitch = env_->GetMethodID(cache_.networkClass, "getSwitch",
        "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Switch;");
    checkJNIException(env_);

    // TwoWindingsTransformer tap changers
    {
        jclass twoWTClass = env_->FindClass("com/powsybl/iidm/network/TwoWindingsTransformer");
        cache_.twt_getRatioTapChanger = env_->GetMethodID(twoWTClass, "getRatioTapChanger",
            "()Lcom/powsybl/iidm/network/RatioTapChanger;");
        cache_.twt_getPhaseTapChanger = env_->GetMethodID(twoWTClass, "getPhaseTapChanger",
            "()Lcom/powsybl/iidm/network/PhaseTapChanger;");
        env_->DeleteLocalRef(twoWTClass);
    }
    cacheClass(cache_.tapChangerClass,       "com/powsybl/iidm/network/TapChanger");
    cacheClass(cache_.ratioTapChangerClass,  "com/powsybl/iidm/network/RatioTapChanger");
    cacheClass(cache_.phaseTapChangerClass,  "com/powsybl/iidm/network/PhaseTapChanger");
    cacheClass(cache_.ptcRegulationModeClass,
        "com/powsybl/iidm/network/PhaseTapChanger$RegulationMode");
    // Common TapChanger methods (looked up on the interface; erased return type is TapChanger)
    cache_.tc_getTapPosition   = env_->GetMethodID(cache_.tapChangerClass, "getTapPosition",   "()I");
    cache_.tc_setTapPosition   = env_->GetMethodID(cache_.tapChangerClass, "setTapPosition",
        "(I)Lcom/powsybl/iidm/network/TapChanger;");
    cache_.tc_getLowTapPosition  = env_->GetMethodID(cache_.tapChangerClass, "getLowTapPosition",  "()I");
    cache_.tc_getHighTapPosition = env_->GetMethodID(cache_.tapChangerClass, "getHighTapPosition", "()I");
    cache_.tc_isRegulating     = env_->GetMethodID(cache_.tapChangerClass, "isRegulating",    "()Z");
    cache_.tc_setRegulating    = env_->GetMethodID(cache_.tapChangerClass, "setRegulating",
        "(Z)Lcom/powsybl/iidm/network/TapChanger;");
    // RatioTapChanger-specific
    cache_.rtc_getTargetV = env_->GetMethodID(cache_.ratioTapChangerClass, "getTargetV", "()D");
    cache_.rtc_setTargetV = env_->GetMethodID(cache_.ratioTapChangerClass, "setTargetV",
        "(D)Lcom/powsybl/iidm/network/RatioTapChanger;");
    // PhaseTapChanger-specific
    cache_.ptc_getRegulationMode = env_->GetMethodID(cache_.phaseTapChangerClass,
        "getRegulationMode",
        "()Lcom/powsybl/iidm/network/PhaseTapChanger$RegulationMode;");
    cache_.ptc_setRegulationMode = env_->GetMethodID(cache_.phaseTapChangerClass,
        "setRegulationMode",
        "(Lcom/powsybl/iidm/network/PhaseTapChanger$RegulationMode;)Lcom/powsybl/iidm/network/PhaseTapChanger;");
    cache_.ptc_getRegulationValue = env_->GetMethodID(cache_.phaseTapChangerClass,
        "getRegulationValue", "()D");
    cache_.ptc_setRegulationValue = env_->GetMethodID(cache_.phaseTapChangerClass,
        "setRegulationValue", "(D)Lcom/powsybl/iidm/network/PhaseTapChanger;");
    checkJNIException(env_);

    // Injection interface (shared getTerminal)
    cacheClass(cache_.injectionClass, "com/powsybl/iidm/network/Injection");
    cache_.injection_getTerminal = env_->GetMethodID(cache_.injectionClass, "getTerminal",
        "()Lcom/powsybl/iidm/network/Terminal;");
    checkJNIException(env_);

    // BusbarSection
    cacheClass(cache_.busbarSectionClass, "com/powsybl/iidm/network/BusbarSection");
    cache_.bbs_getV       = env_->GetMethodID(cache_.busbarSectionClass, "getV",       "()D");
    cache_.bbs_getAngle   = env_->GetMethodID(cache_.busbarSectionClass, "getAngle",   "()D");
    cache_.bbs_getTerminal = env_->GetMethodID(cache_.busbarSectionClass, "getTerminal",
        "()Lcom/powsybl/iidm/network/Terminal;");
    cache_.nbView_getBusbarSections = env_->GetMethodID(cache_.vlNodeBreakerViewClass,
        "getBusbarSections", "()Ljava/lang/Iterable;");
    cache_.network_getBusbarSection = env_->GetMethodID(cache_.networkClass,
        "getBusbarSection", "(Ljava/lang/String;)Lcom/powsybl/iidm/network/BusbarSection;");
    checkJNIException(env_);

    // ActivePowerControl extension
    cacheClass(cache_.activePowerControlClass,
        "com/powsybl/iidm/network/extensions/ActivePowerControl");
    cache_.generator_getExtensionByName = env_->GetMethodID(cache_.generatorClass,
        "getExtensionByName", "(Ljava/lang/String;)Lcom/powsybl/commons/extensions/Extension;");
    cache_.apc_getDroop      = env_->GetMethodID(cache_.activePowerControlClass, "getDroop",      "()D");
    cache_.apc_setDroop      = env_->GetMethodID(cache_.activePowerControlClass, "setDroop",      "(D)V");
    cache_.apc_isParticipate = env_->GetMethodID(cache_.activePowerControlClass, "isParticipate", "()Z");
    cache_.apc_setParticipate= env_->GetMethodID(cache_.activePowerControlClass, "setParticipate","(Z)V");
    checkJNIException(env_);

    // Shared getExtensionByName from the Extendable interface
    {
        jclass extendableClass = env_->FindClass("com/powsybl/commons/extensions/Extendable");
        cache_.extendable_getExtensionByName = env_->GetMethodID(extendableClass,
            "getExtensionByName", "(Ljava/lang/String;)Lcom/powsybl/commons/extensions/Extension;");
        env_->DeleteLocalRef(extendableClass);
    }
    checkJNIException(env_);

    // CoordinatedReactiveControl extension
    cacheClass(cache_.coordinatedReactiveControlClass,
        "com/powsybl/iidm/network/extensions/CoordinatedReactiveControl");
    cache_.crc_getQPercent = env_->GetMethodID(cache_.coordinatedReactiveControlClass, "getQPercent", "()D");
    cache_.crc_setQPercent = env_->GetMethodID(cache_.coordinatedReactiveControlClass, "setQPercent", "(D)Lcom/powsybl/iidm/network/extensions/CoordinatedReactiveControl;");
    checkJNIException(env_);

    // HvdcAngleDroopActivePowerControl extension
    cacheClass(cache_.hvdcAngleDroopActivePowerControlClass,
        "com/powsybl/iidm/network/extensions/HvdcAngleDroopActivePowerControl");
    cache_.hadapc_getDroop   = env_->GetMethodID(cache_.hvdcAngleDroopActivePowerControlClass, "getDroop",   "()F");
    cache_.hadapc_setDroop   = env_->GetMethodID(cache_.hvdcAngleDroopActivePowerControlClass, "setDroop",   "(F)Lcom/powsybl/iidm/network/extensions/HvdcAngleDroopActivePowerControl;");
    cache_.hadapc_getP0      = env_->GetMethodID(cache_.hvdcAngleDroopActivePowerControlClass, "getP0",      "()F");
    cache_.hadapc_setP0      = env_->GetMethodID(cache_.hvdcAngleDroopActivePowerControlClass, "setP0",      "(F)Lcom/powsybl/iidm/network/extensions/HvdcAngleDroopActivePowerControl;");
    cache_.hadapc_isEnabled  = env_->GetMethodID(cache_.hvdcAngleDroopActivePowerControlClass, "isEnabled",  "()Z");
    cache_.hadapc_setEnabled = env_->GetMethodID(cache_.hvdcAngleDroopActivePowerControlClass, "setEnabled", "(Z)Lcom/powsybl/iidm/network/extensions/HvdcAngleDroopActivePowerControl;");
    checkJNIException(env_);

    // HvdcOperatorActivePowerRange extension
    cacheClass(cache_.hvdcOperatorActivePowerRangeClass,
        "com/powsybl/iidm/network/extensions/HvdcOperatorActivePowerRange");
    cache_.hoar_getOprFromCS1toCS2 = env_->GetMethodID(cache_.hvdcOperatorActivePowerRangeClass, "getOprFromCS1toCS2", "()F");
    cache_.hoar_setOprFromCS1toCS2 = env_->GetMethodID(cache_.hvdcOperatorActivePowerRangeClass, "setOprFromCS1toCS2", "(F)Lcom/powsybl/iidm/network/extensions/HvdcOperatorActivePowerRange;");
    cache_.hoar_getOprFromCS2toCS1 = env_->GetMethodID(cache_.hvdcOperatorActivePowerRangeClass, "getOprFromCS2toCS1", "()F");
    cache_.hoar_setOprFromCS2toCS1 = env_->GetMethodID(cache_.hvdcOperatorActivePowerRangeClass, "setOprFromCS2toCS1", "(F)Lcom/powsybl/iidm/network/extensions/HvdcOperatorActivePowerRange;");
    checkJNIException(env_);

    // VoltagePerReactivePowerControl extension
    cacheClass(cache_.voltagePerReactivePowerControlClass,
        "com/powsybl/iidm/network/extensions/VoltagePerReactivePowerControl");
    cache_.vprc_getSlope = env_->GetMethodID(cache_.voltagePerReactivePowerControlClass, "getSlope", "()D");
    cache_.vprc_setSlope = env_->GetMethodID(cache_.voltagePerReactivePowerControlClass, "setSlope", "(D)Lcom/powsybl/iidm/network/extensions/VoltagePerReactivePowerControl;");
    checkJNIException(env_);

    // SlackTerminal extension
    cacheClass(cache_.slackTerminalClass,
        "com/powsybl/iidm/network/extensions/SlackTerminal");
    cache_.st_getTerminal = env_->GetMethodID(cache_.slackTerminalClass, "getTerminal",
        "()Lcom/powsybl/iidm/network/Terminal;");
    checkJNIException(env_);

    // Battery
    cacheClass(cache_.batteryClass, "com/powsybl/iidm/network/Battery");
    cache_.battery_getTargetP = env_->GetMethodID(cache_.batteryClass, "getTargetP", "()D");
    cache_.battery_setTargetP = env_->GetMethodID(cache_.batteryClass, "setTargetP", "(D)Lcom/powsybl/iidm/network/Battery;");
    cache_.battery_getTargetQ = env_->GetMethodID(cache_.batteryClass, "getTargetQ", "()D");
    cache_.battery_setTargetQ = env_->GetMethodID(cache_.batteryClass, "setTargetQ", "(D)Lcom/powsybl/iidm/network/Battery;");
    cache_.battery_getMinP    = env_->GetMethodID(cache_.batteryClass, "getMinP", "()D");
    cache_.battery_getMaxP    = env_->GetMethodID(cache_.batteryClass, "getMaxP", "()D");
    cache_.network_getBatteries = env_->GetMethodID(cache_.networkClass, "getBatteries", "()Ljava/lang/Iterable;");
    cache_.network_getBattery   = env_->GetMethodID(cache_.networkClass, "getBattery",
        "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Battery;");
    checkJNIException(env_);

    // Reactive limits
    cacheClass(cache_.minMaxRLClass, "com/powsybl/iidm/network/MinMaxReactiveLimits");
    cacheClass(cache_.rcCurveClass,  "com/powsybl/iidm/network/ReactiveCapabilityCurve");
    cacheClass(cache_.rcPointClass,  "com/powsybl/iidm/network/ReactiveCapabilityCurve$Point");
    cache_.gen_getReactiveLimits = env_->GetMethodID(cache_.generatorClass,
        "getReactiveLimits", "()Lcom/powsybl/iidm/network/ReactiveLimits;");
    cache_.minMaxRL_getMinQ = env_->GetMethodID(cache_.minMaxRLClass, "getMinQ", "()D");
    cache_.minMaxRL_getMaxQ = env_->GetMethodID(cache_.minMaxRLClass, "getMaxQ", "()D");
    cache_.rcCurve_getPoints = env_->GetMethodID(cache_.rcCurveClass, "getPoints", "()Ljava/util/Collection;");
    cache_.rcPoint_getP    = env_->GetMethodID(cache_.rcPointClass, "getP",    "()D");
    cache_.rcPoint_getMinQ = env_->GetMethodID(cache_.rcPointClass, "getMinQ", "()D");
    cache_.rcPoint_getMaxQ = env_->GetMethodID(cache_.rcPointClass, "getMaxQ", "()D");
    checkJNIException(env_);

    // TapChanger steps
    cacheClass(cache_.rtcStepClass, "com/powsybl/iidm/network/RatioTapChangerStep");
    cacheClass(cache_.ptcStepClass, "com/powsybl/iidm/network/PhaseTapChangerStep");
    cache_.tc_getAllSteps  = env_->GetMethodID(cache_.tapChangerClass, "getAllSteps", "()Ljava/util/SortedMap;");
    cache_.rtcStep_getRho = env_->GetMethodID(cache_.rtcStepClass, "getRho", "()D");
    cache_.rtcStep_getR   = env_->GetMethodID(cache_.rtcStepClass, "getR",   "()D");
    cache_.rtcStep_getX   = env_->GetMethodID(cache_.rtcStepClass, "getX",   "()D");
    cache_.rtcStep_getG   = env_->GetMethodID(cache_.rtcStepClass, "getG",   "()D");
    cache_.rtcStep_getB   = env_->GetMethodID(cache_.rtcStepClass, "getB",   "()D");
    cache_.ptcStep_getAlpha = env_->GetMethodID(cache_.ptcStepClass, "getAlpha", "()D");
    cache_.ptcStep_getRho   = env_->GetMethodID(cache_.ptcStepClass, "getRho",   "()D");
    cache_.ptcStep_getR     = env_->GetMethodID(cache_.ptcStepClass, "getR",     "()D");
    cache_.ptcStep_getX     = env_->GetMethodID(cache_.ptcStepClass, "getX",     "()D");
    cache_.ptcStep_getG     = env_->GetMethodID(cache_.ptcStepClass, "getG",     "()D");
    cache_.ptcStep_getB     = env_->GetMethodID(cache_.ptcStepClass, "getB",     "()D");
    checkJNIException(env_);

    // ThreeWindingsTransformer Leg
    cacheClass(cache_.threeWTLegClass, "com/powsybl/iidm/network/ThreeWindingsTransformer$Leg");
    {
        jclass twtClass = env_->FindClass("com/powsybl/iidm/network/ThreeWindingsTransformer");
        cache_.threeWT_getLeg1 = env_->GetMethodID(twtClass, "getLeg1",
            "()Lcom/powsybl/iidm/network/ThreeWindingsTransformer$Leg;");
        cache_.threeWT_getLeg2 = env_->GetMethodID(twtClass, "getLeg2",
            "()Lcom/powsybl/iidm/network/ThreeWindingsTransformer$Leg;");
        cache_.threeWT_getLeg3 = env_->GetMethodID(twtClass, "getLeg3",
            "()Lcom/powsybl/iidm/network/ThreeWindingsTransformer$Leg;");
        cache_.network_getThreeWTById = env_->GetMethodID(cache_.networkClass,
            "getThreeWindingsTransformer",
            "(Ljava/lang/String;)Lcom/powsybl/iidm/network/ThreeWindingsTransformer;");
        cache_.network_getTwoWTById = env_->GetMethodID(cache_.networkClass,
            "getTwoWindingsTransformer",
            "(Ljava/lang/String;)Lcom/powsybl/iidm/network/TwoWindingsTransformer;");
        env_->DeleteLocalRef(twtClass);
    }
    cache_.leg_getR    = env_->GetMethodID(cache_.threeWTLegClass, "getR",    "()D");
    cache_.leg_getX    = env_->GetMethodID(cache_.threeWTLegClass, "getX",    "()D");
    cache_.leg_getG    = env_->GetMethodID(cache_.threeWTLegClass, "getG",    "()D");
    cache_.leg_getB    = env_->GetMethodID(cache_.threeWTLegClass, "getB",    "()D");
    cache_.leg_getRatedU = env_->GetMethodID(cache_.threeWTLegClass, "getRatedU", "()D");
    cache_.leg_getRatedS = env_->GetMethodID(cache_.threeWTLegClass, "getRatedS", "()D");
    cache_.leg_getRatioTapChanger = env_->GetMethodID(cache_.threeWTLegClass,
        "getRatioTapChanger", "()Lcom/powsybl/iidm/network/RatioTapChanger;");
    cache_.leg_getPhaseTapChanger = env_->GetMethodID(cache_.threeWTLegClass,
        "getPhaseTapChanger", "()Lcom/powsybl/iidm/network/PhaseTapChanger;");
    checkJNIException(env_);

    // ShuntCompensator non-linear model
    cacheClass(cache_.shuntNLModelClass,
        "com/powsybl/iidm/network/ShuntCompensator$NonLinearModel");
    cacheClass(cache_.shuntSectionClass,
        "com/powsybl/iidm/network/ShuntCompensator$Section");
    {
        jclass shuntClass = env_->FindClass("com/powsybl/iidm/network/ShuntCompensator");
        cache_.shunt_getModelType = env_->GetMethodID(shuntClass, "getModelType",
            "()Lcom/powsybl/iidm/network/ShuntCompensatorModelType;");
        cache_.shunt_getNonLinearModel = env_->GetMethodID(shuntClass, "getNonLinearModel",
            "()Lcom/powsybl/iidm/network/ShuntCompensator$NonLinearModel;");
        env_->DeleteLocalRef(shuntClass);
    }
    cache_.shuntNLModel_getAllSections = env_->GetMethodID(cache_.shuntNLModelClass,
        "getAllSections", "()Ljava/util/List;");
    cache_.shuntSection_getB = env_->GetMethodID(cache_.shuntSectionClass, "getB", "()D");
    cache_.shuntSection_getG = env_->GetMethodID(cache_.shuntSectionClass, "getG", "()D");
    checkJNIException(env_);

    // VoltageLevel BusBreakerView.getBuses()
    cache_.bbView_getBuses = env_->GetMethodID(cache_.vlBusBreakerViewClass,
        "getBuses", "()Ljava/lang/Iterable;");
    checkJNIException(env_);

    // Terminal BusBreakerView / NodeBreakerView
    cacheClass(cache_.termBBViewClass, "com/powsybl/iidm/network/Terminal$BusBreakerView");
    cacheClass(cache_.termNBViewClass, "com/powsybl/iidm/network/Terminal$NodeBreakerView");
    cache_.terminal_getBusBreakerView  = env_->GetMethodID(cache_.terminalClass,
        "getBusBreakerView",  "()Lcom/powsybl/iidm/network/Terminal$BusBreakerView;");
    cache_.terminal_getNodeBreakerView = env_->GetMethodID(cache_.terminalClass,
        "getNodeBreakerView", "()Lcom/powsybl/iidm/network/Terminal$NodeBreakerView;");
    cache_.termBBView_getConnectableBus = env_->GetMethodID(cache_.termBBViewClass,
        "getConnectableBus", "()Lcom/powsybl/iidm/network/Bus;");
    cache_.termNBView_getNode = env_->GetMethodID(cache_.termNBViewClass, "getNode", "()I");
    checkJNIException(env_);

    // VSC reactive limits (vscClass is already in the cache struct at slot vscClass)
    {
        jclass vscCls = env_->FindClass("com/powsybl/iidm/network/VscConverterStation");
        cache_.vsc_getReactiveLimits = env_->GetMethodID(vscCls,
            "getReactiveLimits", "()Lcom/powsybl/iidm/network/ReactiveLimits;");
        env_->DeleteLocalRef(vscCls);
    }
    checkJNIException(env_);

    // Retrieve the network object via IidmBridgeRegistry
    jstring jId = env_->NewStringUTF(networkId_.c_str());
    jobject netObj = env_->CallStaticObjectMethod(cache_.iidmRegistryClass, cache_.registry_get, jId);
    env_->DeleteLocalRef(jId);
    checkJNIException(env_);
    if (!netObj) {
        throw NetworkNotFoundException("No network registered with id: " + networkId_);
    }
    networkRef_ = env_->NewGlobalRef(netObj);
    env_->DeleteLocalRef(netObj);
}

void JNIBackend::loadNetwork(const std::string& /*filePath*/) {
    // In JNI mode the network is already in the JVM; loadNetwork is a no-op.
    // If needed, the Java side should have loaded it before C++ attach.
}

void JNIBackend::saveNetwork(const std::string& filePath) {
    jstring jId   = env_->NewStringUTF(networkId_.c_str());
    jstring jPath = env_->NewStringUTF(filePath.c_str());
    env_->CallStaticVoidMethod(cache_.iidmRegistryClass, cache_.registry_save, jId, jPath);
    env_->DeleteLocalRef(jId);
    env_->DeleteLocalRef(jPath);
    checkJNIException(env_);
}

void JNIBackend::close() {
    if (networkRef_ && env_) {
        env_->DeleteGlobalRef(networkRef_);
        networkRef_ = nullptr;
    }
    if (jvm_) {
        jvm_->DetachCurrentThread();
        jvm_ = nullptr;
        env_ = nullptr;
    }
}

// ── Handle helpers ────────────────────────────────────────────────────────────

ObjectHandle JNIBackend::makeHandle(jobject obj) const {
    if (!obj) return INVALID_HANDLE;
    jobject globalRef = env_->NewGlobalRef(obj);
    return reinterpret_cast<ObjectHandle>(globalRef);
}

jobject JNIBackend::toObject(ObjectHandle h) const {
    if (h == INVALID_HANDLE) return nullptr;
    return reinterpret_cast<jobject>(h);
}

std::vector<ObjectHandle> JNIBackend::collectionToHandles(jobject collection) const {
    if (!collection) return {};

    jclass iterableClass = env_->FindClass("java/lang/Iterable");
    jmethodID iteratorMethod = env_->GetMethodID(iterableClass, "iterator", "()Ljava/util/Iterator;");
    jobject iterator = env_->CallObjectMethod(collection, iteratorMethod);
    env_->DeleteLocalRef(iterableClass);

    jclass iteratorClass = env_->FindClass("java/util/Iterator");
    jmethodID hasNext = env_->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID next    = env_->GetMethodID(iteratorClass, "next",    "()Ljava/lang/Object;");

    std::vector<ObjectHandle> result;
    while (env_->CallBooleanMethod(iterator, hasNext)) {
        jobject item = env_->CallObjectMethod(iterator, next);
        result.push_back(makeHandle(item));
        env_->DeleteLocalRef(item);
    }
    env_->DeleteLocalRef(iterator);
    env_->DeleteLocalRef(iteratorClass);
    return result;
}

std::vector<ObjectHandle> JNIBackend::sortedMapValuesToHandles(jobject sortedMap) const {
    if (!sortedMap) return {};
    jclass mapClass = env_->FindClass("java/util/Map");
    jmethodID values = env_->GetMethodID(mapClass, "values", "()Ljava/util/Collection;");
    env_->DeleteLocalRef(mapClass);
    jobject col = env_->CallObjectMethod(sortedMap, values);
    auto result = collectionToHandles(col);
    if (col) env_->DeleteLocalRef(col);
    return result;
}

jobject JNIBackend::fetchApcExtension(jobject gen) const {
    jstring name = env_->NewStringUTF("activePowerControl");
    jobject ext  = env_->CallObjectMethod(gen, cache_.generator_getExtensionByName, name);
    env_->DeleteLocalRef(name);
    checkJNIException(env_);
    return ext; // local ref; caller must DeleteLocalRef if non-null
}

jobject JNIBackend::fetchExtension(jobject obj, const char* extensionName) const {
    jstring name = env_->NewStringUTF(extensionName);
    jobject ext  = env_->CallObjectMethod(obj, cache_.extendable_getExtensionByName, name);
    env_->DeleteLocalRef(name);
    checkJNIException(env_);
    return ext; // local ref; caller must DeleteLocalRef if non-null
}

jobject JNIBackend::fetchCrcExtension(jobject gen) const {
    return fetchExtension(gen, "coordinatedReactiveControl");
}

jobject JNIBackend::fetchHadapcExtension(jobject hvdc) const {
    return fetchExtension(hvdc, "hvdcAngleDroopActivePowerControl");
}

jobject JNIBackend::fetchHoarExtension(jobject hvdc) const {
    return fetchExtension(hvdc, "hvdcOperatorActivePowerRange");
}

jobject JNIBackend::fetchVprcExtension(jobject svc) const {
    return fetchExtension(svc, "voltagePerReactivePowerControl");
}

jobject JNIBackend::fetchStExtension(jobject vl) const {
    return fetchExtension(vl, "slackTerminal");
}

// ── BackendProvider implementation ───────────────────────────────────────────

ObjectHandle JNIBackend::getNetworkHandle() const {
    return reinterpret_cast<ObjectHandle>(networkRef_);
}

double JNIBackend::getDouble(ObjectHandle h, int property) const {
    jobject obj = toObject(h);
    double result = 0.0;
    switch (property) {
        case prop::GEN_TARGET_P:  result = env_->CallDoubleMethod(obj, cache_.generator_getTargetP); break;
        case prop::GEN_TARGET_Q:  result = env_->CallDoubleMethod(obj, cache_.generator_getTargetQ); break;
        case prop::GEN_TARGET_V:  result = env_->CallDoubleMethod(obj, cache_.generator_getTargetV); break;
        case prop::GEN_MIN_P:     result = env_->CallDoubleMethod(obj, cache_.generator_getMinP);    break;
        case prop::GEN_MAX_P:     result = env_->CallDoubleMethod(obj, cache_.generator_getMaxP);    break;
        case prop::LOAD_P0:       result = env_->CallDoubleMethod(obj, cache_.load_getP0);           break;
        case prop::LOAD_Q0:       result = env_->CallDoubleMethod(obj, cache_.load_getQ0);           break;
        case prop::TERMINAL_P:    result = env_->CallDoubleMethod(obj, cache_.terminal_getP);        break;
        case prop::TERMINAL_Q:    result = env_->CallDoubleMethod(obj, cache_.terminal_getQ);        break;
        case prop::BUS_V:         result = env_->CallDoubleMethod(obj, cache_.bus_getV);             break;
        case prop::BUS_ANGLE:     result = env_->CallDoubleMethod(obj, cache_.bus_getAngle);         break;
        case prop::VL_NOMINAL_V:  result = env_->CallDoubleMethod(obj, cache_.vl_getNominalV);       break;
        case prop::VL_LOW_VOLTAGE_LIMIT:  result = env_->CallDoubleMethod(obj, cache_.vl_getLowVoltageLimit);  break;
        case prop::VL_HIGH_VOLTAGE_LIMIT: result = env_->CallDoubleMethod(obj, cache_.vl_getHighVoltageLimit); break;
        case prop::LINE_R: result = env_->CallDoubleMethod(obj, cache_.line_getR); break;
        case prop::LINE_X: result = env_->CallDoubleMethod(obj, cache_.line_getX); break;
        case prop::LINE_G1: result = env_->CallDoubleMethod(obj, cache_.line_getG1); break;
        case prop::LINE_B1: result = env_->CallDoubleMethod(obj, cache_.line_getB1); break;
        case prop::LINE_G2: result = env_->CallDoubleMethod(obj, cache_.line_getG2); break;
        case prop::LINE_B2: result = env_->CallDoubleMethod(obj, cache_.line_getB2); break;
        case prop::TWO_WT_RTC_TARGET_V: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present");
            result = env_->CallDoubleMethod(rtc, cache_.rtc_getTargetV);
            env_->DeleteLocalRef(rtc);
            break;
        }
        case prop::TWO_WT_PTC_REG_VALUE: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            result = env_->CallDoubleMethod(ptc, cache_.ptc_getRegulationValue);
            env_->DeleteLocalRef(ptc);
            break;
        }
        case prop::BBS_V:     result = env_->CallDoubleMethod(obj, cache_.bbs_getV);     break;
        case prop::BBS_ANGLE: result = env_->CallDoubleMethod(obj, cache_.bbs_getAngle); break;
        case prop::EXT_APC_DROOP: {
            jobject apc = fetchApcExtension(obj);
            if (!apc) throw PropertyNotFoundException("ActivePowerControl extension not present");
            result = env_->CallDoubleMethod(apc, cache_.apc_getDroop);
            env_->DeleteLocalRef(apc);
            break;
        }
        case prop::EXT_CRC_Q_PERCENT: {
            jobject crc = fetchCrcExtension(obj);
            if (!crc) throw PropertyNotFoundException("CoordinatedReactiveControl extension not present");
            result = env_->CallDoubleMethod(crc, cache_.crc_getQPercent);
            env_->DeleteLocalRef(crc);
            break;
        }
        case prop::EXT_HADAPC_DROOP: {
            jobject ext = fetchHadapcExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcAngleDroopActivePowerControl extension not present");
            result = static_cast<double>(env_->CallFloatMethod(ext, cache_.hadapc_getDroop));
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_HADAPC_P0: {
            jobject ext = fetchHadapcExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcAngleDroopActivePowerControl extension not present");
            result = static_cast<double>(env_->CallFloatMethod(ext, cache_.hadapc_getP0));
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_HOAR_OPR_CS1_TO_CS2: {
            jobject ext = fetchHoarExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcOperatorActivePowerRange extension not present");
            result = static_cast<double>(env_->CallFloatMethod(ext, cache_.hoar_getOprFromCS1toCS2));
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_HOAR_OPR_CS2_TO_CS1: {
            jobject ext = fetchHoarExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcOperatorActivePowerRange extension not present");
            result = static_cast<double>(env_->CallFloatMethod(ext, cache_.hoar_getOprFromCS2toCS1));
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_VPRC_SLOPE: {
            jobject ext = fetchVprcExtension(obj);
            if (!ext) throw PropertyNotFoundException("VoltagePerReactivePowerControl extension not present");
            result = env_->CallDoubleMethod(ext, cache_.vprc_getSlope);
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::BAT_TARGET_P: result = env_->CallDoubleMethod(obj, cache_.battery_getTargetP); break;
        case prop::BAT_TARGET_Q: result = env_->CallDoubleMethod(obj, cache_.battery_getTargetQ); break;
        case prop::BAT_MIN_P:    result = env_->CallDoubleMethod(obj, cache_.battery_getMinP);    break;
        case prop::BAT_MAX_P:    result = env_->CallDoubleMethod(obj, cache_.battery_getMaxP);    break;
        case prop::GEN_MIN_Q: {
            jobject rl = env_->CallObjectMethod(obj, cache_.gen_getReactiveLimits);
            result = env_->CallDoubleMethod(rl, cache_.minMaxRL_getMinQ);
            env_->DeleteLocalRef(rl); break;
        }
        case prop::GEN_MAX_Q: {
            jobject rl = env_->CallObjectMethod(obj, cache_.gen_getReactiveLimits);
            result = env_->CallDoubleMethod(rl, cache_.minMaxRL_getMaxQ);
            env_->DeleteLocalRef(rl); break;
        }
        case prop::VSC_MIN_Q: {
            jobject rl = env_->CallObjectMethod(obj, cache_.vsc_getReactiveLimits);
            result = env_->CallDoubleMethod(rl, cache_.minMaxRL_getMinQ);
            env_->DeleteLocalRef(rl); break;
        }
        case prop::VSC_MAX_Q: {
            jobject rl = env_->CallObjectMethod(obj, cache_.vsc_getReactiveLimits);
            result = env_->CallDoubleMethod(rl, cache_.minMaxRL_getMaxQ);
            env_->DeleteLocalRef(rl); break;
        }
        case prop::POINT_P:     result = env_->CallDoubleMethod(obj, cache_.rcPoint_getP);    break;
        case prop::POINT_MIN_Q: result = env_->CallDoubleMethod(obj, cache_.rcPoint_getMinQ); break;
        case prop::POINT_MAX_Q: result = env_->CallDoubleMethod(obj, cache_.rcPoint_getMaxQ); break;
        case prop::RTC_STEP_RHO: result = env_->CallDoubleMethod(obj, cache_.rtcStep_getRho); break;
        case prop::RTC_STEP_R:   result = env_->CallDoubleMethod(obj, cache_.rtcStep_getR);   break;
        case prop::RTC_STEP_X:   result = env_->CallDoubleMethod(obj, cache_.rtcStep_getX);   break;
        case prop::RTC_STEP_G:   result = env_->CallDoubleMethod(obj, cache_.rtcStep_getG);   break;
        case prop::RTC_STEP_B:   result = env_->CallDoubleMethod(obj, cache_.rtcStep_getB);   break;
        case prop::PTC_STEP_ALPHA: result = env_->CallDoubleMethod(obj, cache_.ptcStep_getAlpha); break;
        case prop::PTC_STEP_RHO:   result = env_->CallDoubleMethod(obj, cache_.ptcStep_getRho);   break;
        case prop::PTC_STEP_R:     result = env_->CallDoubleMethod(obj, cache_.ptcStep_getR);     break;
        case prop::PTC_STEP_X:     result = env_->CallDoubleMethod(obj, cache_.ptcStep_getX);     break;
        case prop::PTC_STEP_G:     result = env_->CallDoubleMethod(obj, cache_.ptcStep_getG);     break;
        case prop::PTC_STEP_B:     result = env_->CallDoubleMethod(obj, cache_.ptcStep_getB);     break;
        case prop::SHUNT_SECTION_B: result = env_->CallDoubleMethod(obj, cache_.shuntSection_getB); break;
        case prop::SHUNT_SECTION_G: result = env_->CallDoubleMethod(obj, cache_.shuntSection_getG); break;
        // ThreeWT leg doubles — helper lambda to avoid code duplication
        default: {
            // Handle ThreeWT leg property codes (820-878)
            auto twtLegDouble = [&](int legBase) -> double {
                int off = property - legBase;
                jmethodID getLeg = (legBase == prop::THREE_WT_LEG1_BASE) ? cache_.threeWT_getLeg1
                                 : (legBase == prop::THREE_WT_LEG2_BASE) ? cache_.threeWT_getLeg2
                                 : cache_.threeWT_getLeg3;
                jobject leg = env_->CallObjectMethod(obj, getLeg);
                double val = std::numeric_limits<double>::quiet_NaN();
                switch (off) {
                    case prop::THREE_WT_LEG_R_OFF:      val = env_->CallDoubleMethod(leg, cache_.leg_getR);      break;
                    case prop::THREE_WT_LEG_X_OFF:      val = env_->CallDoubleMethod(leg, cache_.leg_getX);      break;
                    case prop::THREE_WT_LEG_G_OFF:      val = env_->CallDoubleMethod(leg, cache_.leg_getG);      break;
                    case prop::THREE_WT_LEG_B_OFF:      val = env_->CallDoubleMethod(leg, cache_.leg_getB);      break;
                    case prop::THREE_WT_LEG_RATED_U_OFF: val = env_->CallDoubleMethod(leg, cache_.leg_getRatedU); break;
                    case prop::THREE_WT_LEG_RATED_S_OFF: val = env_->CallDoubleMethod(leg, cache_.leg_getRatedS); break;
                    case prop::THREE_WT_LEG_RTC_TARGET_V_OFF: {
                        jobject rtc = env_->CallObjectMethod(leg, cache_.leg_getRatioTapChanger);
                        val = env_->CallDoubleMethod(rtc, cache_.rtc_getTargetV);
                        env_->DeleteLocalRef(rtc); break;
                    }
                    case prop::THREE_WT_LEG_PTC_REG_VALUE_OFF: {
                        jobject ptc = env_->CallObjectMethod(leg, cache_.leg_getPhaseTapChanger);
                        val = env_->CallDoubleMethod(ptc, cache_.ptc_getRegulationValue);
                        env_->DeleteLocalRef(ptc); break;
                    }
                    default: break;
                }
                env_->DeleteLocalRef(leg);
                return val;
            };
            if (property >= prop::THREE_WT_LEG1_BASE && property < prop::THREE_WT_LEG1_BASE + 20)
                result = twtLegDouble(prop::THREE_WT_LEG1_BASE);
            else if (property >= prop::THREE_WT_LEG2_BASE && property < prop::THREE_WT_LEG2_BASE + 20)
                result = twtLegDouble(prop::THREE_WT_LEG2_BASE);
            else if (property >= prop::THREE_WT_LEG3_BASE && property < prop::THREE_WT_LEG3_BASE + 20)
                result = twtLegDouble(prop::THREE_WT_LEG3_BASE);
            else
                throw PropertyNotFoundException("Unknown double property: " + std::to_string(property));
        }
    }
    checkJNIException(env_);
    return result;
}

void JNIBackend::setDouble(ObjectHandle h, int property, double value) {
    jobject obj = toObject(h);
    switch (property) {
        case prop::GEN_TARGET_P: env_->CallObjectMethod(obj, cache_.generator_setTargetP, value); break;
        case prop::GEN_TARGET_Q: env_->CallObjectMethod(obj, cache_.generator_setTargetQ, value); break;
        case prop::GEN_TARGET_V: env_->CallObjectMethod(obj, cache_.generator_setTargetV, value); break;
        case prop::LOAD_P0:      env_->CallObjectMethod(obj, cache_.load_setP0, value); break;
        case prop::LOAD_Q0:      env_->CallObjectMethod(obj, cache_.load_setQ0, value); break;
        case prop::TERMINAL_P:   env_->CallObjectMethod(obj, cache_.terminal_setP, value); break;
        case prop::TERMINAL_Q:   env_->CallObjectMethod(obj, cache_.terminal_setQ, value); break;
        case prop::BUS_V:        env_->CallObjectMethod(obj, cache_.bus_setV,     value); break;
        case prop::BUS_ANGLE:    env_->CallObjectMethod(obj, cache_.bus_setAngle, value); break;
        case prop::TWO_WT_RTC_TARGET_V: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present");
            env_->CallObjectMethod(rtc, cache_.rtc_setTargetV, value);
            env_->DeleteLocalRef(rtc);
            break;
        }
        case prop::TWO_WT_PTC_REG_VALUE: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            env_->CallObjectMethod(ptc, cache_.ptc_setRegulationValue, value);
            env_->DeleteLocalRef(ptc);
            break;
        }
        case prop::EXT_APC_DROOP: {
            jobject apc = fetchApcExtension(obj);
            if (!apc) throw PropertyNotFoundException("ActivePowerControl extension not present");
            env_->CallVoidMethod(apc, cache_.apc_setDroop, value);
            env_->DeleteLocalRef(apc);
            break;
        }
        case prop::EXT_CRC_Q_PERCENT: {
            jobject crc = fetchCrcExtension(obj);
            if (!crc) throw PropertyNotFoundException("CoordinatedReactiveControl extension not present");
            env_->CallObjectMethod(crc, cache_.crc_setQPercent, value);
            env_->DeleteLocalRef(crc);
            break;
        }
        case prop::EXT_HADAPC_DROOP: {
            jobject ext = fetchHadapcExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcAngleDroopActivePowerControl extension not present");
            env_->CallObjectMethod(ext, cache_.hadapc_setDroop, static_cast<jfloat>(value));
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_HADAPC_P0: {
            jobject ext = fetchHadapcExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcAngleDroopActivePowerControl extension not present");
            env_->CallObjectMethod(ext, cache_.hadapc_setP0, static_cast<jfloat>(value));
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_HOAR_OPR_CS1_TO_CS2: {
            jobject ext = fetchHoarExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcOperatorActivePowerRange extension not present");
            env_->CallObjectMethod(ext, cache_.hoar_setOprFromCS1toCS2, static_cast<jfloat>(value));
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_HOAR_OPR_CS2_TO_CS1: {
            jobject ext = fetchHoarExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcOperatorActivePowerRange extension not present");
            env_->CallObjectMethod(ext, cache_.hoar_setOprFromCS2toCS1, static_cast<jfloat>(value));
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_VPRC_SLOPE: {
            jobject ext = fetchVprcExtension(obj);
            if (!ext) throw PropertyNotFoundException("VoltagePerReactivePowerControl extension not present");
            env_->CallObjectMethod(ext, cache_.vprc_setSlope, value);
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::BAT_TARGET_P: env_->CallObjectMethod(obj, cache_.battery_setTargetP, value); break;
        case prop::BAT_TARGET_Q: env_->CallObjectMethod(obj, cache_.battery_setTargetQ, value); break;
        default:
            throw PropertyNotFoundException("Unknown double property for set: " + std::to_string(property));
    }
    checkJNIException(env_);
}

int JNIBackend::getInt(ObjectHandle h, int property) const {
    jobject obj = toObject(h);
    switch (property) {
        case prop::GEN_ENERGY_SOURCE: {
            jobject es = env_->CallObjectMethod(obj, cache_.generator_getEnergySource);
            int ord = env_->CallIntMethod(es, cache_.enum_ordinal);
            env_->DeleteLocalRef(es);
            checkJNIException(env_);
            return ord;
        }
        case prop::LOAD_TYPE: {
            jobject lt = env_->CallObjectMethod(obj, cache_.load_getLoadType);
            int ord = env_->CallIntMethod(lt, cache_.enum_ordinal);
            env_->DeleteLocalRef(lt);
            checkJNIException(env_);
            return ord;
        }
        case prop::VL_TOPOLOGY_KIND: {
            jobject tk = env_->CallObjectMethod(obj, cache_.vl_getTopologyKind);
            int ord = env_->CallIntMethod(tk, cache_.enum_ordinal);
            env_->DeleteLocalRef(tk);
            checkJNIException(env_);
            return ord;
        }
        case prop::SW_KIND: {
            jobject kind = env_->CallObjectMethod(obj, cache_.switch_getKind);
            int ord = env_->CallIntMethod(kind, cache_.enum_ordinal);
            env_->DeleteLocalRef(kind);
            checkJNIException(env_);
            return ord;
        }
        case prop::TWO_WT_RTC_TAP_POSITION: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present");
            int v = env_->CallIntMethod(rtc, cache_.tc_getTapPosition);
            env_->DeleteLocalRef(rtc);
            checkJNIException(env_);
            return v;
        }
        case prop::TWO_WT_RTC_LOW_TAP: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present");
            int v = env_->CallIntMethod(rtc, cache_.tc_getLowTapPosition);
            env_->DeleteLocalRef(rtc);
            checkJNIException(env_);
            return v;
        }
        case prop::TWO_WT_RTC_HIGH_TAP: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present");
            int v = env_->CallIntMethod(rtc, cache_.tc_getHighTapPosition);
            env_->DeleteLocalRef(rtc);
            checkJNIException(env_);
            return v;
        }
        case prop::TWO_WT_PTC_TAP_POSITION: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            int v = env_->CallIntMethod(ptc, cache_.tc_getTapPosition);
            env_->DeleteLocalRef(ptc);
            checkJNIException(env_);
            return v;
        }
        case prop::TWO_WT_PTC_LOW_TAP: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            int v = env_->CallIntMethod(ptc, cache_.tc_getLowTapPosition);
            env_->DeleteLocalRef(ptc);
            checkJNIException(env_);
            return v;
        }
        case prop::TWO_WT_PTC_HIGH_TAP: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            int v = env_->CallIntMethod(ptc, cache_.tc_getHighTapPosition);
            env_->DeleteLocalRef(ptc);
            checkJNIException(env_);
            return v;
        }
        case prop::TWO_WT_PTC_REG_MODE: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            jobject mode = env_->CallObjectMethod(ptc, cache_.ptc_getRegulationMode);
            int ord = env_->CallIntMethod(mode, cache_.enum_ordinal);
            env_->DeleteLocalRef(mode);
            env_->DeleteLocalRef(ptc);
            checkJNIException(env_);
            return ord;
        }
        case prop::GEN_REACTIVE_LIMITS_KIND: {
            jobject rl = env_->CallObjectMethod(obj, cache_.gen_getReactiveLimits);
            int kind = 0;
            if (rl) {
                if (env_->IsInstanceOf(rl, cache_.minMaxRLClass))  kind = 1;
                else if (env_->IsInstanceOf(rl, cache_.rcCurveClass)) kind = 2;
                env_->DeleteLocalRef(rl);
            }
            checkJNIException(env_);
            return kind;
        }
        case prop::VSC_REACTIVE_LIMITS_KIND: {
            jobject rl = env_->CallObjectMethod(obj, cache_.vsc_getReactiveLimits);
            int kind = 0;
            if (rl) {
                if (env_->IsInstanceOf(rl, cache_.minMaxRLClass))  kind = 1;
                else if (env_->IsInstanceOf(rl, cache_.rcCurveClass)) kind = 2;
                env_->DeleteLocalRef(rl);
            }
            checkJNIException(env_);
            return kind;
        }
        case prop::SHUNT_MODEL_KIND: {
            jclass shuntModelTypeClass = env_->FindClass("com/powsybl/iidm/network/ShuntCompensatorModelType");
            jobject modelType = env_->CallObjectMethod(obj, cache_.shunt_getModelType);
            int ord = env_->CallIntMethod(modelType, cache_.enum_ordinal);
            env_->DeleteLocalRef(modelType);
            env_->DeleteLocalRef(shuntModelTypeClass);
            checkJNIException(env_);
            return ord;
        }
        case prop::TERMINAL_NODE: {
            jobject nbv = env_->CallObjectMethod(obj, cache_.terminal_getNodeBreakerView);
            int node = env_->CallIntMethod(nbv, cache_.termNBView_getNode);
            env_->DeleteLocalRef(nbv);
            checkJNIException(env_);
            return node;
        }
        default: {
            // ThreeWT leg int properties (820-878)
            auto twtLegInt = [&](int legBase) -> int {
                int off = property - legBase;
                jmethodID getLeg = (legBase == prop::THREE_WT_LEG1_BASE) ? cache_.threeWT_getLeg1
                                 : (legBase == prop::THREE_WT_LEG2_BASE) ? cache_.threeWT_getLeg2
                                 : cache_.threeWT_getLeg3;
                jobject leg = env_->CallObjectMethod(obj, getLeg);
                int val = 0;
                switch (off) {
                    case prop::THREE_WT_LEG_RTC_TAP_POS_OFF: {
                        jobject rtc = env_->CallObjectMethod(leg, cache_.leg_getRatioTapChanger);
                        val = env_->CallIntMethod(rtc, cache_.tc_getTapPosition);
                        env_->DeleteLocalRef(rtc); break;
                    }
                    case prop::THREE_WT_LEG_RTC_LOW_TAP_OFF: {
                        jobject rtc = env_->CallObjectMethod(leg, cache_.leg_getRatioTapChanger);
                        val = env_->CallIntMethod(rtc, cache_.tc_getLowTapPosition);
                        env_->DeleteLocalRef(rtc); break;
                    }
                    case prop::THREE_WT_LEG_RTC_HIGH_TAP_OFF: {
                        jobject rtc = env_->CallObjectMethod(leg, cache_.leg_getRatioTapChanger);
                        val = env_->CallIntMethod(rtc, cache_.tc_getHighTapPosition);
                        env_->DeleteLocalRef(rtc); break;
                    }
                    case prop::THREE_WT_LEG_PTC_TAP_POS_OFF: {
                        jobject ptc = env_->CallObjectMethod(leg, cache_.leg_getPhaseTapChanger);
                        val = env_->CallIntMethod(ptc, cache_.tc_getTapPosition);
                        env_->DeleteLocalRef(ptc); break;
                    }
                    case prop::THREE_WT_LEG_PTC_LOW_TAP_OFF: {
                        jobject ptc = env_->CallObjectMethod(leg, cache_.leg_getPhaseTapChanger);
                        val = env_->CallIntMethod(ptc, cache_.tc_getLowTapPosition);
                        env_->DeleteLocalRef(ptc); break;
                    }
                    case prop::THREE_WT_LEG_PTC_HIGH_TAP_OFF: {
                        jobject ptc = env_->CallObjectMethod(leg, cache_.leg_getPhaseTapChanger);
                        val = env_->CallIntMethod(ptc, cache_.tc_getHighTapPosition);
                        env_->DeleteLocalRef(ptc); break;
                    }
                    case prop::THREE_WT_LEG_PTC_REG_MODE_OFF: {
                        jobject ptc = env_->CallObjectMethod(leg, cache_.leg_getPhaseTapChanger);
                        jobject mode = env_->CallObjectMethod(ptc, cache_.ptc_getRegulationMode);
                        val = env_->CallIntMethod(mode, cache_.enum_ordinal);
                        env_->DeleteLocalRef(mode);
                        env_->DeleteLocalRef(ptc); break;
                    }
                    default: break;
                }
                env_->DeleteLocalRef(leg);
                return val;
            };
            if (property >= prop::THREE_WT_LEG1_BASE && property < prop::THREE_WT_LEG1_BASE + 20)
                return twtLegInt(prop::THREE_WT_LEG1_BASE);
            if (property >= prop::THREE_WT_LEG2_BASE && property < prop::THREE_WT_LEG2_BASE + 20)
                return twtLegInt(prop::THREE_WT_LEG2_BASE);
            if (property >= prop::THREE_WT_LEG3_BASE && property < prop::THREE_WT_LEG3_BASE + 20)
                return twtLegInt(prop::THREE_WT_LEG3_BASE);
            throw PropertyNotFoundException("Unknown int property: " + std::to_string(property));
        }
    }
}

void JNIBackend::setInt(ObjectHandle h, int property, int value) {
    jobject obj = toObject(h);
    switch (property) {
        case prop::TWO_WT_RTC_TAP_POSITION: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present");
            env_->CallObjectMethod(rtc, cache_.tc_setTapPosition, static_cast<jint>(value));
            env_->DeleteLocalRef(rtc);
            break;
        }
        case prop::TWO_WT_PTC_TAP_POSITION: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            env_->CallObjectMethod(ptc, cache_.tc_setTapPosition, static_cast<jint>(value));
            env_->DeleteLocalRef(ptc);
            break;
        }
        case prop::TWO_WT_PTC_REG_MODE: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            jobject modeArr = env_->CallStaticObjectMethod(cache_.ptcRegulationModeClass,
                env_->GetStaticMethodID(cache_.ptcRegulationModeClass, "values",
                    "()[Lcom/powsybl/iidm/network/PhaseTapChanger$RegulationMode;"));
            jobject mode = env_->GetObjectArrayElement(
                static_cast<jobjectArray>(modeArr), value);
            env_->CallObjectMethod(ptc, cache_.ptc_setRegulationMode, mode);
            env_->DeleteLocalRef(mode);
            env_->DeleteLocalRef(modeArr);
            env_->DeleteLocalRef(ptc);
            break;
        }
        default:
            throw PropertyNotFoundException("setInt not supported for property: " + std::to_string(property));
    }
    checkJNIException(env_);
}

bool JNIBackend::getBool(ObjectHandle h, int property) const {
    jobject obj = toObject(h);
    jboolean result = JNI_FALSE;
    switch (property) {
        case prop::GEN_VOLTAGE_REGULATOR_ON:
            result = env_->CallBooleanMethod(obj, cache_.generator_isVoltageRegulatorOn);
            break;
        case prop::TERMINAL_CONNECTED:
            result = env_->CallBooleanMethod(obj, cache_.terminal_isConnected);
            break;
        case prop::SW_OPEN:
            result = env_->CallBooleanMethod(obj, cache_.switch_isOpen);
            break;
        case prop::SW_RETAINED:
            result = env_->CallBooleanMethod(obj, cache_.switch_isRetained);
            break;
        case prop::TWO_WT_RTC_EXISTS: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            result = (rtc != nullptr) ? JNI_TRUE : JNI_FALSE;
            if (rtc) env_->DeleteLocalRef(rtc);
            break;
        }
        case prop::TWO_WT_RTC_REGULATING: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present");
            result = env_->CallBooleanMethod(rtc, cache_.tc_isRegulating);
            env_->DeleteLocalRef(rtc);
            break;
        }
        case prop::TWO_WT_PTC_EXISTS: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            result = (ptc != nullptr) ? JNI_TRUE : JNI_FALSE;
            if (ptc) env_->DeleteLocalRef(ptc);
            break;
        }
        case prop::TWO_WT_PTC_REGULATING: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            result = env_->CallBooleanMethod(ptc, cache_.tc_isRegulating);
            env_->DeleteLocalRef(ptc);
            break;
        }
        case prop::EXT_APC_EXISTS: {
            jobject apc = fetchApcExtension(obj);
            result = (apc != nullptr) ? JNI_TRUE : JNI_FALSE;
            if (apc) env_->DeleteLocalRef(apc);
            break;
        }
        case prop::EXT_APC_PARTICIPATE: {
            jobject apc = fetchApcExtension(obj);
            if (!apc) throw PropertyNotFoundException("ActivePowerControl extension not present");
            result = env_->CallBooleanMethod(apc, cache_.apc_isParticipate);
            env_->DeleteLocalRef(apc);
            break;
        }
        case prop::EXT_CRC_EXISTS: {
            jobject crc = fetchCrcExtension(obj);
            result = (crc != nullptr) ? JNI_TRUE : JNI_FALSE;
            if (crc) env_->DeleteLocalRef(crc);
            break;
        }
        case prop::EXT_HADAPC_EXISTS: {
            jobject ext = fetchHadapcExtension(obj);
            result = (ext != nullptr) ? JNI_TRUE : JNI_FALSE;
            if (ext) env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_HADAPC_ENABLED: {
            jobject ext = fetchHadapcExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcAngleDroopActivePowerControl extension not present");
            result = env_->CallBooleanMethod(ext, cache_.hadapc_isEnabled);
            env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_HOAR_EXISTS: {
            jobject ext = fetchHoarExtension(obj);
            result = (ext != nullptr) ? JNI_TRUE : JNI_FALSE;
            if (ext) env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_VPRC_EXISTS: {
            jobject ext = fetchVprcExtension(obj);
            result = (ext != nullptr) ? JNI_TRUE : JNI_FALSE;
            if (ext) env_->DeleteLocalRef(ext);
            break;
        }
        case prop::EXT_ST_EXISTS: {
            jobject ext = fetchStExtension(obj);
            result = (ext != nullptr) ? JNI_TRUE : JNI_FALSE;
            if (ext) env_->DeleteLocalRef(ext);
            break;
        }
        default: {
            // ThreeWT leg bool properties
            auto twtLegBool = [&](int legBase) -> jboolean {
                int off = property - legBase;
                jmethodID getLeg = (legBase == prop::THREE_WT_LEG1_BASE) ? cache_.threeWT_getLeg1
                                 : (legBase == prop::THREE_WT_LEG2_BASE) ? cache_.threeWT_getLeg2
                                 : cache_.threeWT_getLeg3;
                jobject leg = env_->CallObjectMethod(obj, getLeg);
                jboolean val = JNI_FALSE;
                switch (off) {
                    case prop::THREE_WT_LEG_RTC_EXISTS_OFF: {
                        jobject rtc = env_->CallObjectMethod(leg, cache_.leg_getRatioTapChanger);
                        val = (rtc != nullptr) ? JNI_TRUE : JNI_FALSE;
                        if (rtc) env_->DeleteLocalRef(rtc); break;
                    }
                    case prop::THREE_WT_LEG_RTC_REGULATING_OFF: {
                        jobject rtc = env_->CallObjectMethod(leg, cache_.leg_getRatioTapChanger);
                        if (rtc) { val = env_->CallBooleanMethod(rtc, cache_.tc_isRegulating); env_->DeleteLocalRef(rtc); }
                        break;
                    }
                    case prop::THREE_WT_LEG_PTC_EXISTS_OFF: {
                        jobject ptc = env_->CallObjectMethod(leg, cache_.leg_getPhaseTapChanger);
                        val = (ptc != nullptr) ? JNI_TRUE : JNI_FALSE;
                        if (ptc) env_->DeleteLocalRef(ptc); break;
                    }
                    case prop::THREE_WT_LEG_PTC_REGULATING_OFF: {
                        jobject ptc = env_->CallObjectMethod(leg, cache_.leg_getPhaseTapChanger);
                        if (ptc) { val = env_->CallBooleanMethod(ptc, cache_.tc_isRegulating); env_->DeleteLocalRef(ptc); }
                        break;
                    }
                    default: break;
                }
                env_->DeleteLocalRef(leg);
                return val;
            };
            if (property >= prop::THREE_WT_LEG1_BASE && property < prop::THREE_WT_LEG1_BASE + 20)
                result = twtLegBool(prop::THREE_WT_LEG1_BASE);
            else if (property >= prop::THREE_WT_LEG2_BASE && property < prop::THREE_WT_LEG2_BASE + 20)
                result = twtLegBool(prop::THREE_WT_LEG2_BASE);
            else if (property >= prop::THREE_WT_LEG3_BASE && property < prop::THREE_WT_LEG3_BASE + 20)
                result = twtLegBool(prop::THREE_WT_LEG3_BASE);
            else
                throw PropertyNotFoundException("Unknown bool property: " + std::to_string(property));
        }
    }
    checkJNIException(env_);
    return result == JNI_TRUE;
}

void JNIBackend::setBool(ObjectHandle h, int property, bool value) {
    jobject obj = toObject(h);
    switch (property) {
        case prop::GEN_VOLTAGE_REGULATOR_ON:
            env_->CallObjectMethod(obj, cache_.generator_setVoltageRegulatorOn,
                                   static_cast<jboolean>(value));
            break;
        case prop::TERMINAL_CONNECTED:
            if (value) env_->CallBooleanMethod(obj, cache_.terminal_connect);
            else       env_->CallBooleanMethod(obj, cache_.terminal_disconnect);
            break;
        case prop::SW_OPEN:
            env_->CallVoidMethod(obj, cache_.switch_setOpen, static_cast<jboolean>(value));
            break;
        case prop::SW_RETAINED:
            env_->CallVoidMethod(obj, cache_.switch_setRetained, static_cast<jboolean>(value));
            break;
        case prop::TWO_WT_RTC_REGULATING: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present");
            env_->CallObjectMethod(rtc, cache_.tc_setRegulating, static_cast<jboolean>(value));
            env_->DeleteLocalRef(rtc);
            break;
        }
        case prop::TWO_WT_PTC_REGULATING: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            env_->CallObjectMethod(ptc, cache_.tc_setRegulating, static_cast<jboolean>(value));
            env_->DeleteLocalRef(ptc);
            break;
        }
        case prop::EXT_APC_PARTICIPATE: {
            jobject apc = fetchApcExtension(obj);
            if (!apc) throw PropertyNotFoundException("ActivePowerControl extension not present");
            env_->CallVoidMethod(apc, cache_.apc_setParticipate, static_cast<jboolean>(value));
            env_->DeleteLocalRef(apc);
            break;
        }
        case prop::EXT_HADAPC_ENABLED: {
            jobject ext = fetchHadapcExtension(obj);
            if (!ext) throw PropertyNotFoundException("HvdcAngleDroopActivePowerControl extension not present");
            env_->CallObjectMethod(ext, cache_.hadapc_setEnabled, static_cast<jboolean>(value));
            env_->DeleteLocalRef(ext);
            break;
        }
        default:
            throw PropertyNotFoundException("Unknown bool property for set: " + std::to_string(property));
    }
    checkJNIException(env_);
}

std::string JNIBackend::getString(ObjectHandle h, int property) const {
    jobject obj = toObject(h);
    jstring jstr = nullptr;
    switch (property) {
        case prop::ID: {
            // getId is available on network and all identifiable objects
            jclass identClass = env_->FindClass("com/powsybl/iidm/network/Identifiable");
            jmethodID mid = env_->GetMethodID(identClass, "getId", "()Ljava/lang/String;");
            env_->DeleteLocalRef(identClass);
            jstr = static_cast<jstring>(env_->CallObjectMethod(obj, mid));
            break;
        }
        case prop::NAME: {
            jclass identClass = env_->FindClass("com/powsybl/iidm/network/Identifiable");
            jmethodID mid = env_->GetMethodID(identClass, "getNameOrId", "()Ljava/lang/String;");
            env_->DeleteLocalRef(identClass);
            jstr = static_cast<jstring>(env_->CallObjectMethod(obj, mid));
            break;
        }
        case prop::TERMINAL_BUS_ID: {
            jclass busViewClass = env_->FindClass("com/powsybl/iidm/network/Terminal$BusView");
            jmethodID getBusView = env_->GetMethodID(cache_.terminalClass, "getBusView",
                "()Lcom/powsybl/iidm/network/Terminal$BusView;");
            jmethodID getConnBus = env_->GetMethodID(busViewClass, "getConnectableBus",
                "()Lcom/powsybl/iidm/network/Bus;");
            jobject busView = env_->CallObjectMethod(obj, getBusView);
            if (busView) {
                jobject bus = env_->CallObjectMethod(busView, getConnBus);
                if (bus) {
                    jstr = static_cast<jstring>(env_->CallObjectMethod(bus, cache_.bus_getId));
                    env_->DeleteLocalRef(bus);
                }
                env_->DeleteLocalRef(busView);
            }
            env_->DeleteLocalRef(busViewClass);
            break;
        }
        default:
            throw PropertyNotFoundException("Unknown string property: " + std::to_string(property));
    }
    checkJNIException(env_);

    std::string result;
    if (jstr) {
        const char* chars = env_->GetStringUTFChars(jstr, nullptr);
        if (chars) {
            result = chars;
            env_->ReleaseStringUTFChars(jstr, chars);
        }
        env_->DeleteLocalRef(jstr);
    }
    return result;
}

void JNIBackend::setString(ObjectHandle h, int property, const std::string& value) {
    jobject obj = toObject(h);
    switch (property) {
        case prop::COUNTRY: {
            jstring jval   = env_->NewStringUTF(value.c_str());
            jobject country = env_->CallStaticObjectMethod(
                cache_.countryClass, cache_.country_valueOf, jval);
            env_->DeleteLocalRef(jval);
            checkJNIException(env_);
            env_->CallObjectMethod(obj, cache_.substation_setCountry, country);
            env_->DeleteLocalRef(country);
            break;
        }
        default:
            throw PropertyNotFoundException("setString not supported for property: " + std::to_string(property));
    }
    checkJNIException(env_);
}

std::optional<double> JNIBackend::getOptDouble(ObjectHandle h, int property) const {
    double val = getDouble(h, property);
    if (std::isnan(val)) return std::nullopt;
    return val;
}

std::vector<ObjectHandle> JNIBackend::getChildren(ObjectHandle h, int childType) const {
    jobject obj = toObject(h);
    jobject collection = nullptr;

    switch (childType) {
        case prop::GENERATOR:    collection = env_->CallObjectMethod(obj, cache_.network_getGenerators);    break;
        case prop::LOAD:         collection = env_->CallObjectMethod(obj, cache_.network_getLoads);         break;
        case prop::LINE:         collection = env_->CallObjectMethod(obj, cache_.network_getLines);         break;
        case prop::SUBSTATION:   collection = env_->CallObjectMethod(obj, cache_.network_getSubstations);   break;
        case prop::VOLTAGE_LEVEL: collection = env_->CallObjectMethod(obj, cache_.network_getVoltageLevels); break;
        case prop::BUSBAR_SECTION: {
            // obj is a VoltageLevel in node-breaker topology
            jobject view = env_->CallObjectMethod(obj, cache_.vl_getNodeBreakerView);
            collection   = env_->CallObjectMethod(view, cache_.nbView_getBusbarSections);
            env_->DeleteLocalRef(view);
            break;
        }
        case prop::SWITCH: {
            // obj is a VoltageLevel; choose view based on topology kind
            jobject tkObj = env_->CallObjectMethod(obj, cache_.vl_getTopologyKind);
            int tk = env_->CallIntMethod(tkObj, cache_.enum_ordinal);
            env_->DeleteLocalRef(tkObj);
            if (tk == 0) { // NODE_BREAKER
                jobject view = env_->CallObjectMethod(obj, cache_.vl_getNodeBreakerView);
                collection   = env_->CallObjectMethod(view, cache_.nbView_getSwitches);
                env_->DeleteLocalRef(view);
            } else { // BUS_BREAKER
                jobject view = env_->CallObjectMethod(obj, cache_.vl_getBusBreakerView);
                collection   = env_->CallObjectMethod(view, cache_.bbView_getSwitches);
                env_->DeleteLocalRef(view);
            }
            break;
        }
        case prop::BUS: {
            // obj is a VoltageLevel; get bus-breaker view buses
            jobject view = env_->CallObjectMethod(obj, cache_.vl_getBusBreakerView);
            collection   = env_->CallObjectMethod(view, cache_.bbView_getBuses);
            env_->DeleteLocalRef(view);
            break;
        }
        case prop::BATTERY:
            collection = env_->CallObjectMethod(obj, cache_.network_getBatteries);
            break;
        case prop::REACTIVE_CURVE_POINT: {
            // obj is a Generator; get its reactive capability curve points
            jobject rl = env_->CallObjectMethod(obj, cache_.gen_getReactiveLimits);
            if (!rl) throw PropertyNotFoundException("No reactive limits on generator");
            collection = env_->CallObjectMethod(rl, cache_.rcCurve_getPoints);
            env_->DeleteLocalRef(rl);
            break;
        }
        case prop::TWO_WT_RTC_STEP: {
            jobject rtc = env_->CallObjectMethod(obj, cache_.twt_getRatioTapChanger);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present");
            jobject map = env_->CallObjectMethod(rtc, cache_.tc_getAllSteps);
            env_->DeleteLocalRef(rtc);
            checkJNIException(env_);
            auto result = sortedMapValuesToHandles(map);
            if (map) env_->DeleteLocalRef(map);
            return result;
        }
        case prop::TWO_WT_PTC_STEP: {
            jobject ptc = env_->CallObjectMethod(obj, cache_.twt_getPhaseTapChanger);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present");
            jobject map = env_->CallObjectMethod(ptc, cache_.tc_getAllSteps);
            env_->DeleteLocalRef(ptc);
            checkJNIException(env_);
            auto result = sortedMapValuesToHandles(map);
            if (map) env_->DeleteLocalRef(map);
            return result;
        }
        case prop::THREE_WT_LEG1_RTC_STEP:
        case prop::THREE_WT_LEG2_RTC_STEP:
        case prop::THREE_WT_LEG3_RTC_STEP: {
            jmethodID getLeg = (childType == prop::THREE_WT_LEG1_RTC_STEP) ? cache_.threeWT_getLeg1
                             : (childType == prop::THREE_WT_LEG2_RTC_STEP) ? cache_.threeWT_getLeg2
                             : cache_.threeWT_getLeg3;
            jobject leg = env_->CallObjectMethod(obj, getLeg);
            jobject rtc = env_->CallObjectMethod(leg, cache_.leg_getRatioTapChanger);
            env_->DeleteLocalRef(leg);
            if (!rtc) throw PropertyNotFoundException("RatioTapChanger not present on leg");
            jobject map = env_->CallObjectMethod(rtc, cache_.tc_getAllSteps);
            env_->DeleteLocalRef(rtc);
            checkJNIException(env_);
            auto result = sortedMapValuesToHandles(map);
            if (map) env_->DeleteLocalRef(map);
            return result;
        }
        case prop::THREE_WT_LEG1_PTC_STEP:
        case prop::THREE_WT_LEG2_PTC_STEP:
        case prop::THREE_WT_LEG3_PTC_STEP: {
            jmethodID getLeg = (childType == prop::THREE_WT_LEG1_PTC_STEP) ? cache_.threeWT_getLeg1
                             : (childType == prop::THREE_WT_LEG2_PTC_STEP) ? cache_.threeWT_getLeg2
                             : cache_.threeWT_getLeg3;
            jobject leg = env_->CallObjectMethod(obj, getLeg);
            jobject ptc = env_->CallObjectMethod(leg, cache_.leg_getPhaseTapChanger);
            env_->DeleteLocalRef(leg);
            if (!ptc) throw PropertyNotFoundException("PhaseTapChanger not present on leg");
            jobject map = env_->CallObjectMethod(ptc, cache_.tc_getAllSteps);
            env_->DeleteLocalRef(ptc);
            checkJNIException(env_);
            auto result = sortedMapValuesToHandles(map);
            if (map) env_->DeleteLocalRef(map);
            return result;
        }
        case prop::SHUNT_SECTION: {
            jobject nlm = env_->CallObjectMethod(obj, cache_.shunt_getNonLinearModel);
            if (!nlm) throw PropertyNotFoundException("ShuntCompensator has no non-linear model");
            collection = env_->CallObjectMethod(nlm, cache_.shuntNLModel_getAllSections);
            env_->DeleteLocalRef(nlm);
            break;
        }
        default:
            throw PropertyNotFoundException("Unknown child type: " + std::to_string(childType));
    }
    checkJNIException(env_);
    auto handles = collectionToHandles(collection);
    if (collection) env_->DeleteLocalRef(collection);
    return handles;
}

ObjectHandle JNIBackend::getRelated(ObjectHandle h, int relation) const {
    jobject obj = toObject(h);
    jobject related = nullptr;

    switch (relation) {
        case prop::REL_TERMINAL:
            related = env_->CallObjectMethod(obj, cache_.injection_getTerminal);
            break;
        case prop::REL_TERMINAL1:
            related = env_->CallObjectMethod(obj, cache_.line_getTerminal1);
            break;
        case prop::REL_TERMINAL2:
            related = env_->CallObjectMethod(obj, cache_.line_getTerminal2);
            break;
        case prop::REL_BUS: {
            // Called on a Terminal — get its bus view's connected bus
            jclass busViewClass = env_->FindClass("com/powsybl/iidm/network/Terminal$BusView");
            jmethodID getBusView = env_->GetMethodID(cache_.terminalClass, "getBusView",
                "()Lcom/powsybl/iidm/network/Terminal$BusView;");
            jmethodID getConnBus = env_->GetMethodID(busViewClass, "getConnectableBus",
                "()Lcom/powsybl/iidm/network/Bus;");
            jobject busView = env_->CallObjectMethod(obj, getBusView);
            if (busView) {
                related = env_->CallObjectMethod(busView, getConnBus);
                env_->DeleteLocalRef(busView);
            }
            env_->DeleteLocalRef(busViewClass);
            break;
        }
        case prop::REL_SLACK_TERMINAL: {
            jobject st = fetchStExtension(obj);
            if (!st) throw PropertyNotFoundException("SlackTerminal extension not present");
            related = env_->CallObjectMethod(st, cache_.st_getTerminal);
            env_->DeleteLocalRef(st);
            break;
        }
        case prop::REL_SUBSTATION: {
            jobject opt = env_->CallObjectMethod(obj, cache_.vl_getSubstation);
            checkJNIException(env_);
            if (opt && env_->CallBooleanMethod(opt, cache_.optional_isPresent)) {
                related = env_->CallObjectMethod(opt, cache_.optional_get);
            }
            if (opt) env_->DeleteLocalRef(opt);
            break;
        }
        case prop::REL_CONNECTABLE_BUS: {
            jobject bbv = env_->CallObjectMethod(obj, cache_.terminal_getBusBreakerView);
            if (bbv) {
                related = env_->CallObjectMethod(bbv, cache_.termBBView_getConnectableBus);
                env_->DeleteLocalRef(bbv);
            }
            break;
        }
        default:
            throw PropertyNotFoundException("Unknown relation: " + std::to_string(relation));
    }
    checkJNIException(env_);
    ObjectHandle handle = makeHandle(related);
    if (related) env_->DeleteLocalRef(related);
    return handle;
}

ObjectHandle JNIBackend::findById(int objectType, const std::string& id) const {
    jstring jId = env_->NewStringUTF(id.c_str());
    jobject result = nullptr;

    switch (objectType) {
        case prop::GENERATOR:
            result = env_->CallObjectMethod(networkRef_, cache_.network_getGenerator, jId);
            break;
        case prop::LOAD:
            result = env_->CallObjectMethod(networkRef_, cache_.network_getLoad, jId);
            break;
        case prop::LINE:
            result = env_->CallObjectMethod(networkRef_, cache_.network_getLine, jId);
            break;
        case prop::SWITCH:
            result = env_->CallObjectMethod(networkRef_, cache_.network_getSwitch, jId);
            break;
        case prop::BUSBAR_SECTION:
            result = env_->CallObjectMethod(networkRef_, cache_.network_getBusbarSection, jId);
            break;
        case prop::BATTERY:
            result = env_->CallObjectMethod(networkRef_, cache_.network_getBattery, jId);
            break;
        case prop::TWO_WINDINGS_TRANSFORMER:
            result = env_->CallObjectMethod(networkRef_, cache_.network_getTwoWTById, jId);
            break;
        case prop::THREE_WINDINGS_TRANSFORMER:
            result = env_->CallObjectMethod(networkRef_, cache_.network_getThreeWTById, jId);
            break;
        default:
            env_->DeleteLocalRef(jId);
            throw PropertyNotFoundException("findById not supported for type: " + std::to_string(objectType));
    }
    env_->DeleteLocalRef(jId);
    checkJNIException(env_);

    ObjectHandle handle = makeHandle(result);
    if (result) env_->DeleteLocalRef(result);
    return handle;
}

} // namespace iidm
