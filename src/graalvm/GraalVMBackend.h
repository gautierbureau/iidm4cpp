#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <optional>
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

    std::optional<double> getOptDouble(ObjectHandle h, int property) const override;

    std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const override;
    ObjectHandle              getRelated(ObjectHandle h, int relation) const override;

    ObjectHandle findById(int objectType, const std::string& id) const override;
    ObjectHandle getNetworkHandle() const override;

    void loadNetwork(const std::string& filePath) override;
    void saveNetwork(const std::string& filePath) override;
    void close() override;

private:
    void init(const std::string& libPath);
    void resolveSymbols();

    void* libHandle_ = nullptr;

    graal_isolate_t*       isolate_ = nullptr;
    graal_isolatethread_t* thread_  = nullptr;

    ObjectHandle networkHandle_ = INVALID_HANDLE;

    // ── C function pointer types resolved from the shared library ────────
    using FnCreateIsolate   = int(*)(void*, graal_isolate_t**, graal_isolatethread_t**);
    using FnTearDownIsolate = int(*)(graal_isolatethread_t*);
    using FnLoadNetwork     = ObjectHandle(*)(graal_isolatethread_t*, const char*);
    using FnSaveNetwork     = int(*)(graal_isolatethread_t*, ObjectHandle, const char*);
    using FnGetDouble       = double(*)(graal_isolatethread_t*, ObjectHandle, int);
    using FnSetDouble       = void(*)(graal_isolatethread_t*, ObjectHandle, int, double);
    using FnGetInt          = int(*)(graal_isolatethread_t*, ObjectHandle, int);
    using FnSetInt          = void(*)(graal_isolatethread_t*, ObjectHandle, int, int);
    using FnGetBool         = int(*)(graal_isolatethread_t*, ObjectHandle, int);
    using FnSetBool         = void(*)(graal_isolatethread_t*, ObjectHandle, int, int);
    using FnGetString       = const char*(*)(graal_isolatethread_t*, ObjectHandle, int);
    using FnSetString       = void(*)(graal_isolatethread_t*, ObjectHandle, int, const char*);
    using FnFreeString      = void(*)(graal_isolatethread_t*, const char*);
    using FnGetChildren     = void(*)(graal_isolatethread_t*, ObjectHandle, int,
                                      ObjectHandle*, int*);
    using FnGetRelated      = ObjectHandle(*)(graal_isolatethread_t*, ObjectHandle, int);
    using FnFindById        = ObjectHandle(*)(graal_isolatethread_t*, long, int, const char*);

    FnCreateIsolate   fnCreateIsolate_   = nullptr;
    FnTearDownIsolate fnTearDownIsolate_ = nullptr;
    FnLoadNetwork     fnLoadNetwork_     = nullptr;
    FnSaveNetwork     fnSaveNetwork_     = nullptr;
    FnGetDouble       fnGetDouble_       = nullptr;
    FnSetDouble       fnSetDouble_       = nullptr;
    FnGetInt          fnGetInt_          = nullptr;
    FnSetInt          fnSetInt_          = nullptr;
    FnGetBool         fnGetBool_         = nullptr;
    FnSetBool         fnSetBool_         = nullptr;
    FnGetString       fnGetString_       = nullptr;
    FnSetString       fnSetString_       = nullptr;
    FnFreeString      fnFreeString_      = nullptr;
    FnGetChildren     fnGetChildren_     = nullptr;
    FnGetRelated      fnGetRelated_      = nullptr;
    FnFindById        fnFindById_        = nullptr;
};

} // namespace iidm
