#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class BackendProvider;

// The SlackTerminal extension of a voltage level: an object of its own, from
// VoltageLevel::getSlackTerminal(). It holds one slack terminal per variant.
class SlackTerminal : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::SLACK_TERMINAL;

    SlackTerminal() = default;
    explicit SlackTerminal(ObjectHandle handle, BackendProvider* backend);

    // The slack terminal of the current variant; empty when it has none
    iidm::optional<Terminal> getTerminal() const;
    // The terminal must be in the voltage level (ValidationException otherwise); a
    // default-constructed Terminal unsets it. With cleanIfEmpty, the extension is then removed
    // when no variant has a slack terminal any more.
    SlackTerminal& setTerminal(const Terminal& terminal, bool cleanIfEmpty = false);
    // Whether no variant has a slack terminal
    bool isEmpty() const { return getBool(prop::EXT_ST_EMPTY); }
};

} // namespace iidm
