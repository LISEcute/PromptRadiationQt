#include "MainWindowPR.h"

#include "d_promtRad_GlobalSettings.h"
#include "d_promtRad_LocationFactors.h"
#include "d_promtRad_MonitorLocations.h"
#include "d_promtRad_StopBoundaries.h"
#include "o_promptRad_Detailed.h"
#include "o_promptRad_UiHelpers.h"

#include <QAction>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QTextEdit>
#include <QTextStream>

#include <algorithm>
#include <vector>

//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_log(new QTextEdit(this)),
      m_settingsFilePath(defaultSettingsFilePath())
{
    setWindowTitle(tr("ARIS Prompt Radiation Utility"));
    setWindowIcon(QIcon(QStringLiteral(":/Icons/lisepp_small.png")));
    resize(950, 620);

    QFont fixedWidthFont(QStringLiteral("Courier New"));
    fixedWidthFont.setStyleHint(QFont::Monospace);
    fixedWidthFont.setPointSize(10);

    m_log->setReadOnly(true);
    m_log->setFont(fixedWidthFont);
    setCentralWidget(m_log);

    loadPromptRadiationSettings();
    createMenus();

    m_log->append(tr("Prompt-radiation settings loaded from:\n  %1").arg(settingsFilePath()));
    printGlobalSettingsSummary();
    printLocationFactorSummary();
    printStopBoundarySummary();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* loadIniAction = fileMenu->addAction(tr("Prompt radiation: load INI..."));
    connect(loadIniAction, &QAction::triggered, this, &MainWindow::loadSettingsFromIniFile);

    QAction* saveIniAsAction = fileMenu->addAction(tr("Prompt radiation: save INI as..."));
    connect(saveIniAsAction, &QAction::triggered, this, &MainWindow::saveSettingsAsIniFile);

    QAction* useDefaultIniAction = fileMenu->addAction(tr("Prompt radiation: use default INI"));
    connect(useDefaultIniAction, &QAction::triggered, this, &MainWindow::useDefaultIniFile);

    fileMenu->addSeparator();

    QAction* monitorLocationsAction = fileMenu->addAction(tr("Monitors location table"));
    connect(monitorLocationsAction, &QAction::triggered, this, &MainWindow::showMonitorLocations);

    fileMenu->addSeparator();

    QAction* quitAction = fileMenu->addAction(QIcon(QStringLiteral(":/Icons/quit.png")), tr("Quit"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    //**************************************************************************
    // These are the actions you can move into the existing LISE++ menu.
    QMenu* calcOptions = menuBar()->addMenu(tr("&Options"));

    QAction* editGlobalSettingsAction = calcOptions->addAction(tr("Edit global settings..."));
    connect(editGlobalSettingsAction, &QAction::triggered, this, &MainWindow::editGlobalSettings);

    QAction* editFactorsAction = calcOptions->addAction(tr("Edit location factors..."));
    connect(editFactorsAction, &QAction::triggered, this, &MainWindow::editLocationFactors);

    QAction* editBoundariesAction = calcOptions->addAction(tr("Edit DB0.x stopping boundaries..."));
    connect(editBoundariesAction, &QAction::triggered, this, &MainWindow::editStopLocationBoundaries);


    calcOptions->addSeparator();

    QAction* resetGlobalSettingsAction = calcOptions->addAction(tr("Reset global settings"));
    connect(resetGlobalSettingsAction, &QAction::triggered, this, &MainWindow::resetGlobalSettings);

    QAction* resetFactorsAction = calcOptions->addAction(tr("Reset location factors"));
    connect(resetFactorsAction, &QAction::triggered, this, &MainWindow::resetLocationFactors);

    QAction* resetBoundariesAction = calcOptions->addAction(tr("Reset DB0.x stopping boundaries"));
    connect(resetBoundariesAction, &QAction::triggered, this, &MainWindow::resetStopLocationBoundaries);

    //**************************************************************************

    QMenu* calcMenu = menuBar()->addMenu(tr("&Calculations"));



    QAction* readFilesAndRunAction = calcMenu->addAction(QIcon(QStringLiteral(":/Icons/trans.gif")),
                                                         tr("Read example Yield and Position files and RUN"));
    connect(readFilesAndRunAction, &QAction::triggered, this, &MainWindow::readInputFilesAndRun);

    QAction* runExampleAction = calcMenu->addAction(QIcon(QStringLiteral(":/Icons/trans.gif")),
                                                    tr("Run small example"));
    connect(runExampleAction, &QAction::triggered, this, &MainWindow::runSmallExample);

}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

QString MainWindow::defaultSettingsFilePath() const
{
    // Explicit default INI file, not the Windows registry. For direct LISE++ integration,
    // change only this path if you prefer the existing LISE++ settings directory.
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("PromptRadiation.ini"));
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

QString MainWindow::settingsFilePath() const
{
    return m_settingsFilePath.isEmpty() ? defaultSettingsFilePath() : m_settingsFilePath;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::loadSettingsFromIniFile()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Load prompt-radiation INI"),
        QFileInfo(settingsFilePath()).absolutePath(),
        tr("INI files (*.ini);;All files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    m_settingsFilePath = fileName;
    loadPromptRadiationSettings();

    m_log->append(tr("\nPrompt-radiation settings loaded from selected INI:\n  %1")
                  .arg(settingsFilePath()));
    printGlobalSettingsSummary();
    printLocationFactorSummary();
    printStopBoundarySummary();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::saveSettingsAsIniFile()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save prompt-radiation INI as"),
        settingsFilePath(),
        tr("INI files (*.ini);;All files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    if (QFileInfo(fileName).suffix().isEmpty()) {
        fileName += QStringLiteral(".ini");
    }

    m_settingsFilePath = fileName;
    savePromptRadiationSettings();

    m_log->append(tr("\nPrompt-radiation settings saved to selected INI:\n  %1")
                  .arg(settingsFilePath()));
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::useDefaultIniFile()
{
    m_settingsFilePath = defaultSettingsFilePath();
    loadPromptRadiationSettings();

    m_log->append(tr("\nPrompt-radiation settings switched back to default INI:\n  %1")
                  .arg(settingsFilePath()));
    printGlobalSettingsSummary();
    printLocationFactorSummary();
    printStopBoundarySummary();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::showMonitorLocations()
{
    T_PradiationMonitorLocationsDlg dlg(this);
    dlg.exec();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::editGlobalSettings()
{
    T_PradiationGlobalSettingsDlg dlg(this);
    dlg.setSettings(lise_prompt_rad::globalSettings());

    if (dlg.exec() == QDialog::Accepted) {
        lise_prompt_rad::setGlobalSettings(dlg.settings());
        savePromptRadiationSettings();
        m_log->append(tr("\nGlobal prompt-radiation settings updated and saved to INI:\n  %1")
                      .arg(settingsFilePath()));
        printGlobalSettingsSummary();
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::editLocationFactors()
{
    T_PradiationLocationFactorsDlg dlg(this);
    dlg.setFactors(lise_prompt_rad::locationFactors());

    if (dlg.exec() == QDialog::Accepted) {
        lise_prompt_rad::setLocationFactors(dlg.factors());
        savePromptRadiationSettings();
        m_log->append(tr("\nLocation factors updated from table editor and saved to INI:\n  %1")
                      .arg(settingsFilePath()));
        printLocationFactorSummary();
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::editStopLocationBoundaries()
{
    T_PradiationStopBoundariesDlg dlg(this);
    dlg.setBoundaries(lise_prompt_rad::stopLocationBoundaries());

    if (dlg.exec() == QDialog::Accepted) {
        lise_prompt_rad::setStopLocationBoundaries(dlg.boundaries());
        savePromptRadiationSettings();
        m_log->append(tr("\nDB0.x stopping boundaries updated and saved to INI:\n  %1")
                      .arg(settingsFilePath()));
        printStopBoundarySummary();
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::resetGlobalSettings()
{
    lise_prompt_rad::resetGlobalSettingsToDefaults();
    savePromptRadiationSettings();
    m_log->append(tr("\nGlobal prompt-radiation settings reset to source defaults and saved to INI:\n  %1")
                  .arg(settingsFilePath()));
    printGlobalSettingsSummary();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::resetLocationFactors()
{
    lise_prompt_rad::resetLocationFactorsToDefaults();
    savePromptRadiationSettings();
    m_log->append(tr("\nLocation factors reset to FY2027 template defaults and saved to INI:\n  %1")
                  .arg(settingsFilePath()));
    printLocationFactorSummary();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::resetStopLocationBoundaries()
{
    lise_prompt_rad::resetStopLocationBoundariesToDefaults();
    savePromptRadiationSettings();
    m_log->append(tr("\nDB0.x stopping boundaries reset to Excel/FY2027 defaults and saved to INI:\n  %1")
                  .arg(settingsFilePath()));
    printStopBoundarySummary();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::runSmallExample()
{
    using namespace lise_prompt_rad;

    // Tiny placeholder example only. Replace these vectors with real LISE++ rows:
    // B:G from "Template Detailed" and B:C from "TemplatePositions".
    const std::vector<LiseDetailedInput> liseRows = {
        {"28Mg", 28, 12, "PF", 1.0e8, 175.0},
        {"31Al", 31, 13, "PF", 2.0e7, 170.0}
    };

    const std::vector<TemplatePosition> positions = {
        {"28Mg", -260.0},
        {"31Al", 40.0}
    };

    const DetailedDoseResult result = calculateDetailedTemplate(
        liseRows,
        positions,
        defaultLowIonRowsFY2027(),
        globalSettings().rateCutoffPps);

    m_log->append(tr("\nSmall prompt-radiation example, including default low-ion rows:"));
    m_log->append(QString::fromLatin1("  Rate cutoff = %1 pps")
                  .arg(globalSettings().rateCutoffPps, 0, 'e', 2));
    for (int i = 0; i < kMonitorCount; ++i) {
        m_log->append(QString::fromLatin1("  %1 : %2 mrem/h")
                      .arg(QString::fromLatin1(kMonitorNames[i]), -7)
                      .arg(result.totalsMremPerHr[i], 0, 'g', 8));
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww


void MainWindow::readInputFilesAndRun()
{
    using namespace lise_prompt_rad;

    const QString yieldsPath = prompt_rad_ui::findDefaultInputFile(QStringLiteral("PromtRadiation_InputYields.txt"));
    const QString positionsPath = prompt_rad_ui::findDefaultInputFile(QStringLiteral("PromtRadiation_InputPositions.txt"));

    QStringList messages;
    const std::vector<LiseDetailedInput> liseRows = prompt_rad_ui::readYieldInputFile(yieldsPath, messages);
    const std::vector<TemplatePosition> positions = prompt_rad_ui::readPositionInputFile(positionsPath, messages);

    m_log->append(tr("\nPrompt-radiation calculation from text input files:"));
    m_log->append(tr("  Yields file    : %1").arg(yieldsPath));
    m_log->append(tr("  Positions file : %1").arg(positionsPath));
    for (const QString& message : messages) {
        m_log->append(QStringLiteral("  ") + message);
    }

    if (liseRows.empty() || positions.empty()) {
        m_log->append(tr("  Calculation stopped: one or both input files did not provide usable rows."));
        return;
    }

    const DetailedDoseResult result = calculateDetailedTemplate(
        liseRows,
        positions,
        defaultLowIonRowsFY2027(),
        globalSettings().rateCutoffPps);

    m_log->append(QString::fromLatin1("  Rate cutoff    : %1 pps")
                  .arg(globalSettings().rateCutoffPps, 0, 'e', 2));
    m_log->append(QString::fromLatin1("  Rows after cutoff: %1 detailed + %2 low-ion")
                  .arg(static_cast<int>(result.rows.size()))
                  .arg(static_cast<int>(result.lowIonRows.size())));

    m_log->append(tr("\nMonitor totals:"));
    prompt_rad_ui::appendMonitorTotals(m_log, result);
    prompt_rad_ui::appendTopContributors(m_log, result);
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::loadPromptRadiationSettings()
{
    loadPromptRadiationSettingsFromFile(settingsFilePath());

    // Create/update the INI on first run so the user can edit it by hand if needed.
    savePromptRadiationSettings();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::savePromptRadiationSettings() const
{
    savePromptRadiationSettingsToFile(settingsFilePath());
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::loadPromptRadiationSettingsFromFile(const QString& filePath)
{
    QSettings settings(filePath, QSettings::IniFormat);

    lise_prompt_rad::GlobalPromptRadiationSettings global = lise_prompt_rad::defaultGlobalSettings();
    settings.beginGroup("GlobalSettings");
    global.scaleModelFactor = settings.value("ScaleModelFactor", global.scaleModelFactor).toDouble();
    global.newShieldingFactor = settings.value("NewShieldingFactor", global.newShieldingFactor).toDouble();
    global.insideOutsideFactor = settings.value("InsideOutsideFactor", global.insideOutsideFactor).toDouble();
    global.fenceFactor = settings.value("Fence", global.fenceFactor).toDouble();
    global.occupancyFactor = settings.value("Occupancy", global.occupancyFactor).toDouble();
    global.lightZYieldFactor = settings.value("LightZYieldFactor", global.lightZYieldFactor).toDouble();
    global.blockNameToCalculate = settings.value("BlockNameToCalculate",
                                                  QString::fromStdString(global.blockNameToCalculate)).toString().toStdString();
    global.rateCutoffPps = settings.value("RateCutoff", global.rateCutoffPps).toDouble();
    settings.endGroup();
    lise_prompt_rad::setGlobalSettings(global);

    lise_prompt_rad::LocationFactorTable factors = lise_prompt_rad::defaultLocationFactors();

    settings.beginGroup("LocationFactors");
    for (int r = 0; r < lise_prompt_rad::kMonitorCount; ++r) {
        for (int c = 0; c < lise_prompt_rad::kLocationCount; ++c) {
            const QString key = prompt_rad_ui::locationFactorKey(r, c);
            const QString legacyKey = prompt_rad_ui::legacyLocationFactorKey(r, c);
            factors[r][c] = settings.value(key, settings.value(legacyKey, factors[r][c])).toDouble();
        }
    }
    settings.endGroup();
    lise_prompt_rad::setLocationFactors(factors);

    lise_prompt_rad::StopBoundaryTable boundaries = lise_prompt_rad::defaultStopLocationBoundaries();
    settings.beginGroup("StopLocationBoundaries");
    for (int i = 0; i < lise_prompt_rad::kStopBoundaryCount; ++i) {
        const QString key = QString::fromLatin1("Limit%1").arg(i + 1);
        boundaries[i] = settings.value(key, boundaries[i]).toDouble();
    }
    settings.endGroup();
    lise_prompt_rad::setStopLocationBoundaries(boundaries);
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::savePromptRadiationSettingsToFile(const QString& filePath) const
{
    QSettings settings(filePath, QSettings::IniFormat);

    settings.remove("GlobalSettings");
    settings.beginGroup("GlobalSettings");
    const auto& global = lise_prompt_rad::globalSettings();
    settings.setValue("ScaleModelFactor", global.scaleModelFactor);
    settings.setValue("NewShieldingFactor", global.newShieldingFactor);
    settings.setValue("InsideOutsideFactor", global.insideOutsideFactor);
    settings.setValue("Fence", global.fenceFactor);
    settings.setValue("Occupancy", global.occupancyFactor);
    settings.setValue("LightZYieldFactor", global.lightZYieldFactor);
    settings.setValue("BlockNameToCalculate", QString::fromStdString(global.blockNameToCalculate));
    settings.setValue("RateCutoff", global.rateCutoffPps);
    settings.endGroup();

    // Rewrite the group so old/legacy keys do not remain in the INI file.
    settings.remove("LocationFactors");
    settings.beginGroup("LocationFactors");
    const auto& factors = lise_prompt_rad::locationFactors();
    for (int r = 0; r < lise_prompt_rad::kMonitorCount; ++r) {
        for (int c = 0; c < lise_prompt_rad::kLocationCount; ++c) {
            settings.setValue(prompt_rad_ui::locationFactorKey(r, c), factors[r][c]);
        }
    }
    settings.endGroup();

    settings.beginGroup("StopLocationBoundaries");
    const auto& boundaries = lise_prompt_rad::stopLocationBoundaries();
    for (int i = 0; i < lise_prompt_rad::kStopBoundaryCount; ++i) {
        const QString key = QString::fromLatin1("Limit%1").arg(i + 1);
        settings.setValue(key, boundaries[i]);
    }
    settings.setValue("Rule", "x<Limit1:P2, x<Limit2:P3, x<Limit3:P4, x<Limit4:P5, x<Limit5:P4, x<Limit6:P3, else:P2");
    settings.endGroup();

    settings.sync();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::printGlobalSettingsSummary()
{
    const auto& g = lise_prompt_rad::globalSettings();

    m_log->append(tr("\nActive global prompt-radiation settings:"));
    m_log->append(QString::fromLatin1("  Scale Model Factor   = %1").arg(g.scaleModelFactor, 0, 'g', 12));
    m_log->append(QString::fromLatin1("  New Shielding Factor = %1").arg(g.newShieldingFactor, 0, 'g', 12));
    m_log->append(QString::fromLatin1("  Inside/Outside factor= %1").arg(g.insideOutsideFactor, 0, 'g', 12));
    m_log->append(QString::fromLatin1("  Fence                = %1").arg(g.fenceFactor, 0, 'g', 12));
    m_log->append(QString::fromLatin1("  Occupancy            = %1").arg(g.occupancyFactor, 0, 'g', 12));
    m_log->append(QString::fromLatin1("  Light Z yield factor = %1").arg(g.lightZYieldFactor, 0, 'g', 12));
    m_log->append(QString::fromLatin1("  Block name           = %1").arg(QString::fromStdString(g.blockNameToCalculate)));
    m_log->append(QString::fromLatin1("  Rate cutoff          = %1 pps").arg(g.rateCutoffPps, 0, 'e', 2));
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::printLocationFactorSummary()
{
    const auto& f = lise_prompt_rad::locationFactors();

    m_log->append(tr("\nActive location factors for N0318:"));
    for (int c = 0; c < lise_prompt_rad::kLocationCount; ++c) {
        m_log->append(QString::fromLatin1("  %1 = %2")
                      .arg(QString::fromLatin1(lise_prompt_rad::kLocationNames[c]), -5)
                      .arg(f[0][c], 0, 'g', 12));
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void MainWindow::printStopBoundarySummary()
{
    const auto& b = lise_prompt_rad::stopLocationBoundaries();

    m_log->append(tr("\nActive DB0.x stopping boundaries [mm]:"));
    m_log->append(QString::fromLatin1("  x < %1  -> P2").arg(prompt_rad_ui::signedNumber(b[0])));
    m_log->append(QString::fromLatin1("  x < %1  -> P3").arg(prompt_rad_ui::signedNumber(b[1])));
    m_log->append(QString::fromLatin1("  x < %1  -> P4").arg(prompt_rad_ui::signedNumber(b[2])));
    m_log->append(QString::fromLatin1("  x < %1  -> P5").arg(prompt_rad_ui::signedNumber(b[3])));
    m_log->append(QString::fromLatin1("  x < %1  -> P4").arg(prompt_rad_ui::signedNumber(b[4])));
    m_log->append(QString::fromLatin1("  x < %1  -> P3").arg(prompt_rad_ui::signedNumber(b[5])));
    m_log->append(QStringLiteral("  else      -> P2"));
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
