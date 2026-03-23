#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/NetworkFactory.h>
#include <memory>

namespace iidm {

std::unique_ptr<BackendProvider> selectBackend(const NetworkLoadOptions& opts);

} // namespace iidm
