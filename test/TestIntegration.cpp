#include <gtest/gtest.h>
#include <iidm/NetworkFactory.h>
#include <iidm/IidmException.h>
#include <iidm/iidm.h>
#include <cstdlib>
#include <string>
#include <vector>

// Integration tests that load the real IEEE 14-bus network via the GraalVM
// backend.  They are skipped when the required environment variables are not
// set so that the regular mock-based CI jobs are not affected.
//
// Required env vars (set by the "integration" CI job):
//   IIDM_BRIDGE_GRAALVM_LIB   – full path to libiidm_bridge.so
//   IIDM_IEEE14_NETWORK_PATH  – full path to examples/IEEE14.xiidm

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED

using namespace iidm;

class IntegrationTest : public ::testing::Test {
protected:
    static const char* libPath() { return std::getenv("IIDM_BRIDGE_GRAALVM_LIB"); }
    static const char* netPath() { return std::getenv("IIDM_IEEE14_NETWORK_PATH"); }

    static bool available() {
        return libPath() && libPath()[0] && netPath() && netPath()[0];
    }

    Network loadNetwork() {
        NetworkLoadOptions opts;
        opts.mode           = BackendMode::GRAALVM;
        opts.graalvmLibPath = libPath();
        return NetworkFactory::load(netPath(), opts);
    }
};

TEST_F(IntegrationTest, NetworkIdAndElementCounts) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    EXPECT_EQ(net.getId(), "ieee14bus");
    EXPECT_EQ(net.getGenerators().size(), 5u);
    EXPECT_EQ(net.getLoads().size(),      11u);
    EXPECT_EQ(net.getLines().size(),      17u);
}

TEST_F(IntegrationTest, Generator1Properties) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    auto gen = net.getGenerator("_GEN____1_SM");
    ASSERT_TRUE(gen.has_value()) << "Generator _GEN____1_SM not found";
    EXPECT_NEAR(gen->getTargetP(), 232.3463, 1e-3);
    EXPECT_NEAR(gen->getMinP(),   -9999.0,   1e-1);
    EXPECT_NEAR(gen->getMaxP(),    9999.0,   1e-1);
    EXPECT_TRUE(gen->isVoltageRegulatorOn());
}

TEST_F(IntegrationTest, Load10Properties) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    auto load = net.getLoad("_LOAD__10_EC");
    ASSERT_TRUE(load.has_value()) << "Load _LOAD__10_EC not found";
    EXPECT_NEAR(load->getP0(), 9.0, 1e-6);
    EXPECT_NEAR(load->getQ0(), 5.8, 1e-6);
}

TEST_F(IntegrationTest, LineProperties) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    auto line = net.getLine("_BUS___10-BUS___11-1_AC");
    ASSERT_TRUE(line.has_value()) << "Line _BUS___10-BUS___11-1_AC not found";
    EXPECT_NEAR(line->getR(), 0.156256, 1e-6);
    EXPECT_NEAR(line->getX(), 0.365778, 1e-6);
}

TEST_F(IntegrationTest, SetGeneratorTargetP) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    auto gen = net.getGenerator("_GEN____2_SM");
    ASSERT_TRUE(gen.has_value());
    double original = gen->getTargetP();
    gen->setTargetP(original * 0.9);
    EXPECT_NEAR(gen->getTargetP(), original * 0.9, 1e-6);
}

TEST_F(IntegrationTest, SaveNetwork) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    const std::string outPath = "/tmp/ieee14_roundtrip.xiidm";
    ASSERT_NO_THROW(net.save(outPath));

    // Reload and verify identity is preserved
    Network reloaded = NetworkFactory::load(outPath,
        {BackendMode::GRAALVM, libPath(), ""});
    EXPECT_EQ(reloaded.getId(), net.getId());
}


// ── Dynawo-pattern tests ─────────────────────────────────────────────────────
//
// These tests simulate how Dynawo builds its ComponentInterface objects:
//   • traverse the network once to create interface structs
//   • each struct stores the iidm4cpp objects it needs BY VALUE (the fix)
//   • the source vector/scope is discarded after construction
//   • the stored handles are then exercised through real GraalVM dispatch
//
// Storing by value is safe because every iidm4cpp component is a 16-byte
// handle {ObjectHandle, BackendProvider*} whose data lives in the JVM.
// Storing by const-reference (the old pattern from the C++ IIDM tree) is
// NOT safe because getGenerators() / getVoltageLevels() etc. return fresh
// temporary vectors — the referenced elements are destroyed at end of scope.

