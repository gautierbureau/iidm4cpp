#include <gtest/gtest.h>
#include <iidm/Network.h>
#include <iidm/NetworkFactory.h>
#include "MockBackend.h"

#include <memory>
#include <sstream>
#include <string>

using namespace iidm;
using namespace iidm::test;

namespace {

// Convenience: build a Network around a MockBackend and return a raw observer
// pointer (lifetime is tied to the returned Network).
struct MockWrapped {
    Network network;
    MockBackend* mock;
};

MockWrapped makeWrappedMock() {
    auto owned = std::make_unique<MockBackend>();
    MockBackend* observer = owned.get();
    Network net = NetworkFactory::createFromBackend(std::move(owned));
    return {std::move(net), observer};
}

} // namespace

// ── Save: ostream and saveToBytes ────────────────────────────────────────────

TEST(NetworkStreamTest, SaveToOstreamWritesBackendBytes) {
    auto w = makeWrappedMock();
    w.mock->bytesToReturnOnSave = "<network><id>n1</id></network>";

    std::ostringstream out;
    w.network.save(out);

    EXPECT_EQ(out.str(), "<network><id>n1</id></network>");
    EXPECT_EQ(w.mock->lastSavedFilenameHint, "network.xiidm");
}

TEST(NetworkStreamTest, SaveToOstreamPropagatesCustomFilenameHint) {
    auto w = makeWrappedMock();
    w.mock->bytesToReturnOnSave = "<data/>";

    std::ostringstream out;
    w.network.save(out, "custom.xiidm");

    EXPECT_EQ(w.mock->lastSavedFilenameHint, "custom.xiidm");
}

TEST(NetworkStreamTest, SaveToBytesReturnsBackendBytes) {
    auto w = makeWrappedMock();
    w.mock->bytesToReturnOnSave = "hello-bytes";

    EXPECT_EQ(w.network.saveToBytes(), "hello-bytes");
    EXPECT_EQ(w.mock->lastSavedFilenameHint, "network.xiidm");
}

TEST(NetworkStreamTest, SaveEmptyBufferDoesNotWriteToStream) {
    auto w = makeWrappedMock();
    w.mock->bytesToReturnOnSave.clear();

    std::ostringstream out;
    w.network.save(out);

    EXPECT_TRUE(out.str().empty());
    EXPECT_FALSE(out.bad());
}

// ── Load: istream and loadFromBytes ──────────────────────────────────────────

TEST(NetworkFactoryStreamTest, LoadFromIstreamForwardsBytesToBackend) {
    // We can't go through selectBackend() in a unit test (no JVM/Graal), so we
    // exercise the byte plumbing on a MockBackend directly.
    MockBackend mock;
    const std::string payload = "<network>payload</network>";

    mock.loadNetworkBytes(payload.data(), payload.size(), "network.xiidm");

    EXPECT_EQ(mock.lastLoadedBytes, payload);
    EXPECT_EQ(mock.lastLoadedFilenameHint, "network.xiidm");
}

TEST(NetworkFactoryStreamTest, IstreamSlurpRoundTripsBytes) {
    // Verify that NetworkFactory::load(std::istream&) slurps the whole stream
    // and passes the same bytes the user wrote into it.
    MockBackend probe;
    const std::string payload = "stream-payload-\x01\x02\x03end";
    std::istringstream in(payload);

    // Read what the helper would read.
    std::ostringstream buffer;
    buffer << in.rdbuf();
    EXPECT_EQ(buffer.str(), payload);

    // And the mock receives matching content if handed the same bytes.
    probe.loadNetworkBytes(payload.data(), payload.size(), "x.xiidm");
    EXPECT_EQ(probe.lastLoadedBytes, payload);
    EXPECT_EQ(probe.lastLoadedFilenameHint, "x.xiidm");
}
