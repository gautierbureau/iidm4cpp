package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.nio.file.Path;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * The Fortescue extensions, the asymmetrical load and the phase angle clocks through
 * {@link PropertyDispatcher}, on test/data/asymmetrical_test.xiidm: read, changed, created through
 * their adders (a three windings transformer's legs selected one after the other).
 */
class AsymmetricalExtensionDispatchTest {

    private Network network;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = com.powsybl.iidm.serde.NetworkSerDe.read(Path.of("../test/data/asymmetrical_test.xiidm"));
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    @Test void readAndChanged() {
        long g = PropertyDispatcher.getRelated(reg(network.getGenerator("G")), GENERATOR_FORTESCUE);
        assertEquals(1, PropertyDispatcher.getBool(g, EXT_GF_GROUNDED));
        assertEquals(3, PropertyDispatcher.getDouble(g, EXT_GF_XN));
        PropertyDispatcher.setDouble(g, EXT_GF_GROUNDING_X, 1.6);
        assertEquals(1.6, network.getGenerator("G").getExtension(GeneratorFortescue.class).getGroundingX());

        long l = PropertyDispatcher.getRelated(reg(network.getLine("L12")), LINE_FORTESCUE);
        assertEquals(1, PropertyDispatcher.getBool(l, EXT_LINE_F_OPEN_PHASE_B));
        assertEquals(0.002, PropertyDispatcher.getDouble(l, EXT_LINE_F_G2Z));

        long ld = PropertyDispatcher.getRelated(reg(network.getLoad("LD")), LOAD_ASYMMETRICAL);
        assertEquals(LoadConnectionType.DELTA.ordinal(), PropertyDispatcher.getInt(ld, EXT_LA_CONNECTION_TYPE));
        PropertyDispatcher.setInt(ld, EXT_LA_CONNECTION_TYPE, LoadConnectionType.Y.ordinal());
        assertEquals(LoadConnectionType.Y, network.getLoad("LD").getExtension(LoadAsymmetrical.class).getConnectionType());

        long t2 = reg(network.getTwoWindingsTransformer("T2"));
        long t2f = PropertyDispatcher.getRelated(t2, TWO_WINDINGS_TRANSFORMER_FORTESCUE);
        assertEquals(WindingConnectionType.Y_GROUNDED.ordinal(), PropertyDispatcher.getInt(t2f, EXT_TWO_WTF_CONNECTION_TYPE1));
        assertEquals(500, PropertyDispatcher.getDouble(t2f, EXT_TWO_WTF_XM));
        long clock = PropertyDispatcher.getRelated(t2, TWO_WINDINGS_TRANSFORMER_PHASE_ANGLE_CLOCK);
        assertEquals(11, PropertyDispatcher.getInt(clock, EXT_TWO_WT_PAC_CLOCK));
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.setInt(clock, EXT_TWO_WT_PAC_CLOCK, 12))));

        // The legs of a three windings transformer's
        long t3 = reg(network.getThreeWindingsTransformer("T3"));
        long t3f = PropertyDispatcher.getRelated(t3, THREE_WINDINGS_TRANSFORMER_FORTESCUE);
        long leg2 = PropertyDispatcher.getRelatedByIndex(t3f, LEG_FORTESCUE, ThreeSides.TWO.ordinal());
        assertEquals(1, PropertyDispatcher.getBool(leg2, LEG_FORTESCUE));
        assertEquals(22, PropertyDispatcher.getDouble(leg2, EXT_LEG_F_XZ));
        assertEquals(WindingConnectionType.Y.ordinal(), PropertyDispatcher.getInt(leg2, EXT_LEG_F_CONNECTION_TYPE));
        PropertyDispatcher.setBool(leg2, EXT_LEG_F_FREE_FLUXES, 1);
        assertTrue(network.getThreeWindingsTransformer("T3").getExtension(ThreeWindingsTransformerFortescue.class)
            .getLeg2().isFreeFluxes());
        long t3Clock = PropertyDispatcher.getRelated(t3, THREE_WINDINGS_TRANSFORMER_PHASE_ANGLE_CLOCK);
        assertEquals(7, PropertyDispatcher.getInt(t3Clock, EXT_THREE_WT_PAC_CLOCK_LEG3));
    }

    @Test void threeWindingsTransformerFortescueAdder() {
        ThreeWindingsTransformer t3 = network.getThreeWindingsTransformer("T3");
        t3.removeExtension(ThreeWindingsTransformerFortescue.class);
        long adder = PropertyDispatcher.invoke(reg(t3), THREE_WINDINGS_TRANSFORMER_FORTESCUE);
        PropertyDispatcher.setInt(adder, EXT_THREE_WTF_LEG, ThreeSides.ONE.ordinal());
        PropertyDispatcher.setDouble(adder, EXT_LEG_F_RZ, 1);
        PropertyDispatcher.setInt(adder, EXT_LEG_F_CONNECTION_TYPE, WindingConnectionType.DELTA.ordinal());
        PropertyDispatcher.setInt(adder, EXT_THREE_WTF_LEG, ThreeSides.THREE.ordinal());
        PropertyDispatcher.setDouble(adder, EXT_LEG_F_XZ, 3);
        PropertyDispatcher.setBool(adder, EXT_LEG_F_FREE_FLUXES, 1);
        long added = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(1, PropertyDispatcher.getBool(added, THREE_WINDINGS_TRANSFORMER_FORTESCUE));
        ThreeWindingsTransformerFortescue fortescue = t3.getExtension(ThreeWindingsTransformerFortescue.class);
        assertEquals(1, fortescue.getLeg1().getRz());
        assertEquals(WindingConnectionType.DELTA, fortescue.getLeg1().getConnectionType());
        assertEquals(3, fortescue.getLeg3().getXz());
        assertTrue(fortescue.getLeg3().isFreeFluxes());

        // A leg field before any leg is selected
        t3.removeExtension(ThreeWindingsTransformerFortescue.class);
        long another = PropertyDispatcher.invoke(reg(t3), THREE_WINDINGS_TRANSFORMER_FORTESCUE);
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.setDouble(another, EXT_LEG_F_RZ, 1))));
    }

    @Test void otherAdders() {
        Generator g = network.getGenerator("G");
        g.removeExtension(GeneratorFortescue.class);
        long adder = PropertyDispatcher.invoke(reg(g), GENERATOR_FORTESCUE);
        PropertyDispatcher.setBool(adder, EXT_GF_GROUNDED, 0);
        PropertyDispatcher.setDouble(adder, EXT_GF_RZ, 0.5);
        PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(0.5, g.getExtension(GeneratorFortescue.class).getRz());

        TwoWindingsTransformer t2 = network.getTwoWindingsTransformer("T2");
        t2.removeExtension(TwoWindingsTransformerFortescue.class);
        long t2Adder = PropertyDispatcher.invoke(reg(t2), TWO_WINDINGS_TRANSFORMER_FORTESCUE);
        PropertyDispatcher.setInt(t2Adder, EXT_TWO_WTF_CONNECTION_TYPE2, WindingConnectionType.Y.ordinal());
        PropertyDispatcher.setBool(t2Adder, EXT_TWO_WTF_FREE_FLUXES, 1);
        PropertyDispatcher.invoke(t2Adder, ACT_ADD);
        assertEquals(WindingConnectionType.Y, t2.getExtension(TwoWindingsTransformerFortescue.class).getConnectionType2());

        ThreeWindingsTransformer t3 = network.getThreeWindingsTransformer("T3");
        t3.removeExtension(ThreeWindingsTransformerPhaseAngleClock.class);
        long clockAdder = PropertyDispatcher.invoke(reg(t3), THREE_WINDINGS_TRANSFORMER_PHASE_ANGLE_CLOCK);
        PropertyDispatcher.setInt(clockAdder, EXT_THREE_WT_PAC_CLOCK_LEG2, 1);
        PropertyDispatcher.setInt(clockAdder, EXT_THREE_WT_PAC_CLOCK_LEG3, 2);
        PropertyDispatcher.invoke(clockAdder, ACT_ADD);
        assertEquals(2, t3.getExtension(ThreeWindingsTransformerPhaseAngleClock.class).getPhaseAngleClockLeg3());
    }
}
