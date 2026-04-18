#include "GraalVMBackend.h"
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
    fnLoadNetwork_     = resolveSymbol<FnLoadNetwork>    (libHandle_, "iidm_load_network");
    fnSaveNetwork_     = resolveSymbol<FnSaveNetwork>    (libHandle_, "iidm_save_network");
    fnGetDouble_       = resolveSymbol<FnGetDouble>      (libHandle_, "iidm_get_double");
    fnSetDouble_       = resolveSymbol<FnSetDouble>      (libHandle_, "iidm_set_double");
    fnGetInt_          = resolveSymbol<FnGetInt>         (libHandle_, "iidm_get_int");
    fnSetInt_          = resolveSymbol<FnSetInt>         (libHandle_, "iidm_set_int");
    fnGetBool_         = resolveSymbol<FnGetBool>        (libHandle_, "iidm_get_bool");
    fnSetBool_         = resolveSymbol<FnSetBool>        (libHandle_, "iidm_set_bool");
    fnGetString_       = resolveSymbol<FnGetString>      (libHandle_, "iidm_get_string");
    fnSetString_       = resolveSymbol<FnSetString>      (libHandle_, "iidm_set_string");
    fnFreeString_      = resolveSymbol<FnFreeString>     (libHandle_, "iidm_free_string");
    fnGetChildren_     = resolveSymbol<FnGetChildren>    (libHandle_, "iidm_get_children");
    fnGetRelated_      = resolveSymbol<FnGetRelated>     (libHandle_, "iidm_get_related");
    fnFindById_        = resolveSymbol<FnFindById>       (libHandle_, "iidm_find_by_id");
}

void GraalVMBackend::loadNetwork(const std::string& filePath) {
    networkHandle_ = fnLoadNetwork_(thread_, filePath.c_str());
    if (networkHandle_ == INVALID_HANDLE) {
        throw IidmException("Failed to load network from: " + filePath);
    }
}

void GraalVMBackend::saveNetwork(const std::string& filePath) {
    int rc = fnSaveNetwork_(thread_, networkHandle_, filePath.c_str());
    if (rc != 0) {
        throw IidmException("Failed to save network to: " + filePath);
    }
}

void GraalVMBackend::close() {
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
    return fnGetDouble_(thread_, h, property);
}

void GraalVMBackend::setDouble(ObjectHandle h, int property, double value) {
    fnSetDouble_(thread_, h, property, value);
}

int GraalVMBackend::getInt(ObjectHandle h, int property) const {
    return fnGetInt_(thread_, h, property);
}

void GraalVMBackend::setInt(ObjectHandle h, int property, int value) {
    fnSetInt_(thread_, h, property, value);
}

bool GraalVMBackend::getBool(ObjectHandle h, int property) const {
    return fnGetBool_(thread_, h, property) != 0;
}

void GraalVMBackend::setBool(ObjectHandle h, int property, bool value) {
    fnSetBool_(thread_, h, property, value ? 1 : 0);
}

std::string GraalVMBackend::getString(ObjectHandle h, int property) const {
    const char* raw = fnGetString_(thread_, h, property);
    std::string result(raw ? raw : "");
    if (raw) {
        fnFreeString_(thread_, raw);
    }
    return result;
}

void GraalVMBackend::setString(ObjectHandle h, int property, const std::string& value) {
    fnSetString_(thread_, h, property, value.c_str());
}

std::optional<double> GraalVMBackend::getOptDouble(ObjectHandle h, int property) const {
    double val = fnGetDouble_(thread_, h, property);
    // NaN signals "undefined"
    if (val != val) return std::nullopt;  // NaN check
    return val;
}

std::vector<ObjectHandle> GraalVMBackend::getChildren(ObjectHandle h, int childType) const {
    // First call with nullptr to get count
    int count = 0;
    fnGetChildren_(thread_, h, childType, nullptr, &count);
    if (count == 0) return {};

    std::vector<ObjectHandle> handles(static_cast<size_t>(count));
    fnGetChildren_(thread_, h, childType, handles.data(), &count);
    return handles;
}

ObjectHandle GraalVMBackend::getRelated(ObjectHandle h, int relation) const {
    return fnGetRelated_(thread_, h, relation);
}

ObjectHandle GraalVMBackend::findById(int objectType, const std::string& id) const {
    return fnFindById_(thread_, networkHandle_, objectType, id.c_str());
}

} // namespace iidm
