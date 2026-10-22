#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "MockBackend.h"
#include <utility>

// connect() and disconnect() of every connectable (and of a terminal) are one action on the Java
// side: invoke(object, ACT_CONNECT / ACT_DISCONNECT), which returns the object when this changed
// it (powsybl's boolean result).

using namespace iidm;
using namespace iidm::test;

namespace {

constexpr ObjectHandle COMPONENT = 10;

std::pair<ObjectHandle, int> call(ObjectHandle h, int code) {
    return std::make_pair(h, code);
}

template <class T>
void checkConnectAndDisconnect() {
    MockBackend b;
    T component(COMPONENT, &b);
    EXPECT_TRUE(component.disconnect());
    EXPECT_EQ(b.invocations.back(), call(COMPONENT, prop::ACT_DISCONNECT));
    EXPECT_TRUE(component.connect());
    EXPECT_EQ(b.invocations.back(), call(COMPONENT, prop::ACT_CONNECT));
    b.unchanged.insert(COMPONENT); // could not, or already was
    EXPECT_FALSE(component.connect());
    EXPECT_FALSE(component.disconnect());
    EXPECT_EQ(b.liveHandleCount(), 1u); // the result handles are released
}

} // namespace

TEST(ConnectTest, EveryConnectable) {
    checkConnectAndDisconnect<Generator>();
    checkConnectAndDisconnect<Load>();
    checkConnectAndDisconnect<Battery>();
    checkConnectAndDisconnect<BoundaryLine>();
    checkConnectAndDisconnect<ShuntCompensator>();
    checkConnectAndDisconnect<StaticVarCompensator>();
    checkConnectAndDisconnect<VscConverterStation>();
    checkConnectAndDisconnect<LccConverterStation>();
    checkConnectAndDisconnect<BusbarSection>();
    checkConnectAndDisconnect<Line>();
    checkConnectAndDisconnect<TwoWindingsTransformer>();
    checkConnectAndDisconnect<ThreeWindingsTransformer>();
    checkConnectAndDisconnect<Connectable>();
    checkConnectAndDisconnect<Terminal>();
}

TEST(ConnectTest, ThroughABaseClass) {
    MockBackend b;
    Generator generator(COMPONENT, &b);
    Connectable& connectable = generator;
    EXPECT_TRUE(connectable.disconnect());
    EXPECT_EQ(b.invocations.back(), call(COMPONENT, prop::ACT_DISCONNECT));
}

// With a switch predicate (and a side), connect and disconnect are a call: invoke(object, CONNECT /
// DISCONNECT), its fields SWITCH_PREDICATE and TERMINAL_SIDE (none for every side), then ACT_ADD,
// which returns the object when this changed it.
template <class Operation>
void checkSwitchPredicateCall(Operation operation, int code, SwitchPredicate switches, int side) {
    MockBackend b;
    const ObjectHandle callHandle = b.nextAdder;
    b.addResults[callHandle] = COMPONENT; // it changed
    EXPECT_TRUE(operation(b));
    ASSERT_EQ(b.invocations.size(), 2u);
    EXPECT_EQ(b.invocations[0], call(COMPONENT, code));
    EXPECT_EQ(b.invocations[1], call(callHandle, prop::ACT_ADD));
    EXPECT_EQ(b.ints.at({callHandle, prop::SWITCH_PREDICATE}), static_cast<int>(switches));
    if (side < 0) {
        EXPECT_EQ(b.ints.count({callHandle, prop::TERMINAL_SIDE}), 0u);
    } else {
        EXPECT_EQ(b.ints.at({callHandle, prop::TERMINAL_SIDE}), side);
    }
    EXPECT_EQ(b.liveHandleCount(), 0u); // the call and its result are released

    // Could not, or already was
    b.addResults.clear();
    EXPECT_FALSE(operation(b));
}

TEST(ConnectTest, WithASwitchPredicate) {
    checkSwitchPredicateCall([](MockBackend& b) {
        return Terminal(COMPONENT, &b).connect(SwitchPredicate::BREAKER_OR_DISCONNECTOR);
    }, prop::CONNECT, SwitchPredicate::BREAKER_OR_DISCONNECTOR, -1);
    checkSwitchPredicateCall([](MockBackend& b) {
        return Terminal(COMPONENT, &b).disconnect(SwitchPredicate::CLOSED_BREAKER);
    }, prop::DISCONNECT, SwitchPredicate::CLOSED_BREAKER, -1);
    checkSwitchPredicateCall([](MockBackend& b) {
        return Load(COMPONENT, &b).connect(SwitchPredicate::BREAKER);
    }, prop::CONNECT, SwitchPredicate::BREAKER, -1);
    checkSwitchPredicateCall([](MockBackend& b) {
        return ThreeWindingsTransformer(COMPONENT, &b).connect(SwitchPredicate::NON_NULL, ThreeSides::THREE);
    }, prop::CONNECT, SwitchPredicate::NON_NULL, 2);
    checkSwitchPredicateCall([](MockBackend& b) {
        return Line(COMPONENT, &b).disconnect(SwitchPredicate::NONFICTIONAL, ThreeSides::TWO);
    }, prop::DISCONNECT, SwitchPredicate::NONFICTIONAL, 1);
    checkSwitchPredicateCall([](MockBackend& b) {
        return HvdcLine(COMPONENT, &b).connectConverterStations(SwitchPredicate::OPEN_DISCONNECTOR);
    }, prop::CONNECT, SwitchPredicate::OPEN_DISCONNECTOR, -1);
    checkSwitchPredicateCall([](MockBackend& b) {
        return HvdcLine(COMPONENT, &b).disconnectConverterStations(SwitchPredicate::OPEN, TwoSides::TWO);
    }, prop::DISCONNECT, SwitchPredicate::OPEN, 1);
    checkSwitchPredicateCall([](MockBackend& b) {
        return TieLine(COMPONENT, &b).connectBoundaryLines(SwitchPredicate::NONFICTIONAL_BREAKER, TwoSides::ONE);
    }, prop::CONNECT, SwitchPredicate::NONFICTIONAL_BREAKER, 0);
    checkSwitchPredicateCall([](MockBackend& b) {
        return TieLine(COMPONENT, &b).disconnectBoundaryLines(SwitchPredicate::NONFICTIONAL_CLOSED_BREAKER);
    }, prop::DISCONNECT, SwitchPredicate::NONFICTIONAL_CLOSED_BREAKER, -1);
}

TEST(ConnectTest, NotBoundToANetwork) {
    EXPECT_THROW(Generator().connect(), InvalidHandleException);
    EXPECT_THROW(Line().disconnect(), InvalidHandleException);
    EXPECT_THROW(Terminal().connect(), InvalidHandleException);
    EXPECT_THROW(Terminal().connect(SwitchPredicate::BREAKER), InvalidHandleException);
    EXPECT_THROW(HvdcLine().disconnectConverterStations(SwitchPredicate::BREAKER, TwoSides::ONE), InvalidHandleException);
}
