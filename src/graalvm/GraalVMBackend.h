#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/stdcxx/optional.hpp>
#include <cstdint>
#include <string>
#include <vector>

// Forward declarations for GraalVM types (defined in graal_isolate.h)
struct _graal_isolate_t;
struct _graal_isolatethread_t;
using graal_isolate_t       = _graal_isolate_t;
using graal_isolatethread_t = _graal_isolatethread_t;

namespace iidm {

class GraalVMBackend : public BackendProvider {
public:
    explicit GraalVMBackend(const std::string& libraryPath = "");
    ~GraalVMBackend() override;

    // ── BackendProvider implementation ───────────────────────────────────
    double      getDouble(ObjectHandle h, int property) const override;
    int         getInt(ObjectHandle h, int property) const override;
    bool        getBool(ObjectHandle h, int property) const override;
    std::string getString(ObjectHandle h, int property) const override;

    void setDouble(ObjectHandle h, int property, double value) override;
    void setInt   (ObjectHandle h, int property, int value) override;
    void setBool  (ObjectHandle h, int property, bool value) override;
    void setString(ObjectHandle h, int property, const std::string& value) override;

    iidm::optional<double> getOptDouble(ObjectHandle h, int property) const override;
    iidm::optional<int>         getOptInt(ObjectHandle h, int property) const override;
    iidm::optional<std::string> getOptString(ObjectHandle h, int property) const override;
    std::vector<std::string>    getStringList(ObjectHandle h, int listCode) const override;
    iidm::optional<std::string> getKeyedString(ObjectHandle h, int property, const std::string& key) const override;
    void setKeyedString(ObjectHandle h, int property, const std::string& key,
                        const iidm::optional<std::string>& value) override;
    bool isSameObject(ObjectHandle h, ObjectHandle other) const override;

    std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const override;
    ObjectHandle              getRelated(ObjectHandle h, int relation) const override;
    void                      setRelated(ObjectHandle h, int relation, ObjectHandle value) override;
    std::vector<int>          getIntList(ObjectHandle h, int listCode) const override;
    ObjectHandle              getRelatedByIndex(ObjectHandle h, int relation, int index) const override;
    ObjectHandle              getRelatedByString(ObjectHandle h, int relation, const std::string& key) const override;

    ObjectHandle invoke(ObjectHandle h, int code) override;

    ObjectHandle findById(int objectType, const std::string& id) const override;
    ObjectHandle getNetworkHandle() const override;

    void loadNetwork(const std::string& filePath) override;
    void createNetwork(const std::string& id) override;
    void saveNetwork(const std::string& filePath) override;
    void loadNetworkBytes(const char* data, std::size_t length,
                          const std::string& filenameHint) override;
    std::string saveNetworkBytes(const std::string& filenameHint) const override;
    void close() override;

    void releaseHandle(ObjectHandle h) override;

    // Number of objects registered on the Java side (diagnostics, tests).
    int javaHandleCount() const;

private:
    void init(const std::string& libPath);
    void resolveSymbols();

    void* libHandle_ = nullptr;

    graal_isolate_t*       isolate_ = nullptr;
    graal_isolatethread_t* thread_  = nullptr;

    ObjectHandle networkHandle_ = INVALID_HANDLE;

    // Throws the C++ exception matching a non-zero status of an entry point
    // (the message is fetched with iidm_take_last_error).
    void check(int status) const;
    std::string takeString(char* raw) const;
    iidm::optional<std::string> takeOptString(char* raw) const; // null: no value

