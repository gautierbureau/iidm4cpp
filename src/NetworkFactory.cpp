#include <iidm/NetworkFactory.h>
#include <iidm/IidmException.h>
#include "BackendSelector.h"

#include <istream>
#include <iterator>
#include <sstream>

namespace iidm {

Network NetworkFactory::load(const std::string& filePath, const NetworkLoadOptions& options) {
    auto backend = selectBackend(options);
    backend->loadNetwork(filePath);
    return Network(std::move(backend));
}

Network NetworkFactory::load(std::istream& in, const NetworkLoadOptions& options) {
    std::ostringstream buffer;
    buffer << in.rdbuf();
    const std::string bytes = buffer.str();
    return loadFromBytes(bytes.data(), bytes.size(), options);
}

Network NetworkFactory::loadFromBytes(const char* data, std::size_t length,
                                      const NetworkLoadOptions& options) {
    auto backend = selectBackend(options);
    backend->loadNetworkBytes(data, length, options.filenameHint);
    return Network(std::move(backend));
}

Network NetworkFactory::loadFromBytes(const std::string& bytes,
                                      const NetworkLoadOptions& options) {
    return loadFromBytes(bytes.data(), bytes.size(), options);
}

Network NetworkFactory::wrap(const std::string& networkId, const NetworkLoadOptions& options) {
    NetworkLoadOptions jniOptions = options;
    jniOptions.mode = BackendMode::JNI;
    if (!networkId.empty()) {
        jniOptions.jniNetworkId = networkId;
    }
    auto backend = selectBackend(jniOptions);
    // In wrap mode, the network is already loaded in the JVM — no loadNetwork call needed.
    return Network(std::move(backend));
}

Network NetworkFactory::createFromBackend(std::unique_ptr<BackendProvider> backend) {
    return Network(std::move(backend));
}

void NetworkFactory::save(const Network& network, const std::string& filePath) {
    network.backend_->saveNetwork(filePath);
}

} // namespace iidm