namespace {

// Minimal replica of Dynawo's GeneratorInterfaceIIDM:
// stores every iidm4cpp object it needs as a value member.
struct GeneratorInterface {
    iidm::Generator    gen;
    iidm::Terminal     terminal;
    iidm::VoltageLevel voltageLevel;

    explicit GeneratorInterface(const iidm::Generator& g)
        : gen(g)
        , terminal(g.getTerminal())
        , voltageLevel(g.getTerminal().getVoltageLevel())
    {}

    double      getTargetP()       const { return gen.getTargetP(); }
    bool        isConnected()      const { return terminal.isConnected(); }
    std::string getVoltageLevelId()const { return voltageLevel.getId(); }
    double      getNominalV()      const { return voltageLevel.getNominalV(); }
};

// Minimal replica of Dynawo's LoadInterfaceIIDM.
struct LoadInterface {
    iidm::Load         load;
    iidm::Terminal     terminal;
    iidm::VoltageLevel voltageLevel;

    explicit LoadInterface(const iidm::Load& l)
        : load(l)
        , terminal(l.getTerminal())
        , voltageLevel(l.getTerminal().getVoltageLevel())
    {}

    double      getP0()            const { return load.getP0(); }
    double      getQ0()            const { return load.getQ0(); }
    bool        isConnected()      const { return terminal.isConnected(); }
    std::string getVoltageLevelId()const { return voltageLevel.getId(); }
};

// Minimal replica of Dynawo's VoltageLevelInterfaceIIDM.
// Stores the VoltageLevel (and its buses) by value.
struct VoltageLevelInterface {
    iidm::VoltageLevel vl;

    explicit VoltageLevelInterface(const iidm::VoltageLevel& v) : vl(v) {}

    std::string getId()      const { return vl.getId(); }
    double      getNominalV()const { return vl.getNominalV(); }
};

// Minimal replica of Dynawo's LineInterfaceIIDM.
struct LineInterface {
    iidm::Line         line;
    iidm::Terminal     terminal1;
    iidm::Terminal     terminal2;
    iidm::VoltageLevel vl1;
    iidm::VoltageLevel vl2;

    explicit LineInterface(const iidm::Line& l)
        : line(l)
        , terminal1(l.getTerminal1())
        , terminal2(l.getTerminal2())
        , vl1(l.getTerminal1().getVoltageLevel())
        , vl2(l.getTerminal2().getVoltageLevel())
    {}

    double getR() const { return line.getR(); }
    double getX() const { return line.getX(); }
    std::string getVl1Id() const { return vl1.getId(); }
    std::string getVl2Id() const { return vl2.getId(); }
};

// Minimal replica of Dynawo's TwoWTInterfaceIIDM.
struct TwoWTInterface {
    iidm::TwoWindingsTransformer twt;
    iidm::Terminal     terminal1;
    iidm::Terminal     terminal2;
    iidm::VoltageLevel vl1;
    iidm::VoltageLevel vl2;

    explicit TwoWTInterface(const iidm::TwoWindingsTransformer& t)
        : twt(t)
        , terminal1(t.getTerminal1())
        , terminal2(t.getTerminal2())
        , vl1(t.getTerminal1().getVoltageLevel())
        , vl2(t.getTerminal2().getVoltageLevel())
    {}

    double getR()         const { return twt.getR(); }
    double getRatedU1()   const { return twt.getRatedU1(); }
    double getRatedU2()   const { return twt.getRatedU2(); }
};

// Helper: build a vector of interface objects from a traversal, then let the
// source vector go out of scope before returning.  This is the exact pattern
// that caused the Dynawo lifetime bug when objects were stored by reference.
template<typename Itf, typename Src>
std::vector<Itf> buildInterfaces(const std::vector<Src>& src) {
    std::vector<Itf> interfaces;
    interfaces.reserve(src.size());
    for (const auto& s : src)
        interfaces.emplace_back(s);
    return interfaces;
}

} // anonymous namespace

// ── DynawoPattern: GeneratorInterface ────────────────────────────────────────

