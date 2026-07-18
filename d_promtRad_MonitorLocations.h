#pragma once

#include <QDialog>

class QDialogButtonBox;
class QTableWidget;

class T_PradiationMonitorLocationsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit T_PradiationMonitorLocationsDlg(QWidget* parent = nullptr);

private:
    void fillTable();

    QTableWidget* m_table = nullptr;
    QDialogButtonBox* m_buttons = nullptr;
};
