#pragma once

#include <array>
#include <string>
#include <vector>

namespace lise_prompt_rad {

// Direct C++ translation of PromptRadiationTemplate_Expanded_FY2027.xlsx,
// sheet "Template Detailed".
//
// Excel mapping:
//   B:G  -> input copied from LISE++ results: fragment, A, Z, mechanism, rate, energy
//   H    -> scale factor
//   I    -> DB0.x from TemplatePositions by wildcard fragment lookup
//   J    -> stopping location from DB0.x
//   K:X  -> dose rate at each neutron monitor
//   row2 -> monitor totals = SUMIF(dose columns, ">0") + low-ion block

constexpr int kMonitorCount = 14;
constexpr int kLocationCount = 5;
constexpr int kStopBoundaryCount = 6;

extern const std::array<const char*, kMonitorCount> kMonitorNames;
extern const std::array<const char*, kLocationCount> kLocationNames;
extern const std::array<const char*, kStopBoundaryCount> kStopBoundaryNames;

// Factor-array order. Keep this order synchronized with LocationFactorTable columns.
enum class StopLocation {
    P2 = 0,
    P2P3 = 1,
    P3 = 2,
    P4 = 3,
    P5 = 4
};

using LocationFactorTable = std::array<std::array<double, kLocationCount>, kMonitorCount>;

// Ordered DB0.x upper limits used by locationFromDb0xExcel().
// Default values reproduce the Excel IFS formula:
//   x < -310 -> P2
//   x < -170 -> P3
//   x < -100 -> P4
//   x <  100 -> P5
//   x <  170 -> P4
//   x <  310 -> P3
//   else     -> P2
using StopBoundaryTable = std::array<double, kStopBoundaryCount>;

struct GlobalPromptRadiationSettings {
    double scaleModelFactor = 1.2e-7;
    double newShieldingFactor = 1.0;
    double insideOutsideFactor = 500.0;
    double fenceFactor = 0.35;
    double occupancyFactor = 4.0;
    double lightZYieldFactor = 10.0;
    std::string blockNameToCalculate = "BTS02";
    double rateCutoffPps = 1.0e6;
};

// New integration-ready settings object.  LISE++ can own one instance of this class
// and pass it directly to TPromptRadiationCalculator, avoiding hidden mutable globals.
struct TPromptRadiationConfig {
    GlobalPromptRadiationSettings globalSettings;
    LocationFactorTable locationFactors;
    StopBoundaryTable stopBoundaries;

    TPromptRadiationConfig();

    static TPromptRadiationConfig defaultConfig();

    void resetToDefaults();
    bool isValid() const;
};

struct LiseDetailedInput {
    std::string fragment;    // Excel B: e.g. "28Mg"
    int A = 0;               // Excel C
    int Z = 0;               // Excel D
    std::string mechanism;   // Excel E, e.g. "FA"; not used by formulas
    double ratePps = 0.0;    // Excel F
    double energyMeVu = 0.0; // Excel G

    // Reserved for direct LISE++ integration.  The current text-file workflow does
    // not fill these fields, but LISE++ can provide charge-state-resolved rows.
    int qTarget = 0;
    int qWedge = 0;
    bool hasDb0x = false;
    double db0xMm = 0.0;
    double sigmaXmm = 0.0;
};

struct TemplatePosition {
    std::string fragment; // TemplatePositions column B
    double db0xMm = 0.0;  // TemplatePositions column C
    double sigmaXmm = 0.0; // Reserved for direct LISE++ DB0 position results
};

// Manual low-ion block, corresponding to Template Detailed rows 9:13, columns Z:AT.
// ratePps should already include the template's requested multiplier, e.g. H/He x10.
struct LowIonInput {
    std::string name;
    int A = 0;
    int Z = 0;
    double ratePps = 0.0;
    double energyMeVu = 0.0;
    StopLocation location = StopLocation::P2;
};

struct DetailedDoseRow {
    LiseDetailedInput input;
    double scale = 0.0;     // Excel H
    bool hasDb0x = false;   // Excel I found from TemplatePositions or direct LISE++ row
    double db0xMm = 0.0;
    double sigmaXmm = 0.0;  // Reserved for future partial-location analysis
    StopLocation location = StopLocation::P2; // Excel J; defaults to P2 when DB0.x is missing
    std::array<double, kMonitorCount> doseMremPerHr{}; // Excel K:X
};

struct DetailedDoseResult {
    std::vector<DetailedDoseRow> rows;
    std::vector<DetailedDoseRow> lowIonRows;
    std::array<double, kMonitorCount> totalsMremPerHr{}; // Excel row 2, B:O
};

class TPromptRadiationCalculator {
public:
    explicit TPromptRadiationCalculator(const TPromptRadiationConfig& config);

    const TPromptRadiationConfig& config() const;

