# PromptRadiationQt

Standalone Qt Widgets example for the LISE++ prompt-radiation detailed-template calculation.

## What it contains

- `PromptRadiationDetailed.*` — C++ translation of the Excel `Template Detailed` calculations.
- `LocationFactorsModel.*` — editable Qt table model for the `kLocationFactors` values.
- `GlobalSettingsDialog.*` — editor for global prompt-radiation variables loaded/saved in the INI file.
- `LocationFactorsDialog.*` — table dialog with copy/paste and reset-to-defaults.
- `StopBoundariesDialog.*` — editor for the DB0.x stopping-boundary limits used by the P2/P3/P4/P5 assignment.
- `MainWindow.*` — menu actions under `File` and `Calculations`:
  - `File / Prompt radiation: load INI...`
  - `File / Prompt radiation: save INI as...`
  - `File / Prompt radiation: use default INI`
  - `Calculations / Prompt radiation: edit global settings...`
  - `Calculations / Prompt radiation: edit location factors...`
  - `Prompt radiation: edit DB0.x stopping boundaries...`
  - `Prompt radiation: run small example`
  - `Prompt radiation: read these files and run`
  - `Prompt radiation: reset global settings`
  - `Prompt radiation: reset location factors`
  - `Prompt radiation: reset DB0.x stopping boundaries`



## Text input files from Excel

Two tab-delimited text files are included, copied from `PromptRadiationTemplate_Expanded_FY2027.xlsx`:

```text
PromtRadiation_InputYields.txt     <- sheet "Template Detailed", B4:G100
PromtRadiation_InputPositions.txt  <- sheet "TemplatePositions", B4:G100
```

The menu command

```text
Calculations / Prompt radiation: read these files and run
```

searches for these files next to the executable, in the current working directory, and in the project source directory. It reads the yield rows, reads DB0.x positions, applies the current INI settings, runs `calculateDetailedTemplate(...)`, and prints monitor totals plus the main N0318 contributors.

The filenames intentionally use `PromtRadiation_...` to match the requested names.

## Global settings

The project now has editable global prompt-radiation parameters:

```cpp
struct GlobalPromptRadiationSettings {
    double scaleModelFactor = 1.2e-7;
    double newShieldingFactor = 1.0;
    double insideOutsideFactor = 500.0;
    double fenceFactor = 0.35;
    double occupancyFactor = 4.0;
    double lightZYieldFactor = 10.0;
    std::string blockNameToCalculate = "DB0";
    double rateCutoffPps = 1.0e6;
};
```

They are available through:

```cpp
const GlobalPromptRadiationSettings& defaultGlobalSettings();
const GlobalPromptRadiationSettings& globalSettings();
void setGlobalSettings(const GlobalPromptRadiationSettings& settings);
void resetGlobalSettingsToDefaults();
```

Menu command:

```text
Calculations / Prompt radiation: edit global settings...
```

These values are loaded from and saved to the active INI file. Some are not yet used by the small example calculation, but are ready for the later LISE++ prompt-radiation utility integration.

## DB0.x boundary logic

The old hard-coded logic was:

```cpp
if (db0xMm < -310.0) return StopLocation::P2;
if (db0xMm < -170.0) return StopLocation::P3;
if (db0xMm < -100.0) return StopLocation::P4;
if (db0xMm <  100.0) return StopLocation::P5;
if (db0xMm <  170.0) return StopLocation::P4;
if (db0xMm <  310.0) return StopLocation::P3;
return StopLocation::P2;
```

Now these six limits are stored in an editable table:

```cpp
using StopBoundaryTable = std::array<double, 6>;

const StopBoundaryTable& stopLocationBoundaries();
void setStopLocationBoundaries(const StopBoundaryTable& boundaries);
void resetStopLocationBoundariesToDefaults();
```

The active limits are used automatically by:

