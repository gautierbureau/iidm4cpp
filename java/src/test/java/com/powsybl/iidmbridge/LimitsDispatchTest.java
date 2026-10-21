package com.powsybl.iidmbridge;

import com.powsybl.commons.PowsyblException;
import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.util.List;
import java.util.Set;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Limits through {@link PropertyDispatcher}: the flows limits holders (a branch side, a 3WT leg, a
 * boundary line), their groups, the loading limits and their temporary limits, and their creation.
 */
class LimitsDispatchTest {

    private Network network;
    private Line line;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("limits", "test");
        Substation s = network.newSubstation().setId("S").add();
        VoltageLevel vl1 = s.newVoltageLevel().setId("VL1").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl1.getBusBreakerView().newBus().setId("B1").add();
        VoltageLevel vl2 = s.newVoltageLevel().setId("VL2").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl2.getBusBreakerView().newBus().setId("B2").add();
        line = network.newLine().setId("L")
            .setVoltageLevel1("VL1").setBus1("B1").setConnectableBus1("B1")
            .setVoltageLevel2("VL2").setBus2("B2").setConnectableBus2("B2")
            .setR(1).setX(10).setG1(0).setB1(0).setG2(0).setB2(0).add();
        OperationalLimitsGroup summer = line.newOperationalLimitsGroup1("SUMMER");
        summer.newCurrentLimits().setPermanentLimit(1000).setPermanentLimitName("IST")
            .beginTemporaryLimit().setName("TL20").setAcceptableDuration(20 * 60).setValue(1200).endTemporaryLimit()
            .beginTemporaryLimit().setName("TL1").setAcceptableDuration(60).setValue(1500).setFictitious(true).endTemporaryLimit()
            .add();
        summer.newActivePowerLimits().setPermanentLimit(700).add();
        line.newOperationalLimitsGroup1("WINTER").newCurrentLimits().setPermanentLimit(1100).add();
        line.setSelectedOperationalLimitsGroup1("SUMMER");
        line.newOperationalLimitsGroup2("DEFAULT").newApparentPowerLimits().setPermanentLimit(800).add();
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    private long side(int side) {
        return PropertyDispatcher.getRelatedByIndex(reg(line), FLOWS_LIMITS_HOLDER, side);
    }

    @Test void branchSidesAreHolders() {
        long side1 = side(0);
        assertEquals(1, PropertyDispatcher.getBool(side1, FLOWS_LIMITS_HOLDER));
        assertEquals(1, PropertyDispatcher.isSameObject(side1, side(0)));
        assertEquals(0, PropertyDispatcher.isSameObject(side1, side(1)));
        assertEquals(2, PropertyDispatcher.getChildren(side1, OPERATIONAL_LIMITS_GROUP).length);
        assertEquals("SUMMER", PropertyDispatcher.getString(PropertyDispatcher.getRelated(side1, REL_SELECTED_OLG), OLG_ID));
        assertEquals("SUMMER", PropertyDispatcher.getOptString(side1, SELECTED_OLG_ID));
        assertArrayEquals(new String[] {"SUMMER"}, PropertyDispatcher.getStringList(side1, ALL_SELECTED_OLG_IDS));
        assertEquals(1, PropertyDispatcher.getChildren(side1, REL_SELECTED_OLG).length);

        long winter = PropertyDispatcher.getRelatedByString(side1, OPERATIONAL_LIMITS_GROUP, "WINTER");
        assertSame(line.getOperationalLimitsGroup1("WINTER").orElseThrow(), NetworkRegistry.lookup(winter));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(side1, OPERATIONAL_LIMITS_GROUP, "UNKNOWN"));

        // Limits of the selected group, of every selected group, of a group
        long current = PropertyDispatcher.getRelated(side1, CURRENT_LIMITS);
        assertEquals(1000, PropertyDispatcher.getDouble(current, LL_PERMANENT_LIMIT));
        assertEquals(700, PropertyDispatcher.getDouble(PropertyDispatcher.getRelated(side1, ACTIVE_POWER_LIMITS), LL_PERMANENT_LIMIT));
        assertEquals(0L, PropertyDispatcher.getRelated(side1, APPARENT_POWER_LIMITS));
        assertEquals(1, PropertyDispatcher.getChildren(side1, CURRENT_LIMITS).length);
        long winterLimits = PropertyDispatcher.getRelatedByString(side1, CURRENT_LIMITS, "WINTER");
        assertEquals(1100, PropertyDispatcher.getDouble(winterLimits, LL_PERMANENT_LIMIT));
        assertEquals(1, PropertyDispatcher.isSameObject(current,
            PropertyDispatcher.getRelatedByIndex(side1, LOADING_LIMITS, LimitType.CURRENT.ordinal())));

