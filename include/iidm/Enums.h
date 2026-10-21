#pragma once

#include <iidm/stdcxx/optional.hpp>
#include <iosfwd>
#include <string>

namespace iidm {

enum class EnergySource { HYDRO, NUCLEAR, WIND, THERMAL, SOLAR, OTHER };
enum class LoadType { UNDEFINED, AUXILIARY, FICTITIOUS };
enum class TopologyKind { NODE_BREAKER, BUS_BREAKER };
enum class Country {
    AF, AL, DZ, AD, AO, AG, AR, AM, AU, AT, AZ, BS, BH, BD, BB, BY, BE, BZ,
    BJ, BT, BO, BA, BW, BR, BN, BG, BF, BI, CV, KH, CM, CA, CF, TD, CL, CN,
    CO, KM, CG, CR, HR, CU, CY, CZ, DK, DJ, DM, DO, EC, EG, SV, GQ, ER, EE,
    SZ, ET, FJ, FI, FR, GA, GM, GE, DE, GH, GR, GD, GT, GN, GW, GY, HT, HN,
    HU, IS, IN, ID, IR, IQ, IE, IL, IT, JM, JP, JO, KZ, KE, KI, KP, KR, KW,
    KG, LA, LV, LB, LS, LR, LY, LI, LT, LU, MG, MW, MY, MV, ML, MT, MH, MR,
    MU, MX, FM, MD, MC, MN, ME, MA, MZ, MM, NA, NR, NP, NL, NZ, NI, NE, NG,
    NO, OM, PK, PW, PA, PG, PY, PE, PH, PL, PT, QA, RO, RU, RW, KN, LC, VC,
    WS, SM, ST, SA, SN, RS, SC, SL, SG, SK, SI, SB, SO, ZA, SS, ES, LK, SD,
    SR, SE, CH, SY, TW, TJ, TZ, TH, TL, TG, TO, TT, TN, TR, TM, TV, UG, UA,
    AE, GB, US, UY, UZ, VU, VE, VN, YE, ZM, ZW
};
enum class HvdcConverterStationMode { RECTIFIER, INVERTER };
enum class SwitchKind { BREAKER, DISCONNECTOR, LOAD_BREAK_SWITCH };
enum class PhaseTapChangerRegulationMode { CURRENT_LIMITER, ACTIVE_POWER_CONTROL };
enum class HvdcOperatorActivePowerRangeSide { SIDE1, SIDE2 };
enum class TwoSides { ONE, TWO };
enum class ThreeSides { ONE, TWO, THREE };
// Type of an identifiable (powsybl IdentifiableType, same order)
enum class IdentifiableType {
    NETWORK, SUBSTATION, VOLTAGE_LEVEL, AREA, HVDC_LINE, BUS, SWITCH, BUSBAR_SECTION, LINE, TIE_LINE,
    TWO_WINDINGS_TRANSFORMER, THREE_WINDINGS_TRANSFORMER, GENERATOR, BATTERY, LOAD, SHUNT_COMPENSATOR,
    BOUNDARY_LINE, STATIC_VAR_COMPENSATOR, HVDC_CONVERTER_STATION, OVERLOAD_MANAGEMENT_SYSTEM, GROUND,
    DC_NODE, DC_SWITCH, DC_GROUND, DC_LINE, LINE_COMMUTATED_CONVERTER, VOLTAGE_SOURCE_CONVERTER, DC_BUS
};
// Technology of an HVDC converter station
enum class HvdcType { VSC, LCC };
// Kind of reactive limits (powsybl ReactiveLimitsKind)
enum class ReactiveLimitsKind { MIN_MAX, CURVE, SHAPE };
// Mode of a VoltageRegulation (powsybl 7.4 regulation.RegulationMode)
enum class RegulationMode { VOLTAGE, REACTIVE_POWER, VOLTAGE_PER_REACTIVE_POWER };
// Type of operational limits (powsybl LimitType)
enum class LimitType { ACTIVE_POWER, APPARENT_POWER, CURRENT, VOLTAGE, VOLTAGE_ANGLE };
// Whether limits are maximums (HIGH) or minimums (LOW) (powsybl DetectionKind)
enum class DetectionKind { HIGH, LOW };
// Model of a shunt compensator's sections (powsybl ShuntCompensatorModelType)
enum class ShuntCompensatorModelType { LINEAR, NON_LINEAR };

// ── State estimation extensions (same order as the powsybl-core enums) ──────
enum class MeasurementType {
    ANGLE, ACTIVE_POWER, APPARENT_POWER, REACTIVE_POWER, CURRENT, VOLTAGE, FREQUENCY, OTHER
};
enum class DiscreteMeasurementType { TAP_POSITION, SWITCH_POSITION, SHUNT_COMPENSATOR_SECTION, OTHER };
enum class DiscreteMeasurementTapChanger {
    RATIO_TAP_CHANGER, PHASE_TAP_CHANGER,
    RATIO_TAP_CHANGER_1, PHASE_TAP_CHANGER_1,
    RATIO_TAP_CHANGER_2, PHASE_TAP_CHANGER_2,
    RATIO_TAP_CHANGER_3, PHASE_TAP_CHANGER_3
};
enum class DiscreteMeasurementValueType { BOOLEAN, INT, STRING };
enum class ObservabilityStatus { OBSERVABLE, NON_OBSERVABLE, BORDER };

std::string             getCountryName(Country country);
iidm::optional<Country> countryFromName(const std::string& name);
std::ostream&           operator<<(std::ostream& os, Country c);

} // namespace iidm
