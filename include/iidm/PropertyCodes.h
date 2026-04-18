#pragma once

namespace iidm::prop {

// ── Object types (used in getChildren / findById) ───────────────────────────
constexpr int SUBSTATION                  = 1;
constexpr int VOLTAGE_LEVEL               = 2;
constexpr int GENERATOR                   = 3;
constexpr int LOAD                        = 4;
constexpr int LINE                        = 5;
constexpr int TWO_WINDINGS_TRANSFORMER    = 6;
constexpr int THREE_WINDINGS_TRANSFORMER  = 7;
constexpr int HVDC_LINE                   = 8;
constexpr int DANGLING_LINE               = 9;
constexpr int SHUNT_COMPENSATOR           = 10;
constexpr int STATIC_VAR_COMPENSATOR      = 11;
constexpr int VSC_CONVERTER_STATION       = 12;
constexpr int LCC_CONVERTER_STATION       = 13;
constexpr int TERMINAL                    = 14;
constexpr int BUS                         = 15;
constexpr int SWITCH                      = 16;
constexpr int BUSBAR_SECTION              = 17;
constexpr int BATTERY                     = 18;
constexpr int REACTIVE_CURVE_POINT        = 19;
constexpr int TWO_WT_RTC_STEP             = 20;
constexpr int TWO_WT_PTC_STEP             = 21;
constexpr int THREE_WT_LEG1_RTC_STEP      = 22;
constexpr int THREE_WT_LEG2_RTC_STEP      = 23;
constexpr int THREE_WT_LEG3_RTC_STEP      = 24;
constexpr int THREE_WT_LEG1_PTC_STEP      = 25;
constexpr int THREE_WT_LEG2_PTC_STEP      = 26;
constexpr int THREE_WT_LEG3_PTC_STEP      = 27;
constexpr int SHUNT_SECTION               = 28;

// ── Shared properties (applicable to many types) ────────────────────────────
constexpr int ID                          = 100;
constexpr int NAME                        = 101;
constexpr int COUNTRY                     = 102;

// ── Generator-specific ───────────────────────────────────────────────────────
constexpr int GEN_TARGET_P                = 200;
constexpr int GEN_TARGET_Q                = 201;
constexpr int GEN_TARGET_V                = 202;
constexpr int GEN_MIN_P                   = 203;
constexpr int GEN_MAX_P                   = 204;
constexpr int GEN_VOLTAGE_REGULATOR_ON    = 205;
constexpr int GEN_ENERGY_SOURCE           = 206;
constexpr int GEN_REACTIVE_LIMITS_KIND    = 207; // 0=NONE 1=MIN_MAX 2=CURVE
constexpr int GEN_MIN_Q                   = 208;
constexpr int GEN_MAX_Q                   = 209;

// ── Load-specific ────────────────────────────────────────────────────────────
constexpr int LOAD_P0                     = 300;
constexpr int LOAD_Q0                     = 301;
constexpr int LOAD_TYPE                   = 302;

// ── Terminal ─────────────────────────────────────────────────────────────────
constexpr int TERMINAL_P                  = 400;
constexpr int TERMINAL_Q                  = 401;
constexpr int TERMINAL_CONNECTED          = 402;
constexpr int TERMINAL_BUS_ID            = 403;
constexpr int TERMINAL_NODE               = 404; // node number (node-breaker topology)

// ── Bus ──────────────────────────────────────────────────────────────────────
constexpr int BUS_V                       = 500;
constexpr int BUS_ANGLE                   = 501;

// ── VoltageLevel ─────────────────────────────────────────────────────────────
constexpr int VL_NOMINAL_V                = 600;
constexpr int VL_LOW_VOLTAGE_LIMIT        = 601;
constexpr int VL_HIGH_VOLTAGE_LIMIT       = 602;
constexpr int VL_TOPOLOGY_KIND            = 603;

// ── Line ─────────────────────────────────────────────────────────────────────
constexpr int LINE_R                      = 700;
constexpr int LINE_X                      = 701;
constexpr int LINE_G1                     = 702;
constexpr int LINE_B1                     = 703;
constexpr int LINE_G2                     = 704;
constexpr int LINE_B2                     = 705;

// ── ThreeWindingsTransformer legs ─────────────────────────────────────────────
// Each leg uses a 20-code block: legBase + offset below.
// Leg1 base=820, Leg2 base=840, Leg3 base=860.
constexpr int THREE_WT_LEG1_BASE          = 820;
constexpr int THREE_WT_LEG2_BASE          = 840;
constexpr int THREE_WT_LEG3_BASE          = 860;
// Leg property offsets (add to LEG*_BASE)
constexpr int THREE_WT_LEG_R_OFF              = 0;
constexpr int THREE_WT_LEG_X_OFF              = 1;
constexpr int THREE_WT_LEG_G_OFF              = 2;
constexpr int THREE_WT_LEG_B_OFF              = 3;
constexpr int THREE_WT_LEG_RATED_U_OFF        = 4;
constexpr int THREE_WT_LEG_RATED_S_OFF        = 5;
constexpr int THREE_WT_LEG_RTC_EXISTS_OFF     = 6;
constexpr int THREE_WT_LEG_RTC_TAP_POS_OFF    = 7;
constexpr int THREE_WT_LEG_RTC_LOW_TAP_OFF    = 8;
constexpr int THREE_WT_LEG_RTC_HIGH_TAP_OFF   = 9;
constexpr int THREE_WT_LEG_RTC_REGULATING_OFF = 10;
constexpr int THREE_WT_LEG_RTC_TARGET_V_OFF   = 11;
constexpr int THREE_WT_LEG_PTC_EXISTS_OFF     = 12;
constexpr int THREE_WT_LEG_PTC_TAP_POS_OFF    = 13;
constexpr int THREE_WT_LEG_PTC_LOW_TAP_OFF    = 14;
constexpr int THREE_WT_LEG_PTC_HIGH_TAP_OFF   = 15;
constexpr int THREE_WT_LEG_PTC_REGULATING_OFF = 16;
constexpr int THREE_WT_LEG_PTC_REG_MODE_OFF   = 17;
constexpr int THREE_WT_LEG_PTC_REG_VALUE_OFF  = 18;

// ── TwoWindingsTransformer ───────────────────────────────────────────────────
constexpr int TWO_WT_R                    = 800;
constexpr int TWO_WT_X                    = 801;
constexpr int TWO_WT_G                    = 802;
constexpr int TWO_WT_B                    = 803;
constexpr int TWO_WT_RATED_U1             = 804;
constexpr int TWO_WT_RATED_U2             = 805;
constexpr int TWO_WT_RATED_S              = 806;
constexpr int TWO_WT_RTC_EXISTS           = 807;
constexpr int TWO_WT_RTC_TAP_POSITION     = 808;
constexpr int TWO_WT_RTC_LOW_TAP          = 809;
constexpr int TWO_WT_RTC_HIGH_TAP         = 810;
constexpr int TWO_WT_RTC_REGULATING       = 811;
constexpr int TWO_WT_RTC_TARGET_V         = 812;
constexpr int TWO_WT_PTC_EXISTS           = 813;
constexpr int TWO_WT_PTC_TAP_POSITION     = 814;
constexpr int TWO_WT_PTC_LOW_TAP          = 815;
constexpr int TWO_WT_PTC_HIGH_TAP         = 816;
constexpr int TWO_WT_PTC_REGULATING       = 817;
constexpr int TWO_WT_PTC_REG_MODE         = 818;
constexpr int TWO_WT_PTC_REG_VALUE        = 819;

// ── TapChangerStep properties (shared by RTC and PTC step handles) ────────────
constexpr int RTC_STEP_RHO                = 1900;
constexpr int RTC_STEP_R                  = 1901;
constexpr int RTC_STEP_X                  = 1902;
constexpr int RTC_STEP_G                  = 1903;
constexpr int RTC_STEP_B                  = 1904;
constexpr int PTC_STEP_ALPHA              = 1910;
constexpr int PTC_STEP_RHO                = 1911;
constexpr int PTC_STEP_R                  = 1912;
constexpr int PTC_STEP_X                  = 1913;
constexpr int PTC_STEP_G                  = 1914;
constexpr int PTC_STEP_B                  = 1915;

// ── HvdcLine ─────────────────────────────────────────────────────────────────
constexpr int HVDC_R                      = 900;
constexpr int HVDC_NOMINAL_V             = 901;
constexpr int HVDC_ACTIVE_POWER_SETPOINT = 902;
constexpr int HVDC_MAX_P                  = 903;
constexpr int HVDC_CONVERTERS_MODE        = 904;

// ── DanglingLine ─────────────────────────────────────────────────────────────
constexpr int DL_P0                       = 1000;
constexpr int DL_Q0                       = 1001;
constexpr int DL_R                        = 1002;
constexpr int DL_X                        = 1003;
constexpr int DL_G                        = 1004;
constexpr int DL_B                        = 1005;

// ── ShuntCompensator ─────────────────────────────────────────────────────────
constexpr int SHUNT_SECTION_COUNT         = 1100;
constexpr int SHUNT_MAX_SECTION_COUNT     = 1101;
constexpr int SHUNT_B_PER_SECTION         = 1102;
constexpr int SHUNT_G_PER_SECTION         = 1103;
constexpr int SHUNT_MODEL_KIND            = 1104; // 0=LINEAR 1=NON_LINEAR
constexpr int SHUNT_SECTION_B             = 1105; // on section handle
constexpr int SHUNT_SECTION_G             = 1106; // on section handle

// ── StaticVarCompensator ─────────────────────────────────────────────────────
constexpr int SVC_B_MIN                   = 1200;
constexpr int SVC_B_MAX                   = 1201;
constexpr int SVC_VOLTAGE_SETPOINT        = 1202;
constexpr int SVC_REACTIVE_POWER_SETPOINT = 1203;
constexpr int SVC_REGULATION_MODE         = 1204;

// ── VscConverterStation ──────────────────────────────────────────────────────
constexpr int VSC_LOSS_FACTOR             = 1300;
constexpr int VSC_VOLTAGE_REGULATOR_ON    = 1301;
constexpr int VSC_VOLTAGE_SETPOINT        = 1302;
constexpr int VSC_REACTIVE_POWER_SETPOINT = 1303;
constexpr int VSC_REACTIVE_LIMITS_KIND    = 1304; // 0=NONE 1=MIN_MAX 2=CURVE
constexpr int VSC_MIN_Q                   = 1305;
constexpr int VSC_MAX_Q                   = 1306;

// ── LccConverterStation ──────────────────────────────────────────────────────
constexpr int LCC_LOSS_FACTOR             = 1400;
constexpr int LCC_POWER_FACTOR            = 1401;

// ── BusbarSection ────────────────────────────────────────────────────────────
constexpr int BBS_V                       = 1600;
constexpr int BBS_ANGLE                   = 1601;

// ── Battery ───────────────────────────────────────────────────────────────────
constexpr int BAT_TARGET_P                = 1700;
constexpr int BAT_TARGET_Q                = 1701;
constexpr int BAT_MIN_P                   = 1702;
constexpr int BAT_MAX_P                   = 1703;

// ── ReactiveCapabilityCurve point properties ──────────────────────────────────
constexpr int POINT_P                     = 1800;
constexpr int POINT_MIN_Q                 = 1801;
constexpr int POINT_MAX_Q                 = 1802;

// ── Switch ───────────────────────────────────────────────────────────────────
constexpr int SW_OPEN                     = 1500;
constexpr int SW_RETAINED                 = 1501;
constexpr int SW_KIND                     = 1502;

// ── Extension: ActivePowerControl ────────────────────────────────────────────
constexpr int EXT_APC_EXISTS              = 3000;
constexpr int EXT_APC_DROOP              = 3001;
constexpr int EXT_APC_PARTICIPATE        = 3002;

// ── Extension: CoordinatedReactiveControl ─────────────────────────────────────
constexpr int EXT_CRC_EXISTS    = 3100;
constexpr int EXT_CRC_Q_PERCENT = 3101;

// ── Extension: HvdcAngleDroopActivePowerControl ───────────────────────────────
constexpr int EXT_HADAPC_EXISTS   = 3200;
constexpr int EXT_HADAPC_DROOP    = 3201;
constexpr int EXT_HADAPC_P0       = 3202;
constexpr int EXT_HADAPC_ENABLED  = 3203;

// ── Extension: HvdcOperatorActivePowerRange ───────────────────────────────────
constexpr int EXT_HOAR_EXISTS          = 3300;
constexpr int EXT_HOAR_OPR_CS1_TO_CS2 = 3301;
constexpr int EXT_HOAR_OPR_CS2_TO_CS1 = 3302;

// ── Extension: VoltagePerReactivePowerControl ─────────────────────────────────
constexpr int EXT_VPRC_EXISTS = 3400;
constexpr int EXT_VPRC_SLOPE  = 3401;

// ── Extension: SlackTerminal ──────────────────────────────────────────────────
constexpr int EXT_ST_EXISTS = 3500;

// ── Relations (used in getRelated) ──────────────────────────────────────────
constexpr int REL_TERMINAL1               = 2000;
constexpr int REL_TERMINAL2               = 2001;
constexpr int REL_TERMINAL                = 2002;
constexpr int REL_BUS                     = 2003;
constexpr int REL_VOLTAGE_LEVEL           = 2004;
constexpr int REL_SUBSTATION              = 2005;
constexpr int REL_SLACK_TERMINAL          = 2006;
constexpr int REL_CONNECTABLE_BUS         = 2007; // bus-breaker topology bus (may differ from bus view)

} // namespace iidm::prop
