#pragma once

#include <string>

namespace iidm {

struct TemporaryLimit {
    std::string name;
    double      value;
    int         acceptableDuration;
    bool        fictitious;
};

} // namespace iidm
