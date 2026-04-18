package com.powsybl.iidmbridge;

/**
 * Property and object-type codes shared between the C++ header PropertyCodes.h
 * and the Java PropertyDispatcher.
 *
 * IMPORTANT: Keep in sync with include/iidm/PropertyCodes.h.
 */
public final class PropertyCodes {
    private PropertyCodes() {}

    // ── Object types ──────────────────────────────────────────────────────
    public static final int SUBSTATION                 = 1;
    public static final int VOLTAGE_LEVEL              = 2;
    public static final int GENERATOR                  = 3;
    public static final int LOAD                       = 4;
    public static final int LINE                       = 5;
    public static final int TWO_WINDINGS_TRANSFORMER   = 6;
    public static final int THREE_WINDINGS_TRANSFORMER = 7;
    public static final int HVDC_LINE                  = 8;
    public static final int DANGLING_LINE              = 9;
    public static final int SHUNT_COMPENSATOR          = 10;
    public static final int STATIC_VAR_COMPENSATOR     = 11;
    public static final int VSC_CONVERTER_STATION      = 12;
    public static final int LCC_CONVERTER_STATION      = 13;
    public static final int TERMINAL                   = 14;
    public static final int BUS                        = 15;
    public static final int SWITCH                     = 16;
    public static final int BUSBAR_SECTION             = 17;
    public static final int BATTERY                    = 18;
    public static final int REACTIVE_CURVE_POINT       = 19;
    public static final int TWO_WT_RTC_STEP            = 20;
    public static final int TWO_WT_PTC_STEP            = 21;
    public static final int THREE_WT_LEG1_RTC_STEP     = 22;
    public static final int THREE_WT_LEG2_RTC_STEP     = 23;
    public static final int THREE_WT_LEG3_RTC_STEP     = 24;
    public static final int THREE_WT_LEG1_PTC_STEP     = 25;
    public static final int THREE_WT_LEG2_PTC_STEP     = 26;
    public static final int THREE_WT_LEG3_PTC_STEP     = 27;
    public static final int SHUNT_SECTION               = 28;
    public static final int INTERNAL_CONNECTION         = 29;

    // ── Shared properties ─────────────────────────────────────────────────
    public static final int ID      = 100;
    public static final int NAME    = 101;
    public static final int COUNTRY = 102;

    // ── Generator ────────────────────────────────────────────────────────
    public static final int GEN_TARGET_P             = 200;
    public static final int GEN_TARGET_Q             = 201;
    public static final int GEN_TARGET_V             = 202;
    public static final int GEN_MIN_P                = 203;
    public static final int GEN_MAX_P                = 204;
    public static final int GEN_VOLTAGE_REGULATOR_ON = 205;
    public static final int GEN_ENERGY_SOURCE        = 206;
    public static final int GEN_REACTIVE_LIMITS_KIND = 207; // 0=NONE 1=MIN_MAX 2=CURVE
    public static final int GEN_MIN_Q                = 208;
    public static final int GEN_MAX_Q                = 209;

    // ── Load ─────────────────────────────────────────────────────────────
    public static final int LOAD_P0   = 300;
    public static final int LOAD_Q0   = 301;
    public static final int LOAD_TYPE = 302;

    // ── Terminal ─────────────────────────────────────────────────────────
    public static final int TERMINAL_P         = 400;
    public static final int TERMINAL_Q         = 401;
    public static final int TERMINAL_CONNECTED = 402;
    public static final int TERMINAL_BUS_ID    = 403;
    public static final int TERMINAL_NODE      = 404;

    // ── Bus ──────────────────────────────────────────────────────────────
    public static final int BUS_V     = 500;
    public static final int BUS_ANGLE = 501;

    // ── VoltageLevel ─────────────────────────────────────────────────────
    public static final int VL_NOMINAL_V          = 600;
    public static final int VL_LOW_VOLTAGE_LIMIT  = 601;
    public static final int VL_HIGH_VOLTAGE_LIMIT = 602;
    public static final int VL_TOPOLOGY_KIND      = 603;

    // ── Line ─────────────────────────────────────────────────────────────
    public static final int LINE_R  = 700;
    public static final int LINE_X  = 701;
    public static final int LINE_G1 = 702;
    public static final int LINE_B1 = 703;
    public static final int LINE_G2 = 704;
    public static final int LINE_B2 = 705;

