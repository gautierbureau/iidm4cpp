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
    // Path to the GraalVM shared library. If empty, searched on LD_LIBRARY_PATH.
    std::string graalvmLibPath;
    // Java network ID to look up in embedded mode (optional; if empty, takes
    // the first network registered in the JNI registry).
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
