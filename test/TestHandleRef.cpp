#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

namespace {

// Records the handles released by HandleRef.
class RecordingBackend : public MockBackend {
public:
    std::vector<ObjectHandle> released;
    void releaseHandle(ObjectHandle h) override { released.push_back(h); }
    void close() override { releaseAllHandles(); }
    long releases(ObjectHandle h) const { return std::count(released.begin(), released.end(), h); }
};

} // namespace

TEST(HandleRefTest, ReleasedWhenLastWrapperIsDestroyed) {
    RecordingBackend backend;
    {
        Generator g(10, &backend);
        EXPECT_EQ(backend.liveHandleCount(), 1u);
        Generator copy = g;
        Generator another(g);
        EXPECT_EQ(backend.liveHandleCount(), 1u);
    }
    EXPECT_EQ(backend.releases(10), 1);
    EXPECT_EQ(backend.liveHandleCount(), 0u);
}

TEST(HandleRefTest, ViewsShareTheHandle) {
    RecordingBackend backend;
    VoltageLevel::BusBreakerView view;
    {
        VoltageLevel vl(10, &backend);
        view = vl.getBusBreakerView();   // same handle as the voltage level
    }
    EXPECT_EQ(backend.releases(10), 0) << "still used by the view";
    view = VoltageLevel::BusBreakerView();
    EXPECT_EQ(backend.releases(10), 1);
}

TEST(HandleRefTest, ExtensionsHaveTheirOwnHandle) {
    RecordingBackend backend;
    backend.related[{10, prop::COORDINATED_REACTIVE_CONTROL}] = 11;
    {
        Generator g(10, &backend);
        CoordinatedReactiveControl crc = g.getCoordinatedReactiveControl();
        EXPECT_EQ(backend.liveHandleCount(), 2u);
    }
    EXPECT_EQ(backend.releases(10), 1);
    EXPECT_EQ(backend.releases(11), 1);
}

TEST(HandleRefTest, MovesDoNotReleaseTwice) {
    RecordingBackend backend;
    {
        Generator g(10, &backend);
        Generator moved = std::move(g);
        Generator assigned;
        assigned = std::move(moved);
        EXPECT_EQ(backend.releases(10), 0);
    }
    EXPECT_EQ(backend.releases(10), 1);
}

TEST(HandleRefTest, AssignmentReleasesThePreviousHandle) {
    RecordingBackend backend;
    Generator g(10, &backend);
    g = Generator(11, &backend);
    EXPECT_EQ(backend.releases(10), 1);
    EXPECT_EQ(backend.releases(11), 0);
}

TEST(HandleRefTest, ListsReleaseEveryElement) {
    RecordingBackend backend;
    backend.children[{1, prop::GENERATOR}] = {20, 21, 22};
    {
        auto gens = VoltageLevel(1, &backend).getGenerators();
        EXPECT_EQ(gens.size(), 3u);
        // the temporary voltage level is already released; the 3 generators are alive
        EXPECT_EQ(backend.releases(1), 1);
        EXPECT_EQ(backend.liveHandleCount(), 3u);
    }
    EXPECT_EQ(backend.releases(20) + backend.releases(21) + backend.releases(22), 3);
}

TEST(HandleRefTest, InvalidHandlesAreNotTracked) {
    RecordingBackend backend;
    { Generator g(INVALID_HANDLE, &backend); Generator d; }
    EXPECT_TRUE(backend.released.empty());
    EXPECT_EQ(backend.liveHandleCount(), 0u);
}

TEST(HandleRefTest, CloseReleasesOutstandingHandles) {
    RecordingBackend backend;
    Generator g(10, &backend);
    Load l(11, &backend);
    backend.close();
    EXPECT_EQ(backend.releases(10), 1);
    EXPECT_EQ(backend.releases(11), 1);
    backend.released.clear();
    // Destroying the wrappers afterwards releases nothing more
    g = Generator();
    l = Load();
    EXPECT_TRUE(backend.released.empty());
}

TEST(HandleRefTest, WrappersMayOutliveTheirBackend) {
    Generator survivor;
    {
        RecordingBackend backend;
        survivor = Generator(10, &backend);
    }
    // Destroying the wrapper after its backend must not touch the dead backend
    EXPECT_NO_THROW(survivor = Generator());
}
