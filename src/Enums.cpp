#include <iidm/Enums.h>
#include <iidm/IidmException.h>
#include <ostream>
#include <string>
#include <unordered_map>

namespace iidm {

std::string getCountryName(Country country) {
    switch (country) {
        case Country::AF: return "AF";
        case Country::AL: return "AL";
        case Country::DZ: return "DZ";
        case Country::AD: return "AD";
        case Country::AO: return "AO";
        case Country::AG: return "AG";
        case Country::AR: return "AR";
        case Country::AM: return "AM";
        case Country::AU: return "AU";
        case Country::AT: return "AT";
        case Country::AZ: return "AZ";
        case Country::BS: return "BS";
        case Country::BH: return "BH";
        case Country::BD: return "BD";
        case Country::BB: return "BB";
        case Country::BY: return "BY";
        case Country::BE: return "BE";
        case Country::BZ: return "BZ";
        case Country::BJ: return "BJ";
        case Country::BT: return "BT";
        case Country::BO: return "BO";
        case Country::BA: return "BA";
        case Country::BW: return "BW";
        case Country::BR: return "BR";
        case Country::BN: return "BN";
        case Country::BG: return "BG";
        case Country::BF: return "BF";
        case Country::BI: return "BI";
        case Country::CV: return "CV";
        case Country::KH: return "KH";
        case Country::CM: return "CM";
        case Country::CA: return "CA";
        case Country::CF: return "CF";
        case Country::TD: return "TD";
        case Country::CL: return "CL";
        case Country::CN: return "CN";
        case Country::CO: return "CO";
        case Country::KM: return "KM";
        case Country::CG: return "CG";
        case Country::CR: return "CR";
        case Country::HR: return "HR";
        case Country::CU: return "CU";
        case Country::CY: return "CY";
        case Country::CZ: return "CZ";
        case Country::DK: return "DK";
        case Country::DJ: return "DJ";
        case Country::DM: return "DM";
        case Country::DO: return "DO";
        case Country::EC: return "EC";
        case Country::EG: return "EG";
        case Country::SV: return "SV";
        case Country::GQ: return "GQ";
        case Country::ER: return "ER";
        case Country::EE: return "EE";
        case Country::SZ: return "SZ";
        case Country::ET: return "ET";
        case Country::FJ: return "FJ";
        case Country::FI: return "FI";
        case Country::FR: return "FR";
        case Country::GA: return "GA";
        case Country::GM: return "GM";
        case Country::GE: return "GE";
        case Country::DE: return "DE";
        case Country::GH: return "GH";
        case Country::GR: return "GR";
        case Country::GD: return "GD";
        case Country::GT: return "GT";
        case Country::GN: return "GN";
        case Country::GW: return "GW";
        case Country::GY: return "GY";
        case Country::HT: return "HT";
        case Country::HN: return "HN";
        case Country::HU: return "HU";
        case Country::IS: return "IS";
        case Country::IN: return "IN";
        case Country::ID: return "ID";
        case Country::IR: return "IR";
        case Country::IQ: return "IQ";
        case Country::IE: return "IE";
        case Country::IL: return "IL";
        case Country::IT: return "IT";
        case Country::JM: return "JM";
        case Country::JP: return "JP";
        case Country::JO: return "JO";
        case Country::KZ: return "KZ";
        case Country::KE: return "KE";
        case Country::KI: return "KI";
        case Country::KP: return "KP";
        case Country::KR: return "KR";
        case Country::KW: return "KW";
        case Country::KG: return "KG";
        case Country::LA: return "LA";
        case Country::LV: return "LV";
        case Country::LB: return "LB";
        case Country::LS: return "LS";
        case Country::LR: return "LR";
        case Country::LY: return "LY";
        case Country::LI: return "LI";
        case Country::LT: return "LT";
        case Country::LU: return "LU";
        case Country::MG: return "MG";
        case Country::MW: return "MW";
        case Country::MY: return "MY";
        case Country::MV: return "MV";
        case Country::ML: return "ML";
        case Country::MT: return "MT";
        case Country::MH: return "MH";
        case Country::MR: return "MR";
        case Country::MU: return "MU";
        case Country::MX: return "MX";
        case Country::FM: return "FM";
        case Country::MD: return "MD";
        case Country::MC: return "MC";
        case Country::MN: return "MN";
        case Country::ME: return "ME";
        case Country::MA: return "MA";
        case Country::MZ: return "MZ";
        case Country::MM: return "MM";
        case Country::NA: return "NA";
        case Country::NR: return "NR";
        case Country::NP: return "NP";
        case Country::NL: return "NL";
        case Country::NZ: return "NZ";
        case Country::NI: return "NI";
        case Country::NE: return "NE";
        case Country::NG: return "NG";
        case Country::NO: return "NO";
        case Country::OM: return "OM";
        case Country::PK: return "PK";
        case Country::PW: return "PW";
        case Country::PA: return "PA";
        case Country::PG: return "PG";
        case Country::PY: return "PY";
        case Country::PE: return "PE";
        case Country::PH: return "PH";
        case Country::PL: return "PL";
        case Country::PT: return "PT";
        case Country::QA: return "QA";
        case Country::RO: return "RO";
        case Country::RU: return "RU";
        case Country::RW: return "RW";
        case Country::KN: return "KN";
        case Country::LC: return "LC";
        case Country::VC: return "VC";
        case Country::WS: return "WS";
        case Country::SM: return "SM";
        case Country::ST: return "ST";
        case Country::SA: return "SA";
        case Country::SN: return "SN";
        case Country::RS: return "RS";
        case Country::SC: return "SC";
        case Country::SL: return "SL";
        case Country::SG: return "SG";
        case Country::SK: return "SK";
        case Country::SI: return "SI";
        case Country::SB: return "SB";
        case Country::SO: return "SO";
        case Country::ZA: return "ZA";
        case Country::SS: return "SS";
        case Country::ES: return "ES";
        case Country::LK: return "LK";
        case Country::SD: return "SD";
        case Country::SR: return "SR";
        case Country::SE: return "SE";
        case Country::CH: return "CH";
        case Country::SY: return "SY";
        case Country::TW: return "TW";
        case Country::TJ: return "TJ";
        case Country::TZ: return "TZ";
        case Country::TH: return "TH";
        case Country::TL: return "TL";
        case Country::TG: return "TG";
        case Country::TO: return "TO";
        case Country::TT: return "TT";
        case Country::TN: return "TN";
        case Country::TR: return "TR";
        case Country::TM: return "TM";
        case Country::TV: return "TV";
        case Country::UG: return "UG";
        case Country::UA: return "UA";
        case Country::AE: return "AE";
        case Country::GB: return "GB";
        case Country::US: return "US";
        case Country::UY: return "UY";
        case Country::UZ: return "UZ";
        case Country::VU: return "VU";
        case Country::VE: return "VE";
        case Country::VN: return "VN";
        case Country::YE: return "YE";
        case Country::ZM: return "ZM";
        case Country::ZW: return "ZW";
        default: throw IidmException("Unknown Country enum value");
    }
}

iidm::optional<Country> countryFromName(const std::string& name) {
    if (name.empty()) return {};
    static const std::unordered_map<std::string, Country> table = {
        {"AF",Country::AF},{"AL",Country::AL},{"DZ",Country::DZ},{"AD",Country::AD},
        {"AO",Country::AO},{"AG",Country::AG},{"AR",Country::AR},{"AM",Country::AM},
        {"AU",Country::AU},{"AT",Country::AT},{"AZ",Country::AZ},{"BS",Country::BS},
        {"BH",Country::BH},{"BD",Country::BD},{"BB",Country::BB},{"BY",Country::BY},
        {"BE",Country::BE},{"BZ",Country::BZ},{"BJ",Country::BJ},{"BT",Country::BT},
        {"BO",Country::BO},{"BA",Country::BA},{"BW",Country::BW},{"BR",Country::BR},
        {"BN",Country::BN},{"BG",Country::BG},{"BF",Country::BF},{"BI",Country::BI},
        {"CV",Country::CV},{"KH",Country::KH},{"CM",Country::CM},{"CA",Country::CA},
        {"CF",Country::CF},{"TD",Country::TD},{"CL",Country::CL},{"CN",Country::CN},
        {"CO",Country::CO},{"KM",Country::KM},{"CG",Country::CG},{"CR",Country::CR},
        {"HR",Country::HR},{"CU",Country::CU},{"CY",Country::CY},{"CZ",Country::CZ},
        {"DK",Country::DK},{"DJ",Country::DJ},{"DM",Country::DM},{"DO",Country::DO},
        {"EC",Country::EC},{"EG",Country::EG},{"SV",Country::SV},{"GQ",Country::GQ},
        {"ER",Country::ER},{"EE",Country::EE},{"SZ",Country::SZ},{"ET",Country::ET},
        {"FJ",Country::FJ},{"FI",Country::FI},{"FR",Country::FR},{"GA",Country::GA},
        {"GM",Country::GM},{"GE",Country::GE},{"DE",Country::DE},{"GH",Country::GH},
        {"GR",Country::GR},{"GD",Country::GD},{"GT",Country::GT},{"GN",Country::GN},
        {"GW",Country::GW},{"GY",Country::GY},{"HT",Country::HT},{"HN",Country::HN},
        {"HU",Country::HU},{"IS",Country::IS},{"IN",Country::IN},{"ID",Country::ID},
        {"IR",Country::IR},{"IQ",Country::IQ},{"IE",Country::IE},{"IL",Country::IL},
        {"IT",Country::IT},{"JM",Country::JM},{"JP",Country::JP},{"JO",Country::JO},
        {"KZ",Country::KZ},{"KE",Country::KE},{"KI",Country::KI},{"KP",Country::KP},
        {"KR",Country::KR},{"KW",Country::KW},{"KG",Country::KG},{"LA",Country::LA},
        {"LV",Country::LV},{"LB",Country::LB},{"LS",Country::LS},{"LR",Country::LR},
        {"LY",Country::LY},{"LI",Country::LI},{"LT",Country::LT},{"LU",Country::LU},
        {"MG",Country::MG},{"MW",Country::MW},{"MY",Country::MY},{"MV",Country::MV},
        {"ML",Country::ML},{"MT",Country::MT},{"MH",Country::MH},{"MR",Country::MR},
        {"MU",Country::MU},{"MX",Country::MX},{"FM",Country::FM},{"MD",Country::MD},
        {"MC",Country::MC},{"MN",Country::MN},{"ME",Country::ME},{"MA",Country::MA},
        {"MZ",Country::MZ},{"MM",Country::MM},{"NA",Country::NA},{"NR",Country::NR},
        {"NP",Country::NP},{"NL",Country::NL},{"NZ",Country::NZ},{"NI",Country::NI},
        {"NE",Country::NE},{"NG",Country::NG},{"NO",Country::NO},{"OM",Country::OM},
        {"PK",Country::PK},{"PW",Country::PW},{"PA",Country::PA},{"PG",Country::PG},
        {"PY",Country::PY},{"PE",Country::PE},{"PH",Country::PH},{"PL",Country::PL},
        {"PT",Country::PT},{"QA",Country::QA},{"RO",Country::RO},{"RU",Country::RU},
        {"RW",Country::RW},{"KN",Country::KN},{"LC",Country::LC},{"VC",Country::VC},
        {"WS",Country::WS},{"SM",Country::SM},{"ST",Country::ST},{"SA",Country::SA},
        {"SN",Country::SN},{"RS",Country::RS},{"SC",Country::SC},{"SL",Country::SL},
        {"SG",Country::SG},{"SK",Country::SK},{"SI",Country::SI},{"SB",Country::SB},
        {"SO",Country::SO},{"ZA",Country::ZA},{"SS",Country::SS},{"ES",Country::ES},
        {"LK",Country::LK},{"SD",Country::SD},{"SR",Country::SR},{"SE",Country::SE},
        {"CH",Country::CH},{"SY",Country::SY},{"TW",Country::TW},{"TJ",Country::TJ},
        {"TZ",Country::TZ},{"TH",Country::TH},{"TL",Country::TL},{"TG",Country::TG},
        {"TO",Country::TO},{"TT",Country::TT},{"TN",Country::TN},{"TR",Country::TR},
        {"TM",Country::TM},{"TV",Country::TV},{"UG",Country::UG},{"UA",Country::UA},
        {"AE",Country::AE},{"GB",Country::GB},{"US",Country::US},{"UY",Country::UY},
        {"UZ",Country::UZ},{"VU",Country::VU},{"VE",Country::VE},{"VN",Country::VN},
        {"YE",Country::YE},{"ZM",Country::ZM},{"ZW",Country::ZW},
    };
    auto it = table.find(name);
    if (it == table.end()) throw IidmException("Unknown country code: " + name);
    return it->second;
}

std::ostream& operator<<(std::ostream& os, Country c) {
    return os << getCountryName(c);
}

} // namespace iidm
