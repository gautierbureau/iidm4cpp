#include <iidm/Line.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/PropertyCodes.h>
#include "Extensions.h"

namespace iidm {

double Line::getR() const {
    return backend_->getDouble(handle_, prop::LINE_R);
}

Line& Line::setR(double r) {
    backend_->setDouble(handle_, prop::LINE_R, r);
    return *this;
}

double Line::getX() const {
    return backend_->getDouble(handle_, prop::LINE_X);
}

Line& Line::setX(double x) {
    backend_->setDouble(handle_, prop::LINE_X, x);
    return *this;
}

double Line::getG1() const {
    return backend_->getDouble(handle_, prop::LINE_G1);
}

Line& Line::setG1(double g1) {
    backend_->setDouble(handle_, prop::LINE_G1, g1);
    return *this;
}

double Line::getB1() const {
    return backend_->getDouble(handle_, prop::LINE_B1);
}

Line& Line::setB1(double b1) {
    backend_->setDouble(handle_, prop::LINE_B1, b1);
    return *this;
}

double Line::getG2() const {
    return backend_->getDouble(handle_, prop::LINE_G2);
}

Line& Line::setG2(double g2) {
    backend_->setDouble(handle_, prop::LINE_G2, g2);
    return *this;
}

double Line::getB2() const {
    return backend_->getDouble(handle_, prop::LINE_B2);
}

Line& Line::setB2(double b2) {
    backend_->setDouble(handle_, prop::LINE_B2, b2);
    return *this;
}

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
