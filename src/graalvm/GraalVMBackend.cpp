#include "GraalVMBackend.h"
#include "../BridgeErrors.h"
#include "../PropertyCodesCheck.h"
#include <iidm/IidmException.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

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
// The name the build gives the native image (native-image -o libpowsybl-iidm-native)
#if defined(_WIN32)
constexpr const char* DEFAULT_LIB_NAME = "libpowsybl-iidm-native.dll";
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

using FnDetachThread = int(*)(graal_isolatethread_t*);

// The open isolates, by id, with the graal_detach_thread of their library. A thread that ends
// detaches from those only: closing an isolate detaches all its threads, and another isolate may
// get its address. Never destroyed, as threads may end after the static objects.
struct OpenIsolates {
    std::mutex mutex;
    std::map<std::uint64_t, FnDetachThread> detachThread;
    std::uint64_t lastId = 0;
};

OpenIsolates& openIsolates() {
    static OpenIsolates* isolates = new OpenIsolates();
    return *isolates;
}

// The isolate threads of the calling thread. Plain thread-local values: still readable once the
// thread's destructors have run (static objects closing a network at exit).
struct IsolateThread {
    std::uint64_t          isolateId;
    graal_isolatethread_t* thread;
};
thread_local std::vector<IsolateThread>* threadIsolates = nullptr;
thread_local bool threadEnded = false;

// Detaches the thread from the isolates still open when it ends
struct ThreadExit {
    bool armed = false;
    ~ThreadExit() {
        threadEnded = true;
        if (!threadIsolates) return;
        {
            OpenIsolates& isolates = openIsolates();
            std::lock_guard<std::mutex> lock(isolates.mutex);
            for (const IsolateThread& attached : *threadIsolates) {
                auto open = isolates.detachThread.find(attached.isolateId);
                if (open != isolates.detachThread.end()) open->second(attached.thread);
            }
        }
        delete threadIsolates;
        threadIsolates = nullptr;
    }
};
thread_local ThreadExit threadExit;

// Records an isolate thread of the calling thread, and forgets those of the isolates closed since
void rememberThread(std::uint64_t isolateId, graal_isolatethread_t* thread) {
    if (!threadIsolates) {
        threadIsolates = new std::vector<IsolateThread>();
        if (!threadEnded) threadExit.armed = true;
    } else {
        OpenIsolates& isolates = openIsolates();
        std::lock_guard<std::mutex> lock(isolates.mutex);
        threadIsolates->erase(std::remove_if(threadIsolates->begin(), threadIsolates->end(),
                                             [&isolates](const IsolateThread& attached) {
                                                 return isolates.detachThread.count(attached.isolateId) == 0;
                                             }),
                              threadIsolates->end());
    }
    threadIsolates->push_back({isolateId, thread});
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

    graal_isolatethread_t* thread = nullptr;
    if (fnCreateIsolate_(nullptr, &isolate_, &thread) != 0) {
        throw IidmException("Failed to create GraalVM isolate");
    }
    OpenIsolates& isolates = openIsolates();
    {
        std::lock_guard<std::mutex> lock(isolates.mutex);
        isolateId_ = ++isolates.lastId;
        isolates.detachThread[isolateId_] = fnDetachThread_;
    }
    rememberThread(isolateId_, thread);

    // An image older than the check has no iidm_codes_hash
    using FnCodesHash = std::int64_t(*)(graal_isolatethread_t*);
    auto codesHash = reinterpret_cast<FnCodesHash>(IIDM_DLSYM(libHandle_, "iidm_codes_hash"));
    try {
        detail::checkPropertyCodes(codesHash ? static_cast<std::uint64_t>(codesHash(thread)) : 0,
                                   "The GraalVM native library " + libPath);
    } catch (...) {
        close();
        throw;
    }
}

graal_isolatethread_t* GraalVMBackend::thread() const {
    if (threadIsolates) {
        for (const IsolateThread& attached : *threadIsolates) {
            if (attached.isolateId == isolateId_) return attached.thread;
        }
    }
    return attachThread();
}

graal_isolatethread_t* GraalVMBackend::attachThread() const {
    if (!isolate_) throw InvalidHandleException("The network of this object is closed");
    graal_isolatethread_t* thread = nullptr;
    if (fnAttachThread_(isolate_, &thread) != 0) {
        throw IidmException("Failed to attach the thread to the GraalVM isolate");
    }
    rememberThread(isolateId_, thread);
    return thread;
}

