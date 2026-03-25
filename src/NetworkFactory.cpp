#include <iidm/NetworkFactory.h>
#include <iidm/IidmException.h>
#include "BackendSelector.h"

namespace iidm {

Network NetworkFactory::load(const std::string& filePath, const NetworkLoadOptions& options) {
    auto backend = selectBackend(options);
    backend->loadNetwork(filePath);
    return Network(std::move(backend));
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

} // namespace iidm
