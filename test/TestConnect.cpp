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
    checkConnectAndDisconnect<DanglingLine>();
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

TEST(ConnectTest, NotBoundToANetwork) {
    EXPECT_THROW(Generator().connect(), InvalidHandleException);
    EXPECT_THROW(Line().disconnect(), InvalidHandleException);
    EXPECT_THROW(Terminal().connect(), InvalidHandleException);
}