    // ── TwoWindingsTransformer ────────────────────────────────────────────
    public static final int TWO_WT_R        = 800;
    public static final int TWO_WT_X        = 801;
    public static final int TWO_WT_G        = 802;
    public static final int TWO_WT_B        = 803;
    public static final int TWO_WT_RATED_U1 = 804;
    public static final int TWO_WT_RATED_U2 = 805;
    public static final int TWO_WT_RATED_S         = 806;
    public static final int TWO_WT_RTC_EXISTS      = 807;
    public static final int TWO_WT_RTC_TAP_POSITION = 808;
    public static final int TWO_WT_RTC_LOW_TAP     = 809;
    public static final int TWO_WT_RTC_HIGH_TAP    = 810;
    public static final int TWO_WT_RTC_REGULATING  = 811;
    public static final int TWO_WT_RTC_TARGET_V    = 812;
    public static final int TWO_WT_PTC_EXISTS      = 813;
    public static final int TWO_WT_PTC_TAP_POSITION = 814;
    public static final int TWO_WT_PTC_LOW_TAP     = 815;
    public static final int TWO_WT_PTC_HIGH_TAP    = 816;
    public static final int TWO_WT_PTC_REGULATING  = 817;
    public static final int TWO_WT_PTC_REG_MODE    = 818;
    public static final int TWO_WT_PTC_REG_VALUE   = 819;

    // ── ThreeWindingsTransformer legs ─────────────────────────────────────────
    public static final int THREE_WT_LEG1_BASE = 820;
    public static final int THREE_WT_LEG2_BASE = 840;
    public static final int THREE_WT_LEG3_BASE = 860;
    // Leg 1
    public static final int THREE_WT_LEG1_R              = 820;
    public static final int THREE_WT_LEG1_X              = 821;
    public static final int THREE_WT_LEG1_G              = 822;
    public static final int THREE_WT_LEG1_B              = 823;
    public static final int THREE_WT_LEG1_RATED_U        = 824;
    public static final int THREE_WT_LEG1_RATED_S        = 825;
    public static final int THREE_WT_LEG1_RTC_EXISTS     = 826;
    public static final int THREE_WT_LEG1_RTC_TAP_POS    = 827;
    public static final int THREE_WT_LEG1_RTC_LOW_TAP    = 828;
    public static final int THREE_WT_LEG1_RTC_HIGH_TAP   = 829;
    public static final int THREE_WT_LEG1_RTC_REGULATING = 830;
    public static final int THREE_WT_LEG1_RTC_TARGET_V   = 831;
    public static final int THREE_WT_LEG1_PTC_EXISTS     = 832;
    public static final int THREE_WT_LEG1_PTC_TAP_POS    = 833;
    public static final int THREE_WT_LEG1_PTC_LOW_TAP    = 834;
    public static final int THREE_WT_LEG1_PTC_HIGH_TAP   = 835;
    public static final int THREE_WT_LEG1_PTC_REGULATING = 836;
    public static final int THREE_WT_LEG1_PTC_REG_MODE   = 837;
    public static final int THREE_WT_LEG1_PTC_REG_VALUE  = 838;
    // Leg 2
    public static final int THREE_WT_LEG2_R              = 840;
    public static final int THREE_WT_LEG2_X              = 841;
    public static final int THREE_WT_LEG2_G              = 842;
    public static final int THREE_WT_LEG2_B              = 843;
    public static final int THREE_WT_LEG2_RATED_U        = 844;
    public static final int THREE_WT_LEG2_RATED_S        = 845;
    public static final int THREE_WT_LEG2_RTC_EXISTS     = 846;
    public static final int THREE_WT_LEG2_RTC_TAP_POS    = 847;
    public static final int THREE_WT_LEG2_RTC_LOW_TAP    = 848;
    public static final int THREE_WT_LEG2_RTC_HIGH_TAP   = 849;
    public static final int THREE_WT_LEG2_RTC_REGULATING = 850;
    public static final int THREE_WT_LEG2_RTC_TARGET_V   = 851;
    public static final int THREE_WT_LEG2_PTC_EXISTS     = 852;
    public static final int THREE_WT_LEG2_PTC_TAP_POS    = 853;
    public static final int THREE_WT_LEG2_PTC_LOW_TAP    = 854;
    public static final int THREE_WT_LEG2_PTC_HIGH_TAP   = 855;
    public static final int THREE_WT_LEG2_PTC_REGULATING = 856;
    public static final int THREE_WT_LEG2_PTC_REG_MODE   = 857;
    public static final int THREE_WT_LEG2_PTC_REG_VALUE  = 858;
    // Leg 3
    public static final int THREE_WT_LEG3_R              = 860;
    public static final int THREE_WT_LEG3_X              = 861;
    public static final int THREE_WT_LEG3_G              = 862;
    public static final int THREE_WT_LEG3_B              = 863;
    public static final int THREE_WT_LEG3_RATED_U        = 864;
    public static final int THREE_WT_LEG3_RATED_S        = 865;
    public static final int THREE_WT_LEG3_RTC_EXISTS     = 866;
    public static final int THREE_WT_LEG3_RTC_TAP_POS    = 867;
    public static final int THREE_WT_LEG3_RTC_LOW_TAP    = 868;
    public static final int THREE_WT_LEG3_RTC_HIGH_TAP   = 869;
    public static final int THREE_WT_LEG3_RTC_REGULATING = 870;
    public static final int THREE_WT_LEG3_RTC_TARGET_V   = 871;
    public static final int THREE_WT_LEG3_PTC_EXISTS     = 872;
    public static final int THREE_WT_LEG3_PTC_TAP_POS    = 873;
    public static final int THREE_WT_LEG3_PTC_LOW_TAP    = 874;
    public static final int THREE_WT_LEG3_PTC_HIGH_TAP   = 875;
    public static final int THREE_WT_LEG3_PTC_REGULATING = 876;
    public static final int THREE_WT_LEG3_PTC_REG_MODE   = 877;
    public static final int THREE_WT_LEG3_PTC_REG_VALUE  = 878;

