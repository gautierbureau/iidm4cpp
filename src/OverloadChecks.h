#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/Enums.h>
#include <iidm/HandleRef.h>
#include <iidm/Overload.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include "Wrap.h"
#include <vector>

namespace iidm { namespace detail {

// The overload checks of a side of a branch or of a three windings transformer (side: the ordinal of
// its TwoSides or ThreeSides), with or without a limit reduction.

inline bool overPermanentLimit(BackendProvider* backend, ObjectHandle checked, int side,
                               const iidm::optional<double>& limitReductionValue, LimitType type) {
    std::vector<double> args(1, static_cast<double>(side));
    if (limitReductionValue) args.push_back(*limitReductionValue);
    args.push_back(static_cast<double>(type));
    return backend->evaluate(checked, prop::CHECK_PERMANENT_LIMIT, args) != 0;
}

// A check of the temporary limits (CHECK_TEMPORARY_LIMITS or CHECK_ALL_TEMPORARY_LIMITS), run:
// what it returns
inline ObjectHandle runTemporaryLimitsCheck(BackendProvider* backend, ObjectHandle checked, int check, int side,
                                            const iidm::optional<double>& limitReductionValue, LimitType type) {
    HandleRef query(backend->invoke(checked, check), backend);
    backend->setInt(query, prop::TERMINAL_SIDE, side);
    if (limitReductionValue) backend->setDouble(query, prop::LIMIT_REDUCTION, *limitReductionValue);
    backend->setInt(query, prop::LL_LIMIT_TYPE, static_cast<int>(type));
    return backend->invoke(query, prop::ACT_ADD);
}

inline iidm::optional<Overload> overloadOf(BackendProvider* backend, ObjectHandle checked, int side,
                                           const iidm::optional<double>& limitReductionValue, LimitType type) {
    return wrapOptional<Overload>(
        runTemporaryLimitsCheck(backend, checked, prop::CHECK_TEMPORARY_LIMITS, side, limitReductionValue, type), backend);
}

inline std::vector<Overload> allOverloadsOf(BackendProvider* backend, ObjectHandle checked, int side,
                                            const iidm::optional<double>& limitReductionValue, LimitType type) {
    return elementsOf<Overload>(
        runTemporaryLimitsCheck(backend, checked, prop::CHECK_ALL_TEMPORARY_LIMITS, side, limitReductionValue, type), backend);
}

} } // namespace iidm::detail