    StopLocation locationFromDb0x(double db0xMm, bool hasDb0x = true) const;
    double doseRateMremPerHour(double ratePps,
                               double scale,
                               StopLocation location,
                               int monitorIndex) const;

    DetailedDoseResult calculate(
        const std::vector<LiseDetailedInput>& liseRows,
        const std::vector<TemplatePosition>& templatePositions,
        const std::vector<LowIonInput>& lowIons = {},
        double minRatePps = 0.0) const;

private:
    const TPromptRadiationConfig& m_config;
};

// Editable global settings.  These functions are kept for the current standalone
// application and dialogs.  Internally they now operate on one active
// TPromptRadiationConfig object, so LISE++ can move to explicit config ownership later.
const TPromptRadiationConfig& activePromptRadiationConfig();
void setActivePromptRadiationConfig(const TPromptRadiationConfig& config);
void resetActivePromptRadiationConfigToDefaults();

const GlobalPromptRadiationSettings& defaultGlobalSettings();
const GlobalPromptRadiationSettings& globalSettings();
bool areGlobalSettingsValid(const GlobalPromptRadiationSettings& settings);
void setGlobalSettings(const GlobalPromptRadiationSettings& settings);
void resetGlobalSettingsToDefaults();

// Default FY2027 factors from the uploaded PromptRadiationTemplate_Expanded_FY2027.xlsx,
// sheet "Locations", column D, in monitor/location order above.
const LocationFactorTable& defaultLocationFactors();

// Editable active table used by doseRateMremPerHour() and calculateDetailedTemplate().
const LocationFactorTable& locationFactors();
void setLocationFactors(const LocationFactorTable& factors);
void resetLocationFactorsToDefaults();

// Editable active DB0.x boundaries. These should be loaded from / saved to INI.
const StopBoundaryTable& defaultStopLocationBoundaries();
const StopBoundaryTable& stopLocationBoundaries();
bool areStopLocationBoundariesValid(const StopBoundaryTable& boundaries);
void setStopLocationBoundaries(const StopBoundaryTable& boundaries);
void resetStopLocationBoundariesToDefaults();

// Excel H formula, preserving the template exponents 1.7 and 0.333.
// Returns 0 when A, Z, or energy is invalid, matching blank/zero contribution in Excel.
double scaleFactorFromTemplate(int A, int Z, double energyMeVu);

// Excel J formula. If DB0.x is missing/non-numeric, Excel returns "P2".
// This overload uses the editable active stopLocationBoundaries().
StopLocation locationFromDb0xExcel(double db0xMm, bool hasDb0x = true);

// Same ordered inequality formula, but with an explicitly supplied boundary table.
StopLocation locationFromDb0xExcel(double db0xMm,
                                  bool hasDb0x,
                                  const StopBoundaryTable& boundaries);

std::string stopLocationToString(StopLocation loc);
bool stopLocationFromString(const std::string& text, StopLocation& loc);

// Excel VLOOKUP("*" & fragment & "*", TemplatePositions!B:C, 2, 0).
// Returns true if a DB0.x value is found, false otherwise.
bool findDb0xByFragment(const std::string& fragment,
                        const std::vector<TemplatePosition>& positions,
                        double& db0xMm);

// Direct LISE++ integration helper: locate DB0.x and sigma(x) when available.
bool findDb0PositionByFragment(const std::string& fragment,
                               const std::vector<TemplatePosition>& positions,
                               double& db0xMm,
                               double& sigmaXmm);

// Dose formula for each monitor column:
//   dose = ratePps / 1e8 * scale * LocationFactor[monitor][location]
double doseRateMremPerHour(double ratePps,
                           double scale,
                           StopLocation location,
                           int monitorIndex);

// Same formula, but using an explicitly supplied factors table.
double doseRateMremPerHour(double ratePps,
                           double scale,
                           StopLocation location,
                           int monitorIndex,
                           const LocationFactorTable& factors);

// Main calculation for the "Template Detailed" sheet.
// minRatePps is optional; use 0 to reproduce the spreadsheet exactly.
DetailedDoseResult calculateDetailedTemplate(
    const std::vector<LiseDetailedInput>& liseRows,
    const std::vector<TemplatePosition>& templatePositions,
    const std::vector<LowIonInput>& lowIons = {},
    double minRatePps = 0.0);

// Same calculation with an explicitly supplied config object for future LISE++ integration.
DetailedDoseResult calculateDetailedTemplate(
    const TPromptRadiationConfig& config,
    const std::vector<LiseDetailedInput>& liseRows,
    const std::vector<TemplatePosition>& templatePositions,
    const std::vector<LowIonInput>& lowIons = {},
    double minRatePps = 0.0);

// Example/default FY2027 low-ion rows from the uploaded template.
// The rates are already the starred pps values from column AC.
std::vector<LowIonInput> defaultLowIonRowsFY2027();

} // namespace lise_prompt_rad
