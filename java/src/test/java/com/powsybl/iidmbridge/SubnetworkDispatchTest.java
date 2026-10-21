package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import com.powsybl.iidmbridge.jni.IidmBridgeRegistry;
import org.junit.jupiter.api.*;

import java.io.ByteArrayInputStream;
import java.util.Arrays;
import java.util.List;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Subnetworks through {@link PropertyDispatcher}: creation, lookups in a subnetwork, boundary
 * elements, detaching and flattening; saving a subnetwork or a detached network.
 */
class SubnetworkDispatchTest {

    private Network root;
    private long sub1;
    private long sub2;

    /** SUB1 (VL1, with the unpaired boundary line BL1) and SUB2 (VL2), linked by the line L12. */
    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        root = Network.create("root", "test");
        sub1 = newSubnetwork("SUB1", "first");
        sub2 = newSubnetwork("SUB2", "second");
        VoltageLevel vl1 = newVoltageLevel((Network) obj(sub1), "S1", "VL1");
        newVoltageLevel((Network) obj(sub2), "S2", "VL2");
        root.newLine().setId("L12").setVoltageLevel1("VL1").setBus1("B_VL1").setVoltageLevel2("VL2").setBus2("B_VL2")
            .setR(1).setX(10).setG1(0).setB1(0).setG2(0).setB2(0).add();
        vl1.newBoundaryLine().setId("BL1").setBus("B_VL1").setP0(0).setQ0(0).setR(1).setX(1).setG(0).setB(0)
            .setPairingKey("X").add();
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    private static Object obj(long handle) {
        return NetworkRegistry.lookup(handle);
    }

    private static List<String> ids(long[] handles) {
        return Arrays.stream(handles).mapToObj(h -> PropertyDispatcher.getString(h, ID)).toList();
    }

    private long newSubnetwork(String id, String name) {
        long pending = PropertyDispatcher.invoke(reg(root), NETWORK);
        PropertyDispatcher.setString(pending, ID, id);
        PropertyDispatcher.setString(pending, NAME, name);
        PropertyDispatcher.setString(pending, NETWORK_SOURCE_FORMAT, "fmt");
        return PropertyDispatcher.invoke(pending, ACT_ADD);
    }

    private static VoltageLevel newVoltageLevel(Network network, String substationId, String id) {
        VoltageLevel vl = network.newSubstation().setId(substationId).add()
            .newVoltageLevel().setId(id).setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl.getBusBreakerView().newBus().setId("B_" + id).add();
        vl.newLoad().setId("LD_" + id).setBus("B_" + id).setP0(10).setQ0(1).add();
        return vl;
    }

    private static boolean isBoundaryElement(long network, Object identifiable) {
        long check = PropertyDispatcher.invoke(network, BOUNDARY_ELEMENT);
        PropertyDispatcher.setRelated(check, IDENTIFIABLE, reg(identifiable));
        return PropertyDispatcher.invoke(check, ACT_ADD) != 0L;
    }

    @Test void subnetworks() {
        long r = reg(root);
        assertEquals(List.of("SUB1", "SUB2"), ids(PropertyDispatcher.getChildren(r, NETWORK)));
        assertSame(obj(sub2), obj(PropertyDispatcher.getRelatedByString(r, NETWORK, "SUB2")));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(r, NETWORK, "NONE"));
        assertEquals("first", PropertyDispatcher.getString(sub1, NAME));
        assertEquals("fmt", PropertyDispatcher.getString(sub1, NETWORK_SOURCE_FORMAT));
        assertEquals(0, PropertyDispatcher.getChildren(sub1, NETWORK).length);
        assertSame(root, obj(PropertyDispatcher.getRelated(sub1, REL_PARENT_NETWORK)));
        assertSame(obj(sub1), obj(PropertyDispatcher.getRelated(PropertyDispatcher.findById(r, LOAD, "LD_VL1"), REL_PARENT_NETWORK)));

        // Lookups by id in a subnetwork: its own objects only (the line between them is the root's)
        assertNotEquals(0L, PropertyDispatcher.getRelatedByString(sub1, LOAD, "LD_VL1"));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(sub1, LOAD, "LD_VL2"));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(sub1, LINE, "L12"));
        assertNotEquals(0L, PropertyDispatcher.getRelatedByString(r, LINE, "L12"));
        assertNotEquals(0L, PropertyDispatcher.getRelatedByString(r, REL_BBV_BUS, "B_VL2"));
        assertEquals(0, PropertyDispatcher.getChildren(sub1, LINE).length);

        // Boundary elements: the line to SUB2 and the unpaired boundary line for SUB1, sorted
        assertEquals(List.of("BL1", "L12"), ids(PropertyDispatcher.getChildren(sub1, BOUNDARY_ELEMENT)));
        assertEquals(List.of("BL1"), ids(PropertyDispatcher.getChildren(r, BOUNDARY_ELEMENT)));
        assertTrue(isBoundaryElement(sub1, root.getLine("L12")));
        assertFalse(isBoundaryElement(sub1, root.getLoad("LD_VL1")));
        assertFalse(isBoundaryElement(r, root.getLine("L12")));
    }

    @Test void detachAndFlatten() {
        // The line cannot be split
        assertEquals(0, PropertyDispatcher.getBool(sub1, NETWORK_DETACHABLE));
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(sub1, ACT_DETACH))));
        root.getLine("L12").remove();
        assertEquals(1, PropertyDispatcher.getBool(sub1, NETWORK_DETACHABLE));
        long detached = PropertyDispatcher.invoke(sub1, ACT_DETACH);
        assertEquals("SUB1", PropertyDispatcher.getString(detached, ID));
        assertNotEquals(0L, PropertyDispatcher.getRelatedByString(detached, LOAD, "LD_VL1"));
        assertEquals(List.of("SUB2"), ids(PropertyDispatcher.getChildren(reg(root), NETWORK)));
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(reg(root), ACT_DETACH))));

        // Saved alone
        Network saved = com.powsybl.iidm.serde.NetworkSerDe.read(
            new ByteArrayInputStream(IidmBridgeRegistry.saveToBytes((Network) obj(detached), "network.xiidm")));
        assertEquals("SUB1", saved.getId());
        assertEquals(1, saved.getLoadCount());

        // Flattened: SUB2's objects are the root's
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(sub2, ACT_FLATTEN))));
        assertEquals(0L, PropertyDispatcher.invoke(reg(root), ACT_FLATTEN));
        assertEquals(0, PropertyDispatcher.getChildren(reg(root), NETWORK).length);
        assertSame(root, root.getLoad("LD_VL2").getParentNetwork());
    }
}