void GraalVMBackend::resolveSymbols() {
    fnCreateIsolate_   = resolveSymbol<FnCreateIsolate>  (libHandle_, "graal_create_isolate");
    fnAttachThread_    = resolveSymbol<FnAttachThread>   (libHandle_, "graal_attach_thread");
    fnDetachThread_    = resolveSymbol<FnDetachThread>   (libHandle_, "graal_detach_thread");
    // Unlike graal_tear_down_isolate, does not wait for the threads still attached to detach
    fnTearDownIsolate_ = resolveSymbol<FnTearDownIsolate>(libHandle_, "graal_detach_all_threads_and_tear_down_isolate");
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
    fnGetOptInt_          = resolveSymbol<FnGetOptInt>         (libHandle_, "iidm_get_opt_int");
    fnGetOptString_       = resolveSymbol<FnGetOptString>      (libHandle_, "iidm_get_opt_string");
    fnGetStringList_      = resolveSymbol<FnGetStringList>     (libHandle_, "iidm_get_string_list");
    fnGetKeyedString_     = resolveSymbol<FnGetKeyedString>    (libHandle_, "iidm_get_keyed_string");
    fnSetKeyedString_     = resolveSymbol<FnSetKeyedString>    (libHandle_, "iidm_set_keyed_string");
    fnIsSameObject_       = resolveSymbol<FnIsSameObject>      (libHandle_, "iidm_is_same_object");
    fnEvaluate_           = resolveSymbol<FnEvaluate>          (libHandle_, "iidm_evaluate");
}

void GraalVMBackend::check(int status) const {
    if (status == detail::BRIDGE_OK) return;
    char* raw = fnTakeLastError_(thread());
    std::string message = raw ? std::string(raw) : std::string("unknown error in the GraalVM backend");
    if (raw) fnFree_(thread(), raw);
    detail::throwBridgeError(status, message);
}

std::string GraalVMBackend::takeString(char* raw) const {
    std::string result(raw ? raw : "");
    if (raw) fnFree_(thread(), raw);
    return result;
}

iidm::optional<std::string> GraalVMBackend::takeOptString(char* raw) const {
    if (!raw) return {};
    return takeString(raw);
}

void GraalVMBackend::loadNetwork(const std::string& filePath) {
    ObjectHandle h = INVALID_HANDLE;
    check(fnLoadNetwork_(thread(), filePath.c_str(), &h));
    networkHandle_ = h;
}

void GraalVMBackend::createNetwork(const std::string& id) {
    ObjectHandle h = INVALID_HANDLE;
    check(fnCreateNetwork_(thread(), id.c_str(), &h));
    networkHandle_ = h;
}

void GraalVMBackend::saveNetwork(ObjectHandle network, const std::string& filePath) {
    check(fnSaveNetwork_(thread(), network, filePath.c_str()));
}

void GraalVMBackend::loadNetworkBytes(const char* data, std::size_t length,
                                      const std::string& filenameHint) {
    ObjectHandle h = INVALID_HANDLE;
    check(fnLoadNetworkBytes_(thread(), data, static_cast<int>(length), filenameHint.c_str(), &h));
    networkHandle_ = h;
}

std::string GraalVMBackend::saveNetworkBytes(ObjectHandle network, const std::string& filenameHint) const {
    char* outData = nullptr;
    int   outLen  = 0;
    check(fnSaveNetworkBytes_(thread(), network, filenameHint.c_str(), &outData, &outLen));
    std::string result(outData ? outData : "", static_cast<std::size_t>(outLen));
    if (outData) fnFree_(thread(), outData);
    return result;
}

void GraalVMBackend::releaseHandle(ObjectHandle h) {
    // The network handle lives as long as the backend
    if (isolate_ && h != INVALID_HANDLE && h != networkHandle_) {
        fnReleaseHandle_(thread(), h);
    }
}

int GraalVMBackend::javaHandleCount() const {
    return fnHandleCount_(thread());
}