        // Side 2: no group selected
        long side2 = side(1);
        assertEquals(0L, PropertyDispatcher.getRelated(side2, REL_SELECTED_OLG));
        assertNull(PropertyDispatcher.getOptString(side2, SELECTED_OLG_ID));
        assertThrows(BridgeErrors.PropertyNotFoundException.class, () -> PropertyDispatcher.getString(side2, SELECTED_OLG_ID));
        long apparent = PropertyDispatcher.getRelatedByString(side2, APPARENT_POWER_LIMITS, "DEFAULT");
        assertEquals(800, PropertyDispatcher.getDouble(apparent, LL_PERMANENT_LIMIT));
        assertEquals(1, PropertyDispatcher.getBool(apparent, APPARENT_POWER_LIMITS));
        assertEquals(0, PropertyDispatcher.getBool(apparent, CURRENT_LIMITS));
        assertEquals(1, PropertyDispatcher.getBool(apparent, LOADING_LIMITS));

        assertThrows(IllegalArgumentException.class, () -> side(2));
    }

    @Test void loadingLimits() {
        long limits = PropertyDispatcher.getRelated(side(0), CURRENT_LIMITS);
        CurrentLimits cl = line.getCurrentLimits1().orElseThrow();
        assertEquals(LimitType.CURRENT.ordinal(), PropertyDispatcher.getInt(limits, LL_LIMIT_TYPE));
        assertEquals(DetectionKind.HIGH.ordinal(), PropertyDispatcher.getInt(limits, LL_DETECTION_KIND));
        assertEquals("IST", PropertyDispatcher.getString(limits, LL_PERMANENT_LIMIT_NAME));
        PropertyDispatcher.setString(limits, LL_PERMANENT_LIMIT_NAME, "PATL");
        assertEquals("PATL", cl.getPermanentLimitName());
        PropertyDispatcher.setDouble(limits, LL_PERMANENT_LIMIT, 900);
        assertEquals(900, cl.getPermanentLimit());

        // Temporary limits: objects, by acceptable duration
        long[] temporary = PropertyDispatcher.getChildren(limits, TEMPORARY_LIMIT);
        assertEquals(2, temporary.length);
        long tl1 = PropertyDispatcher.getRelatedByIndex(limits, TEMPORARY_LIMIT, 60);
        assertEquals("TL1", PropertyDispatcher.getString(tl1, TL_NAME));
        assertEquals(1500, PropertyDispatcher.getDouble(tl1, TL_VALUE));
        assertEquals(60, PropertyDispatcher.getInt(tl1, TL_ACCEPTABLE_DURATION));
        assertEquals(1, PropertyDispatcher.getBool(tl1, TL_FICTITIOUS));
        assertEquals(1, PropertyDispatcher.getBool(tl1, TEMPORARY_LIMIT));
        assertEquals(0L, PropertyDispatcher.getRelatedByIndex(limits, TEMPORARY_LIMIT, 7));
        assertEquals(1200, PropertyDispatcher.evaluate(limits, LL_TEMPORARY_LIMIT_VALUE, new double[] {20 * 60}));
        assertTrue(Double.isNaN(PropertyDispatcher.evaluate(limits, LL_TEMPORARY_LIMIT_VALUE, new double[] {7})));

        // setTemporaryLimitValue(acceptableDuration, value) takes two values
        long change = PropertyDispatcher.invoke(limits, LL_TEMPORARY_LIMIT_VALUE);
        PropertyDispatcher.setInt(change, TL_ACCEPTABLE_DURATION, 60);
        PropertyDispatcher.setDouble(change, TL_VALUE, 1400);
        assertEquals(0L, PropertyDispatcher.invoke(change, ACT_ADD));
        assertEquals(1400, cl.getTemporaryLimitValue(60));
        // powsybl replaces the temporary limit: the one read before is not the limits' any more
        assertEquals(1500, PropertyDispatcher.getDouble(tl1, TL_VALUE));
        tl1 = PropertyDispatcher.getRelatedByIndex(limits, TEMPORARY_LIMIT, 60);
        assertEquals(1400, PropertyDispatcher.getDouble(tl1, TL_VALUE));

        // Properties of limits, of a temporary limit, of a group
        PropertyDispatcher.setKeyedString(limits, PROPERTY, "source", "rte");
        assertEquals("rte", cl.getProperty("source"));
        PropertyDispatcher.setKeyedString(tl1, PROPERTY, "note", "short");
        assertEquals("short", cl.getTemporaryLimit(60).getProperty("note"));
        long group = PropertyDispatcher.getRelated(side(0), REL_SELECTED_OLG);
        PropertyDispatcher.setKeyedString(group, PROPERTY, "season", "summer");
        assertArrayEquals(new String[] {"season"}, PropertyDispatcher.getStringList(group, PROPERTY_NAMES));

        // Removal of limits from their group
        long active = PropertyDispatcher.getRelated(group, ACTIVE_POWER_LIMITS);
        PropertyDispatcher.invoke(active, ACT_REMOVE);
        assertTrue(line.getOperationalLimitsGroup1("SUMMER").orElseThrow().getActivePowerLimits().isEmpty());
        PropertyDispatcher.setRelated(group, CURRENT_LIMITS, 0L);
        assertTrue(line.getOperationalLimitsGroup1("SUMMER").orElseThrow().getCurrentLimits().isEmpty());
    }

    @Test void groupSelection() {
        long side1 = side(0);
        PropertyDispatcher.setString(side1, SELECTED_OLG_ID, "WINTER");
        assertEquals("WINTER", line.getSelectedOperationalLimitsGroupId1().orElseThrow());
        PropertyDispatcher.setString(side1, ADD_SELECTED_OLG, "SUMMER");
        assertEquals(Set.of("SUMMER", "WINTER"), Set.copyOf(line.getAllSelectedOperationalLimitsGroupIds(TwoSides.ONE)));
        assertArrayEquals(new String[] {"SUMMER", "WINTER"}, PropertyDispatcher.getStringList(side1, ALL_SELECTED_OLG_IDS));
        assertEquals(line.getAllSelectedOperationalLimitsGroupIdsOrdered(TwoSides.ONE),
            List.of(PropertyDispatcher.getStringList(side1, ALL_SELECTED_OLG_IDS_ORDERED)));
        assertEquals(2, PropertyDispatcher.getChildren(side1, CURRENT_LIMITS).length);
        PropertyDispatcher.setString(side1, DESELECT_OLG, "WINTER");
        assertEquals(List.of("SUMMER"), List.copyOf(line.getAllSelectedOperationalLimitsGroupIds(TwoSides.ONE)));
        // Clearing the selected group deselects every group
        PropertyDispatcher.setRelated(side1, REL_SELECTED_OLG, 0L);
        assertTrue(line.getSelectedOperationalLimitsGroup1().isEmpty());
        assertThrows(PowsyblException.class, () -> PropertyDispatcher.setString(side1, SELECTED_OLG_ID, "UNKNOWN"));
    }

    @Test void groupCreationAndRemoval() {
        long side2 = side(1);
        long adder = PropertyDispatcher.invoke(side2, OPERATIONAL_LIMITS_GROUP);
        PropertyDispatcher.setString(adder, OLG_ID, "NEW");
        long group = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertSame(line.getOperationalLimitsGroup2("NEW").orElseThrow(), NetworkRegistry.lookup(group));
        assertEquals(1, PropertyDispatcher.getBool(group, OLG_EMPTY));

        // getOrCreateSelectedOperationalLimitsGroup: the selected one, else a new default group
        long selected = PropertyDispatcher.invoke(PropertyDispatcher.invoke(side2, REL_SELECTED_OLG), ACT_ADD);
        assertEquals(line.getSelectedOperationalLimitsGroupId2().orElseThrow(), PropertyDispatcher.getString(selected, OLG_ID));
        long chosen = PropertyDispatcher.invoke(side2, REL_SELECTED_OLG);
        PropertyDispatcher.setString(chosen, OLG_ID, "NEW");
        assertEquals(1, PropertyDispatcher.isSameObject(group, PropertyDispatcher.invoke(chosen, ACT_ADD)));
        assertEquals("NEW", line.getSelectedOperationalLimitsGroupId2().orElseThrow());

        PropertyDispatcher.setString(side2, REMOVE_OLG, "DEFAULT");
        assertTrue(line.getOperationalLimitsGroup2("DEFAULT").isEmpty());
    }

    @Test void limitsCreation() {
        long group = PropertyDispatcher.getRelatedByString(side(1), OPERATIONAL_LIMITS_GROUP, "DEFAULT");
        long adder = PropertyDispatcher.invoke(group, CURRENT_LIMITS);
        PropertyDispatcher.setDouble(adder, LL_PERMANENT_LIMIT, 950);
        PropertyDispatcher.setString(adder, LL_PERMANENT_LIMIT_NAME, "PATL");
        PropertyDispatcher.setInt(adder, LL_DETECTION_KIND, DetectionKind.HIGH.ordinal());
        PropertyDispatcher.setKeyedString(adder, PROPERTY, "source", "c++");
        for (int i = 0; i < 2; i++) {
            long temporary = PropertyDispatcher.invoke(adder, TEMPORARY_LIMIT);
            PropertyDispatcher.setString(temporary, TL_NAME, "TL");
            PropertyDispatcher.setBool(temporary, ENSURE_ID_UNICITY, 1); // a free name derived from TL
            PropertyDispatcher.setInt(temporary, TL_ACCEPTABLE_DURATION, 600 / (i + 1));
            PropertyDispatcher.setDouble(temporary, TL_VALUE, 1000 + 100 * i);
            PropertyDispatcher.setBool(temporary, TL_FICTITIOUS, i);
            PropertyDispatcher.setKeyedString(temporary, PROPERTY, "rank", Integer.toString(i));
            assertEquals(0L, PropertyDispatcher.invoke(temporary, ACT_ADD));
        }
        long limits = PropertyDispatcher.invoke(adder, ACT_ADD);
        CurrentLimits cl = line.getOperationalLimitsGroup2("DEFAULT").orElseThrow().getCurrentLimits().orElseThrow();
        assertSame(cl, NetworkRegistry.lookup(limits));
        assertEquals(950, cl.getPermanentLimit());
        assertEquals("PATL", cl.getPermanentLimitName());
        assertEquals("c++", cl.getProperty("source"));
        assertEquals(2, cl.getTemporaryLimits().size());
        assertEquals(1100, cl.getTemporaryLimit(300).getValue());
        assertTrue(cl.getTemporaryLimit(300).isFictitious());
        assertEquals("1", cl.getTemporaryLimit(300).getProperty("rank"));
        assertNotEquals(cl.getTemporaryLimit(600).getName(), cl.getTemporaryLimit(300).getName());
    }

    @Test void boundaryLinesAreHolders() {
        VoltageLevel vl = network.getVoltageLevel("VL1");
        BoundaryLine dl = vl.newBoundaryLine().setId("DL").setBus("B1").setConnectableBus("B1")
            .setR(1).setX(1).setG(0).setB(0).setP0(10).setQ0(5).add();
        dl.newOperationalLimitsGroup("G").newCurrentLimits().setPermanentLimit(300).add();
        dl.setSelectedOperationalLimitsGroup("G");
        long h = reg(dl);
        assertEquals(1, PropertyDispatcher.getBool(h, FLOWS_LIMITS_HOLDER));
        assertEquals(300, PropertyDispatcher.getDouble(PropertyDispatcher.getRelated(h, CURRENT_LIMITS), LL_PERMANENT_LIMIT));
        assertEquals("G", PropertyDispatcher.getString(h, SELECTED_OLG_ID));
        // A boundary line has no sides
        assertThrows(ClassCastException.class, () -> PropertyDispatcher.getRelatedByIndex(h, FLOWS_LIMITS_HOLDER, 0));
    }
}