TEST_F(IntegrationTest, DynawoPattern_GeneratorInterface) {
    if (!available()) GTEST_SKIP() << "GraalVM env vars not set";

    Network net = loadNetwork();

    // Build all generator interfaces, then destroy the source vector
    std::vector<GeneratorInterface> interfaces;
    {
        auto gens = net.getGenerators();   // temporary vector of value-type handles
        interfaces = buildInterfaces<GeneratorInterface>(gens);
    }   // gens destroyed here — interfaces must remain fully valid

    ASSERT_EQ(interfaces.size(), 5u);
    for (const auto& itf : interfaces) {
        EXPECT_TRUE(itf.gen.isValid());
        EXPECT_TRUE(itf.terminal.isValid());
        EXPECT_TRUE(itf.voltageLevel.isValid());

        // All calls go through GraalVM dispatch on the stored handles
        EXPECT_NO_THROW(itf.getTargetP());
        EXPECT_FALSE(itf.getVoltageLevelId().empty());
        EXPECT_GT(itf.getNominalV(), 0.0);
    }

    // Spot-check a specific generator known from IEEE14
    const auto* gen1 = [&]() -> const GeneratorInterface* {
        for (const auto& i : interfaces)
            if (i.gen.getId() == "_GEN____1_SM") return &i;
        return nullptr;
    }();
    ASSERT_NE(gen1, nullptr);
    EXPECT_NEAR(gen1->getTargetP(), 232.3463, 1e-3);
    EXPECT_TRUE(gen1->isConnected());
}

// ── DynawoPattern: LoadInterface ─────────────────────────────────────────────

TEST_F(IntegrationTest, DynawoPattern_LoadInterface) {
    if (!available()) GTEST_SKIP() << "GraalVM env vars not set";

    Network net = loadNetwork();

    std::vector<LoadInterface> interfaces;
    {
        auto loads = net.getLoads();
        interfaces = buildInterfaces<LoadInterface>(loads);
    }

    ASSERT_EQ(interfaces.size(), 11u);
    for (const auto& itf : interfaces) {
        EXPECT_TRUE(itf.load.isValid());
        EXPECT_TRUE(itf.terminal.isValid());
        EXPECT_TRUE(itf.voltageLevel.isValid());
        EXPECT_NO_THROW(itf.getP0());
        EXPECT_FALSE(itf.getVoltageLevelId().empty());
    }

    // Spot-check load 10
    const auto* load10 = [&]() -> const LoadInterface* {
        for (const auto& i : interfaces)
            if (i.load.getId() == "_LOAD__10_EC") return &i;
        return nullptr;
    }();
    ASSERT_NE(load10, nullptr);
    EXPECT_NEAR(load10->getP0(), 9.0, 1e-6);
    EXPECT_NEAR(load10->getQ0(), 5.8, 1e-6);
    EXPECT_TRUE(load10->isConnected());
}

// ── DynawoPattern: VoltageLevelInterface ─────────────────────────────────────

TEST_F(IntegrationTest, DynawoPattern_VoltageLevelInterface) {
    if (!available()) GTEST_SKIP() << "GraalVM env vars not set";

    Network net = loadNetwork();

    std::vector<VoltageLevelInterface> interfaces;
    {
        auto vls = net.getVoltageLevels();
        interfaces = buildInterfaces<VoltageLevelInterface>(vls);
    }

    ASSERT_FALSE(interfaces.empty());
    for (const auto& itf : interfaces) {
        EXPECT_TRUE(itf.vl.isValid());
        EXPECT_FALSE(itf.getId().empty());
        EXPECT_GT(itf.getNominalV(), 0.0);
    }
}

// ── DynawoPattern: LineInterface ─────────────────────────────────────────────

TEST_F(IntegrationTest, DynawoPattern_LineInterface) {
    if (!available()) GTEST_SKIP() << "GraalVM env vars not set";

    Network net = loadNetwork();

    std::vector<LineInterface> interfaces;
    {
        auto lines = net.getLines();
        interfaces = buildInterfaces<LineInterface>(lines);
    }

    ASSERT_EQ(interfaces.size(), 17u);
    for (const auto& itf : interfaces) {
        EXPECT_TRUE(itf.line.isValid());
        EXPECT_TRUE(itf.terminal1.isValid());
        EXPECT_TRUE(itf.terminal2.isValid());
        EXPECT_TRUE(itf.vl1.isValid());
        EXPECT_TRUE(itf.vl2.isValid());
        EXPECT_NO_THROW(itf.getR());
        EXPECT_NO_THROW(itf.getX());
        EXPECT_FALSE(itf.getVl1Id().empty());
        EXPECT_FALSE(itf.getVl2Id().empty());
    }

    // Spot-check a known line
    const auto* line = [&]() -> const LineInterface* {
        for (const auto& i : interfaces)
            if (i.line.getId() == "_BUS___10-BUS___11-1_AC") return &i;
        return nullptr;
    }();
    ASSERT_NE(line, nullptr);
    EXPECT_NEAR(line->getR(), 0.156256, 1e-6);
    EXPECT_NEAR(line->getX(), 0.365778, 1e-6);
}

