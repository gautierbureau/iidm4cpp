#include "GraalVMBackend.h"
#include "../BridgeErrors.h"
#include <iidm/IidmException.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>

// ── Platform-specific dynamic library loading ──────────────────────────────
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
   static std::string platformDlError() {
       DWORD err = GetLastError();
       char buf[256] = {};
       FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      nullptr, err, 0, buf, static_cast<DWORD>(sizeof(buf)), nullptr);
       return buf;
   }
#  define IIDM_DLOPEN(path)      static_cast<void*>(LoadLibraryA(path))
#  define IIDM_DLSYM(lib, name)  static_cast<void*>(GetProcAddress(static_cast<HMODULE>(lib), (name)))
#  define IIDM_DLCLOSE(lib)      FreeLibrary(static_cast<HMODULE>(lib))
#  define IIDM_DLERROR()         platformDlError()
#else
#  include <dlfcn.h>
#  define IIDM_DLOPEN(path)      dlopen((path), RTLD_LAZY | RTLD_LOCAL)
#  define IIDM_DLSYM(lib, name)  dlsym((lib), (name))
#  define IIDM_DLCLOSE(lib)      dlclose(lib)
#  define IIDM_DLERROR()         std::string(dlerror())
#endif

// ── Platform default shared-library name ──────────────────────────────────
#if defined(_WIN32)
constexpr const char* DEFAULT_LIB_NAME = "powsybl-iidm-native.dll";
#elif defined(__APPLE__)
constexpr const char* DEFAULT_LIB_NAME = "libpowsybl-iidm-native.dylib";
#else
constexpr const char* DEFAULT_LIB_NAME = "libpowsybl-iidm-native.so";
#endif

