#pragma once

#include <QDialog>

#include "LocationFactorsModel.h"

class QDialogButtonBox;
class QTableView;

class LocationFactorsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LocationFactorsDialog(QWidget* parent = nullptr);

    void setFactors(const lise_prompt_rad::LocationFactorTable& factors);
    lise_prompt_rad::LocationFactorTable factors() const;

private slots:
    void resetToDefaults();
    void copySelectionToClipboard() const;
    void pasteFromClipboard();

private:
    LocationFactorsModel* m_model = nullptr;
    QTableView* m_table = nullptr;
    QDialogButtonBox* m_buttons = nullptr;
};