    // ── TapChangerStep properties ─────────────────────────────────────────────
    public static final int RTC_STEP_RHO   = 1900;
    public static final int RTC_STEP_R     = 1901;
    public static final int RTC_STEP_X     = 1902;
    public static final int RTC_STEP_G     = 1903;
    public static final int RTC_STEP_B     = 1904;
    public static final int PTC_STEP_ALPHA = 1910;
    public static final int PTC_STEP_RHO   = 1911;
    public static final int PTC_STEP_R     = 1912;
    public static final int PTC_STEP_X     = 1913;
    public static final int PTC_STEP_G     = 1914;
    public static final int PTC_STEP_B     = 1915;

    // ── TwoWT tap changer additional properties ───────────────────────────────
    public static final int TWO_WT_RTC_TARGET_DEADBAND = 1920;
    public static final int TWO_WT_RTC_REG_TERMINAL_ID = 1921;
    public static final int TWO_WT_PTC_TARGET_DEADBAND = 1922;
    public static final int TWO_WT_PTC_REG_TERMINAL_ID = 1923;

    // ── ThreeWT leg tap changer additional properties ─────────────────────────
    public static final int THREE_WT_LEG1_RTC_TARGET_DEADBAND = 1950;
    public static final int THREE_WT_LEG1_RTC_REG_TERMINAL_ID = 1951;
    public static final int THREE_WT_LEG2_RTC_TARGET_DEADBAND = 1952;
    public static final int THREE_WT_LEG2_RTC_REG_TERMINAL_ID = 1953;
    public static final int THREE_WT_LEG3_RTC_TARGET_DEADBAND = 1954;
    public static final int THREE_WT_LEG3_RTC_REG_TERMINAL_ID = 1955;
    public static final int THREE_WT_LEG1_PTC_TARGET_DEADBAND = 1956;
    public static final int THREE_WT_LEG1_PTC_REG_TERMINAL_ID = 1957;
    public static final int THREE_WT_LEG2_PTC_TARGET_DEADBAND = 1958;
    public static final int THREE_WT_LEG2_PTC_REG_TERMINAL_ID = 1959;
    public static final int THREE_WT_LEG3_PTC_TARGET_DEADBAND = 1960;
    public static final int THREE_WT_LEG3_PTC_REG_TERMINAL_ID = 1961;

    // ── HvdcLine ─────────────────────────────────────────────────────────
    public static final int HVDC_R                      = 900;
    public static final int HVDC_NOMINAL_V              = 901;
    public static final int HVDC_ACTIVE_POWER_SETPOINT  = 902;
    public static final int HVDC_MAX_P                  = 903;
    public static final int HVDC_CONVERTERS_MODE        = 904;
    public static final int HVDC_CONVERTER_STATION1_ID  = 905;
    public static final int HVDC_CONVERTER_STATION2_ID  = 906;

    // ── DanglingLine ─────────────────────────────────────────────────────
    public static final int DL_P0 = 1000;
    public static final int DL_Q0 = 1001;
    public static final int DL_R  = 1002;
    public static final int DL_X  = 1003;
    public static final int DL_G  = 1004;
    public static final int DL_B  = 1005;

    // ── ShuntCompensator ─────────────────────────────────────────────────
    public static final int SHUNT_SECTION_COUNT        = 1100;
    public static final int SHUNT_MAX_SECTION_COUNT    = 1101;
    public static final int SHUNT_B_PER_SECTION        = 1102;
    public static final int SHUNT_G_PER_SECTION        = 1103;
    public static final int SHUNT_MODEL_KIND           = 1104; // 0=LINEAR 1=NON_LINEAR
    public static final int SHUNT_SECTION_B            = 1105; // on section handle
    public static final int SHUNT_SECTION_G            = 1106; // on section handle
    public static final int SHUNT_VOLTAGE_REGULATOR_ON = 1107;
    public static final int SHUNT_TARGET_V             = 1108;
    public static final int SHUNT_REGULATING_TERMINAL_ID = 1109;

