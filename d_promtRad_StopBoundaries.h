#pragma once

#include <QDialog>

#include "o_promptRad_Detailed.h"

class QDialogButtonBox;
class QTableWidget;

class T_PradiationStopBoundariesDlg : public QDialog
{
    Q_OBJECT

public:
    explicit T_PradiationStopBoundariesDlg(QWidget* parent = nullptr);

    void setBoundaries(const lise_prompt_rad::StopBoundaryTable& boundaries);
    lise_prompt_rad::StopBoundaryTable boundaries() const;

public slots:
    void accept() override;

private slots:
    void resetToDefaults();

private:
    void fillTable();
    bool readBoundaries(lise_prompt_rad::StopBoundaryTable& out, QString* errorMessage) const;

    lise_prompt_rad::StopBoundaryTable m_boundaries{};
    QTableWidget* m_table = nullptr;
    QDialogButtonBox* m_buttons = nullptr;
};
