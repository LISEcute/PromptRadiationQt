#include "GlobalSettingsDialog.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QSize>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <cmath>

namespace {

enum GlobalSettingRow {
    RowScaleModelFactor = 0,
    RowNewShieldingFactor,
    RowInsideOutsideFactor,
    RowFence,
    RowOccupancy,
    RowLightZYieldFactor,
    RowBlockNameToCalculate,
    RowRateCutoff,
    RowCount
};
//--------------------------------------------------------------------------------

const char* settingLabel(int row)
{
    switch (row) {
    case RowScaleModelFactor:     return "Scale Model Factor";
    case RowNewShieldingFactor:   return "New Shielding Factor";
    case RowInsideOutsideFactor:  return "Inside/Outside factor";
    case RowFence:                return "Fence";
    case RowOccupancy:            return "Occupancy";
    case RowLightZYieldFactor:    return "Light Z yield factor";
    case RowBlockNameToCalculate: return "Block name to calculate";
    case RowRateCutoff:           return "Rate cutoff [pps]";
    default:                      return "";
    }
}
//--------------------------------------------------------------------------------

const char* settingDescription(int row)
{
    switch (row) {
    case RowScaleModelFactor:
        return "Scale-model coefficient from prompt-radiation template.";
    case RowNewShieldingFactor:
        return "Additional shielding correction factor.";
    case RowInsideOutsideFactor:
        return "Conversion factor from inside monitor estimate to outside wall estimate.";
    case RowFence:
        return "Distance/geometry factor from wall to fence.";
    case RowOccupancy:
        return "Occupancy divisor.";
    case RowLightZYieldFactor:
        return "Multiplier for H/He or light-Z yields.";
    case RowBlockNameToCalculate:
        return "LISE++ block name used later for DB0.x position lookup.";
    case RowRateCutoff:
        return "Minimum rate included in later automatic LISE++ fragment collection.";
    default:
        return "";
    }
}
//--------------------------------------------------------------------------------

QString numberText(double value)
{
    return QLocale::c().toString(value, 'g', 12);
}
//--------------------------------------------------------------------------------

bool readDoubleCell(const QTableWidget* table,
                    int row,
                    double& value,
                    QString* errorMessage)
{
    const QTableWidgetItem* item = table->item(row, 1);
    bool ok = false;
    value = QLocale::c().toDouble(item ? item->text().trimmed() : QString(), &ok);
    if (!ok || !std::isfinite(value)) {
        if (errorMessage) {
            *errorMessage = QObject::tr("%1 is not a valid number.").arg(QString::fromLatin1(settingLabel(row)));
        }
        return false;
    }
    return true;
}
//--------------------------------------------------------------------------------

} // namespace