    // ── StaticVarCompensator ─────────────────────────────────────────────
    public static final int SVC_B_MIN                   = 1200;
    public static final int SVC_B_MAX                   = 1201;
    public static final int SVC_VOLTAGE_SETPOINT        = 1202;
    public static final int SVC_REACTIVE_POWER_SETPOINT = 1203;
    public static final int SVC_REGULATION_MODE         = 1204;

    // ── VscConverterStation ──────────────────────────────────────────────
    public static final int VSC_LOSS_FACTOR             = 1300;
    public static final int VSC_VOLTAGE_REGULATOR_ON    = 1301;
    public static final int VSC_VOLTAGE_SETPOINT        = 1302;
    public static final int VSC_REACTIVE_POWER_SETPOINT = 1303;
    public static final int VSC_REACTIVE_LIMITS_KIND    = 1304; // 0=NONE 1=MIN_MAX 2=CURVE
    public static final int VSC_MIN_Q                   = 1305;
    public static final int VSC_MAX_Q                   = 1306;

    // ── LccConverterStation ──────────────────────────────────────────────
    public static final int LCC_LOSS_FACTOR  = 1400;
    public static final int LCC_POWER_FACTOR = 1401;

    // ── BusbarSection ────────────────────────────────────────────────────────
    public static final int BBS_V     = 1600;
    public static final int BBS_ANGLE = 1601;
    public static final int IC_NODE1  = 1610;
    public static final int IC_NODE2  = 1611;

    // ── Battery ───────────────────────────────────────────────────────────────
    public static final int BAT_TARGET_P = 1700;
    public static final int BAT_TARGET_Q = 1701;
    public static final int BAT_MIN_P    = 1702;
    public static final int BAT_MAX_P    = 1703;

    // ── ReactiveCapabilityCurve point properties ──────────────────────────────
    public static final int POINT_P     = 1800;
    public static final int POINT_MIN_Q = 1801;
    public static final int POINT_MAX_Q = 1802;

    // ── Switch ───────────────────────────────────────────────────────────────
    public static final int SW_OPEN     = 1500;
    public static final int SW_RETAINED = 1501;
    public static final int SW_KIND     = 1502;
    public static final int SW_NODE1    = 1503;
    public static final int SW_NODE2    = 1504;

    // ── Extension: ActivePowerControl ────────────────────────────────────
    public static final int EXT_APC_EXISTS      = 3000;
    public static final int EXT_APC_DROOP       = 3001;
    public static final int EXT_APC_PARTICIPATE = 3002;

    // ── Extension: CoordinatedReactiveControl ─────────────────────────────
    public static final int EXT_CRC_EXISTS    = 3100;
    public static final int EXT_CRC_Q_PERCENT = 3101;

    // ── Extension: HvdcAngleDroopActivePowerControl ───────────────────────
    public static final int EXT_HADAPC_EXISTS  = 3200;
    public static final int EXT_HADAPC_DROOP   = 3201;
    public static final int EXT_HADAPC_P0      = 3202;
    public static final int EXT_HADAPC_ENABLED = 3203;

    // ── Extension: HvdcOperatorActivePowerRange ───────────────────────────
    public static final int EXT_HOAR_EXISTS          = 3300;
    public static final int EXT_HOAR_OPR_CS1_TO_CS2 = 3301;
    public static final int EXT_HOAR_OPR_CS2_TO_CS1 = 3302;

    // ── Extension: VoltagePerReactivePowerControl ─────────────────────────
    public static final int EXT_VPRC_EXISTS = 3400;
    public static final int EXT_VPRC_SLOPE  = 3401;

    // ── Extension: SlackTerminal ──────────────────────────────────────────
    public static final int EXT_ST_EXISTS = 3500;

    // ── Relations ────────────────────────────────────────────────────────
    public static final int REL_TERMINAL1        = 2000;
    public static final int REL_TERMINAL2        = 2001;
    public static final int REL_TERMINAL         = 2002;
    public static final int REL_BUS              = 2003;
    public static final int REL_VOLTAGE_LEVEL    = 2004;
    public static final int REL_SUBSTATION       = 2005;
    public static final int REL_SLACK_TERMINAL   = 2006;
    public static final int REL_CONNECTABLE_BUS      = 2007;
    public static final int REL_REGULATING_TERMINAL  = 2008;
}
