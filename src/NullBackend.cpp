#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>

namespace iidm { namespace detail {

namespace {

// Backend of objects that are not bound to a network. Every operation throws, so using a
// default-constructed wrapper (or a moved-from Network) is a C++ exception, not a crash.
class NullBackend : public BackendProvider {
public:
    double      getDouble(ObjectHandle, int) const override { fail(); }
    int         getInt(ObjectHandle, int) const override { fail(); }
    bool        getBool(ObjectHandle, int) const override { fail(); }
    std::string getString(ObjectHandle, int) const override { fail(); }

    void setDouble(ObjectHandle, int, double) override { fail(); }
    void setInt   (ObjectHandle, int, int) override { fail(); }
    void setBool  (ObjectHandle, int, bool) override { fail(); }
    void setString(ObjectHandle, int, const std::string&) override { fail(); }

    iidm::optional<double> getOptDouble(ObjectHandle, int) const override { fail(); }
    iidm::optional<int>         getOptInt(ObjectHandle, int) const override { fail(); }
    iidm::optional<std::string> getOptString(ObjectHandle, int) const override { fail(); }
    std::vector<std::string>    getStringList(ObjectHandle, int) const override { fail(); }
    iidm::optional<std::string> getKeyedString(ObjectHandle, int, const std::string&) const override { fail(); }
    void setKeyedString(ObjectHandle, int, const std::string&, const iidm::optional<std::string>&) override { fail(); }
    double evaluate(ObjectHandle, int, const std::vector<double>&) const override { fail(); }
    bool isSameObject(ObjectHandle, ObjectHandle) const override { fail(); }

    std::vector<ObjectHandle> getChildren(ObjectHandle, int) const override { fail(); }
    ObjectHandle              getRelated(ObjectHandle, int) const override { fail(); }
    void                      setRelated(ObjectHandle, int, ObjectHandle) override { fail(); }
    std::vector<int>          getIntList(ObjectHandle, int) const override { fail(); }
    ObjectHandle              getRelatedByIndex(ObjectHandle, int, int) const override { fail(); }
    ObjectHandle              getRelatedByString(ObjectHandle, int, const std::string&) const override { fail(); }

    ObjectHandle invoke(ObjectHandle, int) override { fail(); }

    ObjectHandle findById(int, const std::string&) const override { fail(); }
    ObjectHandle getNetworkHandle() const override { fail(); }

    void loadNetwork(const std::string&) override { fail(); }
    void createNetwork(const std::string&) override { fail(); }
    void saveNetwork(const std::string&) override { fail(); }
    void loadNetworkBytes(const char*, std::size_t, const std::string&) override { fail(); }
    std::string saveNetworkBytes(const std::string&) const override { fail(); }

    void close() override {}

private:
    [[noreturn]] static void fail() {
        throw InvalidHandleException(
            "Object not bound to a network (default-constructed, or moved-from Network)");
    }
};

} // namespace

BackendProvider* nullBackend() noexcept {
    static NullBackend instance;
    return &instance;
}

} } // namespace iidm::detail