// ── DynawoPattern: TwoWindingsTransformerInterface ───────────────────────────

TEST_F(IntegrationTest, DynawoPattern_TwoWTInterface) {
    if (!available()) GTEST_SKIP() << "GraalVM env vars not set";

    Network net = loadNetwork();

    std::vector<TwoWTInterface> interfaces;
    {
        auto twts = net.getTwoWindingsTransformers();
        interfaces = buildInterfaces<TwoWTInterface>(twts);
    }

    ASSERT_FALSE(interfaces.empty());
    for (const auto& itf : interfaces) {
        EXPECT_TRUE(itf.twt.isValid());
        EXPECT_TRUE(itf.terminal1.isValid());
        EXPECT_TRUE(itf.terminal2.isValid());
        EXPECT_TRUE(itf.vl1.isValid());
        EXPECT_TRUE(itf.vl2.isValid());
        EXPECT_NO_THROW(itf.getR());
        EXPECT_GT(itf.getRatedU1(), 0.0);
        EXPECT_GT(itf.getRatedU2(), 0.0);
    }
}

// ── DynawoPattern: LCC/VSC (conditional — only if network contains them) ─────
//
// The IEEE-14 network has no HVDC; these tests validate that LCC_LOSS_FACTOR
// and VSC_LOSS_FACTOR dispatch correctly when LCC/VSC elements are present.
// They are skipped gracefully on IEEE-14 but will run on any network that
// contains converter stations.

TEST_F(IntegrationTest, DynawoPattern_LccLossFactor) {
    if (!available()) GTEST_SKIP() << "GraalVM env vars not set";

    Network net = loadNetwork();
    auto lccs = net.getLccConverterStations();
    if (lccs.empty()) GTEST_SKIP() << "Network contains no LCC converter stations";

    // Store by value — simulating a fixed LccConverterStationInterfaceIIDM
    struct LccInterface {
        iidm::LccConverterStation lcc;
        iidm::Terminal terminal;
        iidm::VoltageLevel voltageLevel;
        explicit LccInterface(const iidm::LccConverterStation& s)
            : lcc(s), terminal(s.getTerminal())
            , voltageLevel(s.getTerminal().getVoltageLevel()) {}
        float getLossFactor()  const { return lcc.getLossFactor(); }
        float getPowerFactor() const { return lcc.getPowerFactor(); }
    };

    std::vector<LccInterface> interfaces;
    for (const auto& l : lccs) interfaces.emplace_back(l);
    lccs.clear(); // explicitly discard source

    for (const auto& itf : interfaces) {
        EXPECT_TRUE(itf.lcc.isValid());
        EXPECT_NO_THROW(itf.getLossFactor());
        EXPECT_NO_THROW(itf.getPowerFactor());
        EXPECT_GE(itf.getLossFactor(), 0.0f);
        EXPECT_GE(itf.getPowerFactor(), 0.0f);
    }
}

TEST_F(IntegrationTest, DynawoPattern_VscLossFactor) {
    if (!available()) GTEST_SKIP() << "GraalVM env vars not set";

    Network net = loadNetwork();
    auto vscs = net.getVscConverterStations();
    if (vscs.empty()) GTEST_SKIP() << "Network contains no VSC converter stations";

    struct VscInterface {
        iidm::VscConverterStation vsc;
        iidm::Terminal terminal;
        iidm::VoltageLevel voltageLevel;
        explicit VscInterface(const iidm::VscConverterStation& s)
            : vsc(s), terminal(s.getTerminal())
            , voltageLevel(s.getTerminal().getVoltageLevel()) {}
        float getLossFactor() const { return vsc.getLossFactor(); }
    };

    std::vector<VscInterface> interfaces;
    for (const auto& v : vscs) interfaces.emplace_back(v);
    vscs.clear();

    for (const auto& itf : interfaces) {
        EXPECT_TRUE(itf.vsc.isValid());
        EXPECT_NO_THROW(itf.getLossFactor());
        EXPECT_GE(itf.getLossFactor(), 0.0f);
    }
}

#endif // IIDM_BRIDGE_GRAALVM_ENABLED