void GraalVMBackend::close() {
    if (isolate_) {
        // Wrappers still alive at this point release nothing later on
        releaseAllHandles();
        try {
            graal_isolatethread_t* current = thread();
            {
                // The threads that end from now on leave their detaching to the tear-down
                OpenIsolates& isolates = openIsolates();
                std::lock_guard<std::mutex> lock(isolates.mutex);
                isolates.detachThread.erase(isolateId_);
            }
            fnTearDownIsolate_(current);
        } catch (const IidmException&) {
            // This thread cannot be attached: the isolate is left as it is
        }
        isolateId_ = 0;
        isolate_   = nullptr;
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
    check(fnGetDouble_(thread(), h, property, &value));
    return value;
}

void GraalVMBackend::setDouble(ObjectHandle h, int property, double value) {
    check(fnSetDouble_(thread(), h, property, value));
}

int GraalVMBackend::getInt(ObjectHandle h, int property) const {
    int value = 0;
    check(fnGetInt_(thread(), h, property, &value));
    return value;
}

void GraalVMBackend::setInt(ObjectHandle h, int property, int value) {
    check(fnSetInt_(thread(), h, property, value));
}

bool GraalVMBackend::getBool(ObjectHandle h, int property) const {
    int value = 0;
    check(fnGetBool_(thread(), h, property, &value));
    return value != 0;
}

void GraalVMBackend::setBool(ObjectHandle h, int property, bool value) {
    check(fnSetBool_(thread(), h, property, value ? 1 : 0));
}

std::string GraalVMBackend::getString(ObjectHandle h, int property) const {
    char* raw = nullptr;
    check(fnGetString_(thread(), h, property, &raw));
    return takeString(raw);
}

void GraalVMBackend::setString(ObjectHandle h, int property, const std::string& value) {
    check(fnSetString_(thread(), h, property, value.c_str()));
}

iidm::optional<double> GraalVMBackend::getOptDouble(ObjectHandle h, int property) const {
    double val = getDouble(h, property);
    // NaN signals "undefined"
    if (val != val) return {};  // NaN check
    return val;
}

iidm::optional<int> GraalVMBackend::getOptInt(ObjectHandle h, int property) const {
    std::int64_t value = 0;
    check(fnGetOptInt_(thread(), h, property, &value));
    if (value == std::numeric_limits<std::int64_t>::min()) return {}; // PropertyDispatcher.NO_INT
    return static_cast<int>(value);
}

iidm::optional<std::string> GraalVMBackend::getOptString(ObjectHandle h, int property) const {
    char* raw = nullptr;
    check(fnGetOptString_(thread(), h, property, &raw));
    return takeOptString(raw);
}

std::vector<std::string> GraalVMBackend::getStringList(ObjectHandle h, int listCode) const {
    char* raw = nullptr;
    int length = 0;
    check(fnGetStringList_(thread(), h, listCode, &raw, &length));
    // Each string is followed by a 0 byte
    std::vector<std::string> values;
    for (int start = 0, i = 0; i < length; ++i) {
        if (raw[i] == '\0') {
            values.emplace_back(raw + start, static_cast<std::size_t>(i - start));
            start = i + 1;
        }
    }
    if (raw) fnFree_(thread(), raw);
    return values;
}

iidm::optional<std::string> GraalVMBackend::getKeyedString(ObjectHandle h, int property, const std::string& key) const {
    char* raw = nullptr;
    check(fnGetKeyedString_(thread(), h, property, key.c_str(), &raw));
    return takeOptString(raw);
}

void GraalVMBackend::setKeyedString(ObjectHandle h, int property, const std::string& key,
                                    const iidm::optional<std::string>& value) {
    check(fnSetKeyedString_(thread(), h, property, key.c_str(), value ? value->c_str() : nullptr));
}

bool GraalVMBackend::isSameObject(ObjectHandle h, ObjectHandle other) const {
    int same = 0;
    check(fnIsSameObject_(thread(), h, other, &same));
    return same != 0;
}

std::vector<ObjectHandle> GraalVMBackend::getChildren(ObjectHandle h, int childType) const {
    ObjectHandle* raw = nullptr;
    int count = 0;
    check(fnGetChildren_(thread(), h, childType, &raw, &count));
    std::vector<ObjectHandle> handles(raw, raw + (raw ? count : 0));
    if (raw) fnFree_(thread(), raw);
    return handles;
}

ObjectHandle GraalVMBackend::getRelated(ObjectHandle h, int relation) const {
    ObjectHandle related = INVALID_HANDLE;
    check(fnGetRelated_(thread(), h, relation, &related));
    return related;
}

ObjectHandle GraalVMBackend::invoke(ObjectHandle h, int code) {
    ObjectHandle result = INVALID_HANDLE;
    check(fnInvoke_(thread(), h, code, &result));
    return result;
}

void GraalVMBackend::setRelated(ObjectHandle h, int relation, ObjectHandle value) {
    check(fnSetRelated_(thread(), h, relation, value));
}

std::vector<int> GraalVMBackend::getIntList(ObjectHandle h, int listCode) const {
    int* raw = nullptr;
    int count = 0;
    check(fnGetIntList_(thread(), h, listCode, &raw, &count));
    std::vector<int> values(raw, raw + (raw ? count : 0));
    if (raw) fnFree_(thread(), raw);
    return values;
}

double GraalVMBackend::evaluate(ObjectHandle h, int code, const std::vector<double>& args) const {
    double result = 0.0;
    check(fnEvaluate_(thread(), h, code, args.empty() ? nullptr : args.data(), static_cast<int>(args.size()), &result));
    return result;
}

ObjectHandle GraalVMBackend::getRelatedByIndex(ObjectHandle h, int relation, int index) const {
    ObjectHandle related = INVALID_HANDLE;
    check(fnGetRelatedByIndex_(thread(), h, relation, index, &related));
    return related;
}

ObjectHandle GraalVMBackend::getRelatedByString(ObjectHandle h, int relation, const std::string& key) const {
    ObjectHandle related = INVALID_HANDLE;
    check(fnGetRelatedByString_(thread(), h, relation, key.c_str(), &related));
    return related;
}

ObjectHandle GraalVMBackend::findById(int objectType, const std::string& id) const {
    ObjectHandle found = INVALID_HANDLE;
    check(fnFindById_(thread(), networkHandle_, objectType, id.c_str(), &found));
    return found;
}

} // namespace iidm
