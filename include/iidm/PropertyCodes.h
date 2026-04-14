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

// ── Load-specific ────────────────────────────────────────────────────────────
constexpr int LOAD_P0                     = 300;
constexpr int LOAD_Q0                     = 301;
constexpr int LOAD_TYPE                   = 302;

// ── Terminal ─────────────────────────────────────────────────────────────────
constexpr int TERMINAL_P                  = 400;
constexpr int TERMINAL_Q                  = 401;
constexpr int TERMINAL_CONNECTED          = 402;
constexpr int TERMINAL_BUS_ID            = 403;

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

// ── TwoWindingsTransformer ───────────────────────────────────────────────────
constexpr int TWO_WT_R                    = 800;
constexpr int TWO_WT_X                    = 801;
constexpr int TWO_WT_G                    = 802;
constexpr int TWO_WT_B                    = 803;
constexpr int TWO_WT_RATED_U1             = 804;
constexpr int TWO_WT_RATED_U2             = 805;
constexpr int TWO_WT_RATED_S              = 806;

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

// ── LccConverterStation ──────────────────────────────────────────────────────
constexpr int LCC_LOSS_FACTOR             = 1400;
constexpr int LCC_POWER_FACTOR            = 1401;

// ── Extension: ActivePowerControl ────────────────────────────────────────────
constexpr int EXT_APC_EXISTS              = 3000;
constexpr int EXT_APC_DROOP              = 3001;
constexpr int EXT_APC_PARTICIPATE        = 3002;

// ── Relations (used in getRelated) ──────────────────────────────────────────
constexpr int REL_TERMINAL1               = 2000;
constexpr int REL_TERMINAL2               = 2001;
constexpr int REL_TERMINAL                = 2002;
constexpr int REL_BUS                     = 2003;
constexpr int REL_VOLTAGE_LEVEL           = 2004;
constexpr int REL_SUBSTATION              = 2005;

} // namespace iidm::prop
