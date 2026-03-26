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
    cacheClass(cache_.iidmRegistryClass, "com/powsybl/iidmbridge/jni/IidmBridgeRegistry");
    cacheClass(cache_.listClass,         "java/util/List");
    cacheClass(cache_.energySourceClass, "com/powsybl/iidm/network/EnergySource");
    cacheClass(cache_.loadTypeClass,     "com/powsybl/iidm/network/LoadType");
    cacheClass(cache_.topologyKindClass, "com/powsybl/iidm/network/TopologyKind");
    cacheClass(cache_.countryClass,      "com/powsybl/iidm/network/Country");

    // Enum ordinal (shared across all enums)
    jclass enumClass = env_->FindClass("java/lang/Enum");
    cache_.enum_ordinal = env_->GetMethodID(enumClass, "ordinal", "()I");
    env_->DeleteLocalRef(enumClass);

    // IidmBridgeRegistry
    cache_.registry_get = env_->GetStaticMethodID(cache_.iidmRegistryClass,
        "get", "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Network;");
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
        default:
            throw PropertyNotFoundException("Unknown double property: " + std::to_string(property));
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
        default:
            throw PropertyNotFoundException("Unknown int property: " + std::to_string(property));
    }
}

void JNIBackend::setInt(ObjectHandle h, int property, int /*value*/) {
    (void)h;
    throw PropertyNotFoundException("setInt not supported for property: " + std::to_string(property));
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
        default:
            throw PropertyNotFoundException("Unknown bool property: " + std::to_string(property));
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

void JNIBackend::setString(ObjectHandle /*h*/, int property, const std::string& /*value*/) {
    throw PropertyNotFoundException("setString not supported for property: " + std::to_string(property));
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
            related = env_->CallObjectMethod(obj, cache_.generator_getTerminal);
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
