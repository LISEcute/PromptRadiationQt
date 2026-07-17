#include "PromptRadiationDetailed.h"

#include <algorithm>
#include <cmath>
#include <cctype>

namespace lise_prompt_rad {

const std::array<const char*, kMonitorCount> kMonitorNames = {
    "N0318", "N0306", "N0319", "N0325A", "N0325B", "N0304B", "N0314",
    "N0305", "N0308B", "N0322", "N0323", "N0324", "N0304", "N0315"
};

const std::array<const char*, kLocationCount> kLocationNames = {
    "P2", "P2/P3", "P3", "P4", "P5"
};

const std::array<const char*, kStopBoundaryCount> kStopBoundaryNames = {
    "P2 left  (x < limit)",
    "P3 left  (x < limit)",
    "P4 left  (x < limit)",
    "P5 center (x < limit)",
    "P4 right (x < limit)",
    "P3 right (x < limit)"
};

static const LocationFactorTable kDefaultLocationFactors = {{
    {{0.16,      0.20333333333333334, 0.29,     0.27,     0.015}},   // N0318
    {{7.8e-05,   0.001452,            0.0042,   0.016,    0.015}},   // N0306
    {{1.6e-05,   0.000264,            0.00076,  0.0019,   0.03}},    // N0319
    {{0.58,      0.68,                0.88,     4.5,      0.1}},     // N0325A
    {{0.097,     0.14466666666666667, 0.24,     0.29,     0.022}},   // N0325B
    {{0.073,     0.132,               0.25,     0.94,     0.62}},    // N0304B
    {{0.0042,    0.011466666666666667,0.026,    0.11,     37.1}},    // N0314
    {{0.0042,    0.011466666666666667,0.026,    0.11,     37.1}},    // N0305; template block label says N0315 but formula uses it for N0305
    {{0.027,     0.08133333333333333, 0.19,     0.94,     20.3}},    // N0308B
    {{0.0012,    0.0058,              0.015,    0.012,    0.0083}},  // N0322
    {{0.0008,    0.013533333333333333,0.039,    0.0012,   0.007}},   // N0323
    {{0.0012,    0.0008116666666666666,3.5e-05,0.0002,   6.8e-05}}, // N0324
    {{1.5e-06,   4.0e-06,             9.0e-06,  8.5e-05,  0.0019}},  // N0304
    {{0.0021,    0.0016266666666666667,0.00068,0.00057,  1.6e-05}}  // N0315
}};

static const StopBoundaryTable kDefaultStopLocationBoundaries = {{
    -310.0, -170.0, -100.0, 100.0, 170.0, 310.0
}};

static LocationFactorTable gLocationFactors = kDefaultLocationFactors;
static StopBoundaryTable gStopLocationBoundaries = kDefaultStopLocationBoundaries;

static const GlobalPromptRadiationSettings kDefaultGlobalSettings = {
    1.2e-7, // Scale Model Factor
    1.0,   // New Shielding Factor
    500.0, // Inside/Outside factor
    0.35,  // Fence
    4.0,   // Occupancy
    10.0,  // Light Z yield factor
    "DB0", // Block name to calculate
    1.0e6  // Rate cutoff [pps]
};

static GlobalPromptRadiationSettings gGlobalSettings = kDefaultGlobalSettings;

const GlobalPromptRadiationSettings& defaultGlobalSettings()
{
    return kDefaultGlobalSettings;
}
//--------------------------------------------------------------------------------

const GlobalPromptRadiationSettings& globalSettings()
{
    return gGlobalSettings;
}
//--------------------------------------------------------------------------------

bool areGlobalSettingsValid(const GlobalPromptRadiationSettings& settings)
{
    return std::isfinite(settings.scaleModelFactor)
        && std::isfinite(settings.newShieldingFactor)
        && std::isfinite(settings.insideOutsideFactor)
        && std::isfinite(settings.fenceFactor)
        && std::isfinite(settings.occupancyFactor)
        && std::isfinite(settings.lightZYieldFactor)
        && std::isfinite(settings.rateCutoffPps)
        && std::any_of(settings.blockNameToCalculate.begin(),
                       settings.blockNameToCalculate.end(),
                       [](unsigned char ch) { return !std::isspace(ch); });
}
//--------------------------------------------------------------------------------

void setGlobalSettings(const GlobalPromptRadiationSettings& settings)
{
    gGlobalSettings = areGlobalSettingsValid(settings) ? settings : kDefaultGlobalSettings;
}
//--------------------------------------------------------------------------------

void resetGlobalSettingsToDefaults()
{
    gGlobalSettings = kDefaultGlobalSettings;
}
//--------------------------------------------------------------------------------

const LocationFactorTable& defaultLocationFactors()
{
    return kDefaultLocationFactors;
}
//--------------------------------------------------------------------------------

const LocationFactorTable& locationFactors()
{
    return gLocationFactors;
}
//--------------------------------------------------------------------------------

void setLocationFactors(const LocationFactorTable& factors)
{
    gLocationFactors = factors;
}
//--------------------------------------------------------------------------------

void resetLocationFactorsToDefaults()
{
    gLocationFactors = kDefaultLocationFactors;
}
//--------------------------------------------------------------------------------

const StopBoundaryTable& defaultStopLocationBoundaries()
{
    return kDefaultStopLocationBoundaries;
}
//--------------------------------------------------------------------------------

const StopBoundaryTable& stopLocationBoundaries()
{
    return gStopLocationBoundaries;
}
//--------------------------------------------------------------------------------

bool areStopLocationBoundariesValid(const StopBoundaryTable& boundaries)
{
    for (int i = 1; i < kStopBoundaryCount; ++i) {
        if (!std::isfinite(boundaries[i - 1]) || !std::isfinite(boundaries[i]) ||
            boundaries[i - 1] >= boundaries[i]) {
            return false;
        }
    }
    return true;
}
//--------------------------------------------------------------------------------

void setStopLocationBoundaries(const StopBoundaryTable& boundaries)
{
    gStopLocationBoundaries = areStopLocationBoundariesValid(boundaries)
                              ? boundaries
                              : kDefaultStopLocationBoundaries;
}
//--------------------------------------------------------------------------------

void resetStopLocationBoundariesToDefaults()
{
    gStopLocationBoundaries = kDefaultStopLocationBoundaries;
}
//--------------------------------------------------------------------------------

static std::string lowerAscii(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return s;
}
//--------------------------------------------------------------------------------

static std::string trimAscii(const std::string& s)
{
    std::size_t first = 0;
    while (first < s.size() && std::isspace(static_cast<unsigned char>(s[first]))) {
        ++first;
    }
    std::size_t last = s.size();
    while (last > first && std::isspace(static_cast<unsigned char>(s[last - 1]))) {
        --last;
    }
    return s.substr(first, last - first);
}
//--------------------------------------------------------------------------------

double scaleFactorFromTemplate(int A, int Z, double energyMeVu)
{
    if (A <= 0 || Z <= 0 || energyMeVu <= 0.0) {
        return 0.0;
    }

    const double a = static_cast<double>(A);
    const double z = static_cast<double>(Z);

    // Excel H formula:
    // POWER(E/160,1.7) * POWER(Z/20,-1.7) * (A/48)
    // * (POWER(POWER(A,0.333),2) / POWER(POWER(48,0.333),2))
    // * POWER(A/20,0.333)
    return std::pow(energyMeVu / 160.0, 1.7)
         * std::pow(z / 20.0, -1.7)
         * (a / 48.0)
         * (std::pow(std::pow(a, 0.333), 2.0) /
            std::pow(std::pow(48.0, 0.333), 2.0))
         * std::pow(a / 20.0, 0.333);
}
//--------------------------------------------------------------------------------

StopLocation locationFromDb0xExcel(double db0xMm, bool hasDb0x)
{
    return locationFromDb0xExcel(db0xMm, hasDb0x, gStopLocationBoundaries);
}
//--------------------------------------------------------------------------------

StopLocation locationFromDb0xExcel(double db0xMm,
                                  bool hasDb0x,
                                  const StopBoundaryTable& boundaries)
{
    if (!hasDb0x || !std::isfinite(db0xMm)) {
        return StopLocation::P2;
    }

    const StopBoundaryTable& b = areStopLocationBoundariesValid(boundaries)
                                 ? boundaries
                                 : kDefaultStopLocationBoundaries;

    // Ordered inequalities from Excel IFS formula in column J, now editable:
    // b[0] -> P2, b[1] -> P3, b[2] -> P4, b[3] -> P5, b[4] -> P4, b[5] -> P3.
    if (db0xMm < b[0]) return StopLocation::P2;
    if (db0xMm < b[1]) return StopLocation::P3;
    if (db0xMm < b[2]) return StopLocation::P4;
    if (db0xMm < b[3]) return StopLocation::P5;
    if (db0xMm < b[4]) return StopLocation::P4;
    if (db0xMm < b[5]) return StopLocation::P3;
    return StopLocation::P2;
}
//--------------------------------------------------------------------------------

std::string stopLocationToString(StopLocation loc)
{
    switch (loc) {
    case StopLocation::P2:   return "P2";
    case StopLocation::P2P3: return "P2/P3";
    case StopLocation::P3:   return "P3";
    case StopLocation::P4:   return "P4";
    case StopLocation::P5:   return "P5";
    }
    return "P2";
}
//--------------------------------------------------------------------------------

bool stopLocationFromString(const std::string& text, StopLocation& loc)
{
    const std::string s = lowerAscii(trimAscii(text));
    if (s == "p2")    { loc = StopLocation::P2;   return true; }
    if (s == "p2/p3") { loc = StopLocation::P2P3; return true; }
    if (s == "p3")    { loc = StopLocation::P3;   return true; }
    if (s == "p4")    { loc = StopLocation::P4;   return true; }
    if (s == "p5")    { loc = StopLocation::P5;   return true; }
    return false;
}
//--------------------------------------------------------------------------------

bool findDb0xByFragment(const std::string& fragment,
                        const std::vector<TemplatePosition>& positions,
                        double& db0xMm)
{
    const std::string key = lowerAscii(trimAscii(fragment));
    if (key.empty()) {
        return false;
    }

    // Mimic Excel VLOOKUP("*fragment*", TemplatePositions!B:C, 2, 0):
    // return the first row where column B contains the fragment string.
    for (const TemplatePosition& p : positions) {
        const std::string name = lowerAscii(p.fragment);
        if (name.find(key) != std::string::npos) {
            db0xMm = p.db0xMm;
            return true;
        }
    }
    return false;
}
//--------------------------------------------------------------------------------

double doseRateMremPerHour(double ratePps,
                           double scale,
                           StopLocation location,
                           int monitorIndex,
                           const LocationFactorTable& factors)
{
    if (monitorIndex < 0 || monitorIndex >= kMonitorCount) {
        return 0.0;
    }
    if (ratePps <= 0.0 || scale <= 0.0) {
        return 0.0;
    }

    const int locationIndex = static_cast<int>(location);
    if (locationIndex < 0 || locationIndex >= kLocationCount) {
        return 0.0;
    }

    return (ratePps / 1.0e8) * scale * factors[monitorIndex][locationIndex];
}
//--------------------------------------------------------------------------------

double doseRateMremPerHour(double ratePps,
                           double scale,
                           StopLocation location,
                           int monitorIndex)
{
    return doseRateMremPerHour(ratePps, scale, location, monitorIndex, gLocationFactors);
}
//--------------------------------------------------------------------------------

static DetailedDoseRow calculateOneLiseRow(const LiseDetailedInput& input,
                                           const std::vector<TemplatePosition>& templatePositions)
{
    DetailedDoseRow out;
    out.input = input;
    out.scale = scaleFactorFromTemplate(input.A, input.Z, input.energyMeVu);

    out.hasDb0x = findDb0xByFragment(input.fragment, templatePositions, out.db0xMm);
    out.location = locationFromDb0xExcel(out.db0xMm, out.hasDb0x);

    for (int i = 0; i < kMonitorCount; ++i) {
        out.doseMremPerHr[i] = doseRateMremPerHour(input.ratePps, out.scale, out.location, i);
    }
    return out;
}
//--------------------------------------------------------------------------------

static DetailedDoseRow calculateOneLowIonRow(const LowIonInput& input)
{
    LiseDetailedInput row;
    row.fragment = input.name;
    row.A = input.A;
    row.Z = input.Z;
    row.ratePps = input.ratePps;
    row.energyMeVu = input.energyMeVu;
    row.mechanism = "low";

    DetailedDoseRow out;
    out.input = row;
    out.scale = scaleFactorFromTemplate(input.A, input.Z, input.energyMeVu);
    out.hasDb0x = false;
    out.db0xMm = 0.0;
    out.location = input.location;

    for (int i = 0; i < kMonitorCount; ++i) {
        out.doseMremPerHr[i] = doseRateMremPerHour(input.ratePps, out.scale, out.location, i);
    }
    return out;
}
//--------------------------------------------------------------------------------

DetailedDoseResult calculateDetailedTemplate(
    const std::vector<LiseDetailedInput>& liseRows,
    const std::vector<TemplatePosition>& templatePositions,
    const std::vector<LowIonInput>& lowIons,
    double minRatePps)
{
    DetailedDoseResult result;
    result.totalsMremPerHr.fill(0.0);

    for (const LiseDetailedInput& input : liseRows) {
        if (input.fragment.empty() || input.ratePps < minRatePps) {
            continue;
        }

        DetailedDoseRow out = calculateOneLiseRow(input, templatePositions);
        result.rows.push_back(out);

        // Excel row 2 uses SUMIF(column, ">0").
        for (int i = 0; i < kMonitorCount; ++i) {
            if (out.doseMremPerHr[i] > 0.0) {
                result.totalsMremPerHr[i] += out.doseMremPerHr[i];
            }
        }
    }

    for (const LowIonInput& input : lowIons) {
        DetailedDoseRow out = calculateOneLowIonRow(input);
        result.lowIonRows.push_back(out);

        for (int i = 0; i < kMonitorCount; ++i) {
            if (out.doseMremPerHr[i] > 0.0) {
                result.totalsMremPerHr[i] += out.doseMremPerHr[i];
            }
        }
    }

    return result;
}
//--------------------------------------------------------------------------------

std::vector<LowIonInput> defaultLowIonRowsFY2027()
{
    // These are rows 9:13 from the uploaded FY2027 template.
    // Column AC is "Enter pps*"; the note says multiply LISE estimate by 10.
    return {
        {"H-3 low",  3, 1, 1.010e9, 218.51, StopLocation::P2},
        {"H-2 low",  2, 1, 1.630e9, 223.40, StopLocation::P5},
        {"He-4 low", 4, 2, 2.350e9, 215.65, StopLocation::P2},
        {"H-1 low",  1, 1, 0.0,     0.0,    StopLocation::P4},
        {"He-3 low", 3, 2, 7.500e7, 214.63, StopLocation::P3}
    };
}
//--------------------------------------------------------------------------------

} // namespace lise_prompt_rad