GlobalSettingsDialog::GlobalSettingsDialog(QWidget* parent)
    : QDialog(parent),
      m_settings(lise_prompt_rad::globalSettings()),
      m_table(new QTableWidget(RowCount, 3, this)),
      m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    setWindowTitle(tr("Prompt Radiation Global Settings"));
    setWindowIcon(QIcon(QStringLiteral(":/Icons/lisepp_small.png")));
    resize(760, 430);

    auto* note = new QLabel(
        tr("These global prompt-radiation parameters are loaded from and saved to the active INI file. "
           "Some of them are placeholders for later LISE++ integration."),
        this);
    note->setWordWrap(true);

    m_table->setHorizontalHeaderLabels({tr("Variable"), tr("Value"), tr("Comment")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);

    QPushButton* resetButton = m_buttons->addButton(tr("Reset defaults"), QDialogButtonBox::ResetRole);

    const QSize iconSize(18, 18);
    if (QPushButton* okButton = m_buttons->button(QDialogButtonBox::Ok)) {
        okButton->setIcon(QIcon(QStringLiteral(":/Icons/sign_ok.png")));
        okButton->setIconSize(iconSize);
    }
    if (QPushButton* cancelButton = m_buttons->button(QDialogButtonBox::Cancel)) {
        cancelButton->setIcon(QIcon(QStringLiteral(":/Icons/cancel.png")));
        cancelButton->setIconSize(iconSize);
    }

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(note);
    layout->addWidget(m_table);
    layout->addWidget(m_buttons);

    fillTable();

    connect(m_buttons, &QDialogButtonBox::accepted, this, &GlobalSettingsDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(resetButton, &QPushButton::clicked, this, &GlobalSettingsDialog::resetToDefaults);
}
//--------------------------------------------------------------------------------

void GlobalSettingsDialog::setSettings(const lise_prompt_rad::GlobalPromptRadiationSettings& settings)
{
    m_settings = lise_prompt_rad::areGlobalSettingsValid(settings)
                 ? settings
                 : lise_prompt_rad::defaultGlobalSettings();
    fillTable();
}
//--------------------------------------------------------------------------------

lise_prompt_rad::GlobalPromptRadiationSettings GlobalSettingsDialog::settings() const
{
    return m_settings;
}
//--------------------------------------------------------------------------------

void GlobalSettingsDialog::fillTable()
{
    for (int r = 0; r < RowCount; ++r) {
        auto* labelItem = new QTableWidgetItem(QString::fromLatin1(settingLabel(r)));
        labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(r, 0, labelItem);

        QString value;
        switch (r) {
        case RowScaleModelFactor:     value = numberText(m_settings.scaleModelFactor); break;
        case RowNewShieldingFactor:   value = numberText(m_settings.newShieldingFactor); break;
        case RowInsideOutsideFactor:  value = numberText(m_settings.insideOutsideFactor); break;
        case RowFence:                value = numberText(m_settings.fenceFactor); break;
        case RowOccupancy:            value = numberText(m_settings.occupancyFactor); break;
        case RowLightZYieldFactor:    value = numberText(m_settings.lightZYieldFactor); break;
        case RowBlockNameToCalculate: value = QString::fromStdString(m_settings.blockNameToCalculate); break;
        case RowRateCutoff:           value = numberText(m_settings.rateCutoffPps); break;
        default: break;
        }

        auto* valueItem = new QTableWidgetItem(value);
        if (r != RowBlockNameToCalculate) {
            valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
        m_table->setItem(r, 1, valueItem);

        auto* commentItem = new QTableWidgetItem(QString::fromLatin1(settingDescription(r)));
        commentItem->setFlags(commentItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(r, 2, commentItem);
    }
}
//--------------------------------------------------------------------------------

bool GlobalSettingsDialog::readSettings(lise_prompt_rad::GlobalPromptRadiationSettings& out,
                                        QString* errorMessage) const
{
    if (!readDoubleCell(m_table, RowScaleModelFactor, out.scaleModelFactor, errorMessage)) return false;
    if (!readDoubleCell(m_table, RowNewShieldingFactor, out.newShieldingFactor, errorMessage)) return false;
    if (!readDoubleCell(m_table, RowInsideOutsideFactor, out.insideOutsideFactor, errorMessage)) return false;
    if (!readDoubleCell(m_table, RowFence, out.fenceFactor, errorMessage)) return false;
    if (!readDoubleCell(m_table, RowOccupancy, out.occupancyFactor, errorMessage)) return false;
    if (!readDoubleCell(m_table, RowLightZYieldFactor, out.lightZYieldFactor, errorMessage)) return false;
    if (!readDoubleCell(m_table, RowRateCutoff, out.rateCutoffPps, errorMessage)) return false;

    const QTableWidgetItem* blockItem = m_table->item(RowBlockNameToCalculate, 1);
    const QString blockName = blockItem ? blockItem->text().trimmed() : QString();
    if (blockName.isEmpty()) {
        if (errorMessage) {
            *errorMessage = tr("Block name to calculate cannot be empty.");
        }
        return false;
    }
    out.blockNameToCalculate = blockName.toStdString();

    if (!lise_prompt_rad::areGlobalSettingsValid(out)) {
        if (errorMessage) {
            *errorMessage = tr("Global prompt-radiation settings are not valid.");
        }
        return false;
    }

    return true;
}
//--------------------------------------------------------------------------------

void GlobalSettingsDialog::accept()
{
    lise_prompt_rad::GlobalPromptRadiationSettings newSettings;
    QString errorMessage;
    if (!readSettings(newSettings, &errorMessage)) {
        QMessageBox::warning(this, tr("Invalid global settings"), errorMessage);
        return;
    }

    m_settings = newSettings;
    QDialog::accept();
}
//--------------------------------------------------------------------------------

void GlobalSettingsDialog::resetToDefaults()
{
    const int answer = QMessageBox::question(
        this,
        tr("Reset global settings"),
        tr("Replace global prompt-radiation settings with source-code default values?"));

    if (answer == QMessageBox::Yes) {
        m_settings = lise_prompt_rad::defaultGlobalSettings();
        fillTable();
    }
}
//--------------------------------------------------------------------------------
