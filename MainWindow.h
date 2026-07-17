#pragma once

#include <QMainWindow>
#include <QString>

class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void loadSettingsFromIniFile();
    void saveSettingsAsIniFile();
    void useDefaultIniFile();
    void editGlobalSettings();
    void editLocationFactors();
    void editStopLocationBoundaries();
    void runSmallExample();
    void resetGlobalSettings();
    void resetLocationFactors();
    void resetStopLocationBoundaries();

private:
    void createMenus();
    QString defaultSettingsFilePath() const;
    QString settingsFilePath() const;
    void loadPromptRadiationSettings();
    void savePromptRadiationSettings() const;
    void loadPromptRadiationSettingsFromFile(const QString& filePath);
    void savePromptRadiationSettingsToFile(const QString& filePath) const;
    void printGlobalSettingsSummary();
    void printLocationFactorSummary();
    void printStopBoundarySummary();

    QTextEdit* m_log = nullptr;
    QString m_settingsFilePath;
};