    // ── C function pointer types resolved from the shared library ────────
    // Every entry point that can fail returns a status (0 = success, else a BridgeErrorKind)
    // and writes its result through an out-pointer.
    using FnCreateIsolate   = int(*)(void*, graal_isolate_t**, graal_isolatethread_t**);
    using FnTearDownIsolate = int(*)(graal_isolatethread_t*);
    using FnTakeLastError   = char*(*)(graal_isolatethread_t*);
    using FnFree            = void(*)(graal_isolatethread_t*, void*);
    using FnReleaseHandle   = void(*)(graal_isolatethread_t*, ObjectHandle);
    using FnHandleCount     = int(*)(graal_isolatethread_t*);
    using FnLoadNetwork     = int(*)(graal_isolatethread_t*, const char*, ObjectHandle*);
    using FnCreateNetwork   = int(*)(graal_isolatethread_t*, const char*, ObjectHandle*);
    using FnSaveNetwork     = int(*)(graal_isolatethread_t*, ObjectHandle, const char*);
    using FnLoadNetworkBytes = int(*)(graal_isolatethread_t*, const char*, int, const char*, ObjectHandle*);
    using FnSaveNetworkBytes = int(*)(graal_isolatethread_t*, ObjectHandle, const char*, char**, int*);
    using FnGetDouble       = int(*)(graal_isolatethread_t*, ObjectHandle, int, double*);
    using FnSetDouble       = int(*)(graal_isolatethread_t*, ObjectHandle, int, double);
    using FnGetInt          = int(*)(graal_isolatethread_t*, ObjectHandle, int, int*);
    using FnSetInt          = int(*)(graal_isolatethread_t*, ObjectHandle, int, int);
    using FnGetBool         = int(*)(graal_isolatethread_t*, ObjectHandle, int, int*);
    using FnSetBool         = int(*)(graal_isolatethread_t*, ObjectHandle, int, int);
    using FnGetString       = int(*)(graal_isolatethread_t*, ObjectHandle, int, char**);
    using FnSetString       = int(*)(graal_isolatethread_t*, ObjectHandle, int, const char*);
    using FnGetChildren     = int(*)(graal_isolatethread_t*, ObjectHandle, int, ObjectHandle**, int*);
    using FnGetRelated      = int(*)(graal_isolatethread_t*, ObjectHandle, int, ObjectHandle*);
    using FnSetRelated      = int(*)(graal_isolatethread_t*, ObjectHandle, int, ObjectHandle);
    using FnGetIntList      = int(*)(graal_isolatethread_t*, ObjectHandle, int, int**, int*);
    using FnGetRelatedByIndex  = int(*)(graal_isolatethread_t*, ObjectHandle, int, int, ObjectHandle*);
    using FnGetRelatedByString = int(*)(graal_isolatethread_t*, ObjectHandle, int, const char*, ObjectHandle*);
    using FnFindById           = int(*)(graal_isolatethread_t*, ObjectHandle, int, const char*, ObjectHandle*);
    using FnInvoke             = int(*)(graal_isolatethread_t*, ObjectHandle, int, ObjectHandle*);
    using FnGetOptInt          = int(*)(graal_isolatethread_t*, ObjectHandle, int, std::int64_t*);
    using FnGetOptString       = int(*)(graal_isolatethread_t*, ObjectHandle, int, char**);
    using FnGetStringList      = int(*)(graal_isolatethread_t*, ObjectHandle, int, char**, int*);
    using FnGetKeyedString     = int(*)(graal_isolatethread_t*, ObjectHandle, int, const char*, char**);
    using FnSetKeyedString     = int(*)(graal_isolatethread_t*, ObjectHandle, int, const char*, const char*);
    using FnIsSameObject       = int(*)(graal_isolatethread_t*, ObjectHandle, ObjectHandle, int*);

    FnCreateIsolate   fnCreateIsolate_   = nullptr;
    FnTearDownIsolate fnTearDownIsolate_ = nullptr;
    FnTakeLastError   fnTakeLastError_   = nullptr;
    FnFree            fnFree_            = nullptr;
    FnReleaseHandle   fnReleaseHandle_   = nullptr;
    FnHandleCount     fnHandleCount_     = nullptr;
    FnLoadNetwork     fnLoadNetwork_     = nullptr;
    FnCreateNetwork   fnCreateNetwork_   = nullptr;
    FnSaveNetwork     fnSaveNetwork_     = nullptr;
    FnLoadNetworkBytes fnLoadNetworkBytes_ = nullptr;
    FnSaveNetworkBytes fnSaveNetworkBytes_ = nullptr;
    FnGetDouble       fnGetDouble_       = nullptr;
    FnSetDouble       fnSetDouble_       = nullptr;
    FnGetInt          fnGetInt_          = nullptr;
    FnSetInt          fnSetInt_          = nullptr;
    FnGetBool         fnGetBool_         = nullptr;
    FnSetBool         fnSetBool_         = nullptr;
    FnGetString       fnGetString_       = nullptr;
    FnSetString       fnSetString_       = nullptr;
    FnGetChildren     fnGetChildren_     = nullptr;
    FnGetRelated        fnGetRelated_        = nullptr;
    FnSetRelated        fnSetRelated_        = nullptr;
    FnGetIntList        fnGetIntList_        = nullptr;
    FnGetRelatedByIndex  fnGetRelatedByIndex_  = nullptr;
    FnGetRelatedByString fnGetRelatedByString_ = nullptr;
    FnFindById           fnFindById_           = nullptr;
    FnInvoke             fnInvoke_             = nullptr;
    FnGetOptInt          fnGetOptInt_          = nullptr;
    FnGetOptString       fnGetOptString_       = nullptr;
    FnGetStringList      fnGetStringList_      = nullptr;
    FnGetKeyedString     fnGetKeyedString_     = nullptr;
    FnSetKeyedString     fnSetKeyedString_     = nullptr;
    FnIsSameObject       fnIsSameObject_       = nullptr;
};

} // namespace iidm