namespace iidm {

namespace {

// Resolve a symbol from the shared library; throws if not found.
template<typename FnPtr>
FnPtr resolveSymbol(void* lib, const char* name) {
    void* sym = IIDM_DLSYM(lib, name);
    if (!sym) {
        throw IidmException(std::string("Failed to resolve symbol '") + name +
                            "': " + IIDM_DLERROR());
    }
    return reinterpret_cast<FnPtr>(sym);
}

} // anonymous namespace

GraalVMBackend::GraalVMBackend(const std::string& libraryPath) {
    std::string libPath = libraryPath;
    if (libPath.empty()) {
        const char* envLib = std::getenv("IIDM_BRIDGE_GRAALVM_LIB");
        libPath = envLib ? envLib : DEFAULT_LIB_NAME;
    }
    init(libPath);
}

GraalVMBackend::~GraalVMBackend() {
    close();
}

void GraalVMBackend::init(const std::string& libPath) {
    libHandle_ = IIDM_DLOPEN(libPath.c_str());
    if (!libHandle_) {
        throw BackendNotAvailableException(
            "Cannot load GraalVM native library '" + libPath + "': " + IIDM_DLERROR());
    }

    resolveSymbols();

    if (fnCreateIsolate_(nullptr, &isolate_, &thread_) != 0) {
        throw IidmException("Failed to create GraalVM isolate");
    }
}

void GraalVMBackend::resolveSymbols() {
    fnCreateIsolate_   = resolveSymbol<FnCreateIsolate>  (libHandle_, "graal_create_isolate");
    fnTearDownIsolate_ = resolveSymbol<FnTearDownIsolate>(libHandle_, "graal_tear_down_isolate");
    fnTakeLastError_   = resolveSymbol<FnTakeLastError>  (libHandle_, "iidm_take_last_error");
    fnFree_            = resolveSymbol<FnFree>           (libHandle_, "iidm_free");
    fnReleaseHandle_   = resolveSymbol<FnReleaseHandle>  (libHandle_, "iidm_release_handle");
    fnHandleCount_     = resolveSymbol<FnHandleCount>    (libHandle_, "iidm_handle_count");
    fnLoadNetwork_     = resolveSymbol<FnLoadNetwork>    (libHandle_, "iidm_load_network");
    fnCreateNetwork_   = resolveSymbol<FnCreateNetwork>  (libHandle_, "iidm_create_network");
    fnSaveNetwork_     = resolveSymbol<FnSaveNetwork>    (libHandle_, "iidm_save_network");
    fnLoadNetworkBytes_ = resolveSymbol<FnLoadNetworkBytes>(libHandle_, "iidm_load_network_bytes");
    fnSaveNetworkBytes_ = resolveSymbol<FnSaveNetworkBytes>(libHandle_, "iidm_save_network_bytes");
    fnGetDouble_       = resolveSymbol<FnGetDouble>      (libHandle_, "iidm_get_double");
    fnSetDouble_       = resolveSymbol<FnSetDouble>      (libHandle_, "iidm_set_double");
    fnGetInt_          = resolveSymbol<FnGetInt>         (libHandle_, "iidm_get_int");
    fnSetInt_          = resolveSymbol<FnSetInt>         (libHandle_, "iidm_set_int");
    fnGetBool_         = resolveSymbol<FnGetBool>        (libHandle_, "iidm_get_bool");
    fnSetBool_         = resolveSymbol<FnSetBool>        (libHandle_, "iidm_set_bool");
    fnGetString_       = resolveSymbol<FnGetString>      (libHandle_, "iidm_get_string");
    fnSetString_       = resolveSymbol<FnSetString>      (libHandle_, "iidm_set_string");
    fnGetChildren_     = resolveSymbol<FnGetChildren>    (libHandle_, "iidm_get_children");
    fnGetRelated_        = resolveSymbol<FnGetRelated>       (libHandle_, "iidm_get_related");
    fnSetRelated_        = resolveSymbol<FnSetRelated>       (libHandle_, "iidm_set_related");
    fnGetIntList_        = resolveSymbol<FnGetIntList>       (libHandle_, "iidm_get_int_list");
    fnGetRelatedByIndex_  = resolveSymbol<FnGetRelatedByIndex> (libHandle_, "iidm_get_related_by_index");
    fnGetRelatedByString_ = resolveSymbol<FnGetRelatedByString>(libHandle_, "iidm_get_related_by_string");
    fnFindById_           = resolveSymbol<FnFindById>          (libHandle_, "iidm_find_by_id");
    fnInvoke_             = resolveSymbol<FnInvoke>            (libHandle_, "iidm_invoke");
}

void GraalVMBackend::check(int status) const {
    if (status == detail::BRIDGE_OK) return;
    char* raw = fnTakeLastError_(thread_);
    std::string message = raw ? std::string(raw) : std::string("unknown error in the GraalVM backend");
    if (raw) fnFree_(thread_, raw);
    detail::throwBridgeError(status, message);
}

std::string GraalVMBackend::takeString(char* raw) const {
    std::string result(raw ? raw : "");
    if (raw) fnFree_(thread_, raw);
    return result;
}

void GraalVMBackend::loadNetwork(const std::string& filePath) {
    ObjectHandle h = INVALID_HANDLE;
    check(fnLoadNetwork_(thread_, filePath.c_str(), &h));
    networkHandle_ = h;
}

void GraalVMBackend::createNetwork(const std::string& id) {
    ObjectHandle h = INVALID_HANDLE;
    check(fnCreateNetwork_(thread_, id.c_str(), &h));
    networkHandle_ = h;
}

void GraalVMBackend::saveNetwork(const std::string& filePath) {
    check(fnSaveNetwork_(thread_, networkHandle_, filePath.c_str()));
}

void GraalVMBackend::loadNetworkBytes(const char* data, std::size_t length,
                                      const std::string& filenameHint) {
    ObjectHandle h = INVALID_HANDLE;
    check(fnLoadNetworkBytes_(thread_, data, static_cast<int>(length), filenameHint.c_str(), &h));
    networkHandle_ = h;
}

std::string GraalVMBackend::saveNetworkBytes(const std::string& filenameHint) const {
    char* outData = nullptr;
    int   outLen  = 0;
    check(fnSaveNetworkBytes_(thread_, networkHandle_, filenameHint.c_str(), &outData, &outLen));
    std::string result(outData ? outData : "", static_cast<std::size_t>(outLen));
    if (outData) fnFree_(thread_, outData);
    return result;
}

void GraalVMBackend::releaseHandle(ObjectHandle h) {
    // The network handle lives as long as the backend
    if (thread_ && h != INVALID_HANDLE && h != networkHandle_) {
        fnReleaseHandle_(thread_, h);
    }
}

int GraalVMBackend::javaHandleCount() const {
    return fnHandleCount_(thread_);
}

void GraalVMBackend::close() {
    if (thread_) {
        // Wrappers still alive at this point release nothing later on
        releaseAllHandles();
    }
    if (thread_ && fnTearDownIsolate_) {
        fnTearDownIsolate_(thread_);
        thread_  = nullptr;
        isolate_ = nullptr;
    }
    if (libHandle_) {
        IIDM_DLCLOSE(libHandle_);
        libHandle_ = nullptr;
    }
}

ObjectHandle GraalVMBackend::getNetworkHandle() const {
    return networkHandle_;
}

double GraalVMBackend::getDouble(ObjectHandle h, int property) const {
    double value = 0.0;
    check(fnGetDouble_(thread_, h, property, &value));
    return value;
}

void GraalVMBackend::setDouble(ObjectHandle h, int property, double value) {
    check(fnSetDouble_(thread_, h, property, value));
}

int GraalVMBackend::getInt(ObjectHandle h, int property) const {
    int value = 0;
    check(fnGetInt_(thread_, h, property, &value));
    return value;
}

void GraalVMBackend::setInt(ObjectHandle h, int property, int value) {
    check(fnSetInt_(thread_, h, property, value));
}

bool GraalVMBackend::getBool(ObjectHandle h, int property) const {
    int value = 0;
    check(fnGetBool_(thread_, h, property, &value));
    return value != 0;
}

void GraalVMBackend::setBool(ObjectHandle h, int property, bool value) {
    check(fnSetBool_(thread_, h, property, value ? 1 : 0));
}

std::string GraalVMBackend::getString(ObjectHandle h, int property) const {
    char* raw = nullptr;
    check(fnGetString_(thread_, h, property, &raw));
    return takeString(raw);
}

void GraalVMBackend::setString(ObjectHandle h, int property, const std::string& value) {
    check(fnSetString_(thread_, h, property, value.c_str()));
}

iidm::optional<double> GraalVMBackend::getOptDouble(ObjectHandle h, int property) const {
    double val = getDouble(h, property);
    // NaN signals "undefined"
    if (val != val) return {};  // NaN check
    return val;
}

std::vector<ObjectHandle> GraalVMBackend::getChildren(ObjectHandle h, int childType) const {
    ObjectHandle* raw = nullptr;
    int count = 0;
    check(fnGetChildren_(thread_, h, childType, &raw, &count));
    std::vector<ObjectHandle> handles(raw, raw + (raw ? count : 0));
    if (raw) fnFree_(thread_, raw);
    return handles;
}

ObjectHandle GraalVMBackend::getRelated(ObjectHandle h, int relation) const {
    ObjectHandle related = INVALID_HANDLE;
    check(fnGetRelated_(thread_, h, relation, &related));
    return related;
}

ObjectHandle GraalVMBackend::invoke(ObjectHandle h, int code) {
    ObjectHandle result = INVALID_HANDLE;
    check(fnInvoke_(thread_, h, code, &result));
    return result;
}

void GraalVMBackend::setRelated(ObjectHandle h, int relation, ObjectHandle value) {
    check(fnSetRelated_(thread_, h, relation, value));
}

std::vector<int> GraalVMBackend::getIntList(ObjectHandle h, int listCode) const {
    int* raw = nullptr;
    int count = 0;
    check(fnGetIntList_(thread_, h, listCode, &raw, &count));
    std::vector<int> values(raw, raw + (raw ? count : 0));
    if (raw) fnFree_(thread_, raw);
    return values;
}

ObjectHandle GraalVMBackend::getRelatedByIndex(ObjectHandle h, int relation, int index) const {
    ObjectHandle related = INVALID_HANDLE;
    check(fnGetRelatedByIndex_(thread_, h, relation, index, &related));
    return related;
}

ObjectHandle GraalVMBackend::getRelatedByString(ObjectHandle h, int relation, const std::string& key) const {
    ObjectHandle related = INVALID_HANDLE;
    check(fnGetRelatedByString_(thread_, h, relation, key.c_str(), &related));
    return related;
}

ObjectHandle GraalVMBackend::findById(int objectType, const std::string& id) const {
    ObjectHandle found = INVALID_HANDLE;
    check(fnFindById_(thread_, networkHandle_, objectType, id.c_str(), &found));
    return found;
}

} // namespace iidm
