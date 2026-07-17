#pragma once

#include "PromptRadiationDetailed.h"

#include <QDialog>

class QDialogButtonBox;
class QTableWidget;

class GlobalSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalSettingsDialog(QWidget* parent = nullptr);

    void setSettings(const lise_prompt_rad::GlobalPromptRadiationSettings& settings);
    lise_prompt_rad::GlobalPromptRadiationSettings settings() const;

public slots:
    void accept() override;

private slots:
    void resetToDefaults();

private:
    void fillTable();
    bool readSettings(lise_prompt_rad::GlobalPromptRadiationSettings& out,
                      QString* errorMessage) const;

    lise_prompt_rad::GlobalPromptRadiationSettings m_settings;
    QTableWidget* m_table = nullptr;
    QDialogButtonBox* m_buttons = nullptr;
};
