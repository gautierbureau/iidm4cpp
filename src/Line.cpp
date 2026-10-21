#include <iidm/Line.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/PropertyCodes.h>
#include "Extensions.h"

namespace iidm {

bool Line::hasLineFortescue() const {
    return detail::hasExtension<LineFortescue>(backend_, handle_);
}

LineFortescue Line::getLineFortescue() const {
    return detail::getExtension<LineFortescue>(backend_, handle_, "LineFortescue");
}

LineFortescueAdder Line::newLineFortescue() {
    return detail::newExtension<LineFortescue, LineFortescueAdder>(backend_, handle_);
}

void Line::removeLineFortescue() {
    detail::removeExtension<LineFortescue>(backend_, handle_);
}

bool Line::hasLinePosition() const {
    return detail::hasExtension<LinePosition>(backend_, handle_);
}

LinePosition Line::getLinePosition() const {
    return detail::getExtension<LinePosition>(backend_, handle_, "LinePosition");
}

LinePositionAdder Line::newLinePosition() {
    return detail::newExtension<LinePosition, LinePositionAdder>(backend_, handle_);
}

void Line::removeLinePosition() {
    detail::removeExtension<LinePosition>(backend_, handle_);
}

} // namespace iidm