```cpp
lise_prompt_rad::locationFromDb0xExcel(db0xMm, hasDb0x);
lise_prompt_rad::calculateDetailedTemplate(...);
```

The dialog validates that the six values are strictly increasing.

## INI file

The example uses an explicit INI file, not the Windows registry:

```cpp
QSettings settings(settingsFilePath(), QSettings::IniFormat);
```

By default the file is created next to the executable:

```text
PromptRadiation.ini
```

The user can also load and save a different INI file from the `File` menu. After loading or saving-as another file, later edits are written to that selected file. `File / Prompt radiation: use default INI` switches back to the default `PromptRadiation.ini` next to the executable.

It stores three groups. Global settings and location factors are written with simple keys so they are easy to edit by hand:

```ini
[GlobalSettings]
ScaleModelFactor=1.2e-7
NewShieldingFactor=1
InsideOutsideFactor=500
Fence=0.35
Occupancy=4
LightZYieldFactor=10
BlockNameToCalculate=DB0
RateCutoff=1e6

[LocationFactors]
N0318_P2=0.16
N0318_P2P3=0.20333333333333334
N0318_P3=0.29
N0318_P4=0.27
N0318_P5=0.015
...

[StopLocationBoundaries]
Limit1=-310
Limit2=-170
Limit3=-100
Limit4=100
Limit5=170
Limit6=310
Rule=x<Limit1:P2, x<Limit2:P3, x<Limit3:P4, x<Limit4:P5, x<Limit5:P4, x<Limit6:P3, else:P2
```

The source still keeps the hard-coded defaults in `PromptRadiationDetailed.cpp` as `kDefaultGlobalSettings`, `kDefaultLocationFactors`, and `kDefaultStopLocationBoundaries`. If the INI file is missing or incomplete, those source defaults are used. Pressing `reset global settings`, `reset location factors`, or `reset DB0.x stopping boundaries` restores the source defaults and writes them back to the INI file.

For direct LISE++ integration, change only `MainWindow::defaultSettingsFilePath()` if you want to use an existing LISE++ settings directory. The currently selected file is stored in `m_settingsFilePath` during the session.

## Build in Qt Creator

Open `PromptRadiationQt.pro`, select a Desktop Qt kit, then Build/Run.

## Integration into LISE++

Move the files into LISE++ and connect the actions like in `MainWindow::createMenus()`:

```cpp
QAction* editGlobalSettingsAction = calcMenu->addAction(tr("Prompt radiation: edit global settings..."));
connect(editGlobalSettingsAction, &QAction::triggered, this, &MainWindow::editGlobalSettings);

QAction* editFactorsAction = calcMenu->addAction(tr("Prompt radiation: edit location factors..."));
connect(editFactorsAction, &QAction::triggered, this, &MainWindow::editLocationFactors);

QAction* editBoundariesAction = calcMenu->addAction(tr("Prompt radiation: edit DB0.x stopping boundaries..."));
connect(editBoundariesAction, &QAction::triggered, this, &MainWindow::editStopLocationBoundaries);
```

Load settings once before running the calculation, and save after the user accepts either editor dialog.

## Icons

The project now includes a small `Icons/` folder copied from the LISE++ icon archive:

- `sign_ok.png` for OK buttons
- `cancel.png` for Cancel buttons
- `copy.png` for Copy buttons
- `paste1.png` for Paste buttons
- `lisepp_small.png` as the runtime application/window/dialog icon
- `quit.png` for the Quit menu action
- `trans.gif` for the run-example menu action
- `lise_small_new.ico` as the Windows executable icon through `RC_ICONS`

The Qt resource file is `PromptRadiationResources.qrc`. Dialog buttons use the resource paths such as `:/Icons/sign_ok.png`.


## Menu icons

The menu action **Prompt radiation: run small example** uses `Icons/trans.gif`.
The **Quit** action uses `Icons/quit.png`, copied from the closest available red-X/cancel icon in `Icons.zip`.
