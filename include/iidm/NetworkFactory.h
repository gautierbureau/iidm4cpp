#pragma once

#include <iidm/Network.h>
#include <iosfwd>
#include <string>

namespace iidm {

enum class BackendMode {
    AUTO,    // probe for existing JVM first, fall back to GraalVM
    GRAALVM, // always use GraalVM native image
    JNI      // always use existing JVM (throws if no JVM found)
};

struct NetworkLoadOptions {
    BackendMode mode = BackendMode::AUTO;
    // Path to the GraalVM native image. If empty: IIDM_BRIDGE_GRAALVM_LIB, else
    // libpowsybl-iidm-native (.so, .dylib or .dll) from the library search path.
    std::string graalvmLibPath;
    // Id of the network in the JNI registry (IidmBridgeRegistry): the JNI backend works on the
    // network registered under it (embedded mode), and registers under it the networks it loads
    // or creates. If empty, "default", the id of IidmBridgeRegistry.register(network).
    std::string jniNetworkId;
    // Filename hint passed to PowSyBl when loading/saving from a stream or
    // byte buffer. The extension drives importer/exporter selection; today
    // only XIIDM is supported.
    std::string filenameHint = "network.xiidm";
};

class NetworkFactory {
public:
    // Load a network from an XIIDM file (standalone or embedded).
    static Network load(const std::string& filePath,
                        const NetworkLoadOptions& options = {});

    // Load a network from a std::istream. Reads the stream to EOF, then hands
    // the bytes to the active backend. `options.filenameHint` selects the
    // importer (default "network.xiidm").
    static Network load(std::istream& in,
                        const NetworkLoadOptions& options = {});

    // Load a network from a raw byte buffer (e.g. an existing std::string or
    // std::vector<char>). Same semantics as the istream overload.
    static Network loadFromBytes(const char* data, std::size_t length,
                                 const NetworkLoadOptions& options = {});
    static Network loadFromBytes(const std::string& bytes,
                                 const NetworkLoadOptions& options = {});

    // Save a network to an XIIDM file.
    // A new, empty network (the backend is chosen as for load)
    static Network create(const std::string& id,
                          const NetworkLoadOptions& options = {});

    static void save(const Network& network, const std::string& filePath);

    // Embedded mode only: wrap a network that is already in the JVM without
    // loading from file. The Java side must have called
    // IidmBridgeRegistry.register(network) before this point.
    static Network wrap(const std::string& networkId,
                        const NetworkLoadOptions& options = {});

    // Advanced / testing: create a Network from a custom BackendProvider.
    // Useful for injecting mock backends in unit tests.
    static Network createFromBackend(std::unique_ptr<BackendProvider> backend);
};

} // namespace iidm
