#include "d_promtRad_StopBoundaries.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSize>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QLocale>

namespace {

const char* resultLocationNameForBoundaryRow(int row)
{
    switch (row) {
    case 0: return "P2";
    case 1: return "P3";
    case 2: return "P4";
    case 3: return "P5";
    case 4: return "P4";
    case 5: return "P3";
    default: return "P2";
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

} // namespace

T_PradiationStopBoundariesDlg::T_PradiationStopBoundariesDlg(QWidget* parent)
    : QDialog(parent),
      m_boundaries(lise_prompt_rad::stopLocationBoundaries()),
      m_table(new QTableWidget(lise_prompt_rad::kStopBoundaryCount, 3, this)),
      m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    setWindowTitle(tr("Prompt Radiation DB0.x Stopping Boundaries"));
    setWindowIcon(QIcon(QStringLiteral(":/Icons/lisepp_small.png")));
    resize(680, 360);

    auto* note = new QLabel(
        tr("These limits are applied in this exact order to assign the stopping location from DB0.x. "
           "Values must be strictly increasing. If DB0.x is missing, location remains P2."),
        this);
    note->setWordWrap(true);

    m_table->setHorizontalHeaderLabels({tr("Rule"), tr("Upper limit, DB0.x [mm]"), tr("Returned location")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
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

    connect(m_buttons, &QDialogButtonBox::accepted, this, &T_PradiationStopBoundariesDlg::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(resetButton, &QPushButton::clicked, this, &T_PradiationStopBoundariesDlg::resetToDefaults);
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void T_PradiationStopBoundariesDlg::setBoundaries(const lise_prompt_rad::StopBoundaryTable& boundaries)
{
    m_boundaries = lise_prompt_rad::areStopLocationBoundariesValid(boundaries)
                   ? boundaries
                   : lise_prompt_rad::defaultStopLocationBoundaries();
    fillTable();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

lise_prompt_rad::StopBoundaryTable T_PradiationStopBoundariesDlg::boundaries() const
{
    return m_boundaries;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void T_PradiationStopBoundariesDlg::fillTable()
{
    for (int r = 0; r < lise_prompt_rad::kStopBoundaryCount; ++r) {
        auto* ruleItem = new QTableWidgetItem(QString::fromLatin1(lise_prompt_rad::kStopBoundaryNames[r]));
        ruleItem->setFlags(ruleItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(r, 0, ruleItem);

        auto* limitItem = new QTableWidgetItem(QLocale::c().toString(m_boundaries[r], 'g', 12));
        limitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(r, 1, limitItem);

        auto* locationItem = new QTableWidgetItem(QString::fromLatin1(resultLocationNameForBoundaryRow(r)));
        locationItem->setFlags(locationItem->flags() & ~Qt::ItemIsEditable);
        locationItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(r, 2, locationItem);
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

bool T_PradiationStopBoundariesDlg::readBoundaries(lise_prompt_rad::StopBoundaryTable& out,
                                          QString* errorMessage) const
{
    for (int r = 0; r < lise_prompt_rad::kStopBoundaryCount; ++r) {
        const QTableWidgetItem* item = m_table->item(r, 1);
        bool ok = false;
        const double value = QLocale::c().toDouble(item ? item->text().trimmed() : QString(), &ok);
        if (!ok) {
            if (errorMessage) {
                *errorMessage = tr("Boundary row %1 is not a valid number.").arg(r + 1);
            }
            return false;
        }
        out[r] = value;
    }

    if (!lise_prompt_rad::areStopLocationBoundariesValid(out)) {
        if (errorMessage) {
            *errorMessage = tr("Boundary values must be strictly increasing, for example: -310, -170, -100, 100, 170, 310.");
        }
        return false;
    }

    return true;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void T_PradiationStopBoundariesDlg::accept()
{
    lise_prompt_rad::StopBoundaryTable table{};
    QString errorMessage;
    if (!readBoundaries(table, &errorMessage)) {
        QMessageBox::warning(this, tr("Invalid boundaries"), errorMessage);
        return;
    }

    m_boundaries = table;
    QDialog::accept();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void T_PradiationStopBoundariesDlg::resetToDefaults()
{
    const int answer = QMessageBox::question(
        this,
        tr("Reset stopping boundaries"),
        tr("Replace DB0.x stopping boundaries with Excel/FY2027 default values?"));

    if (answer == QMessageBox::Yes) {
        m_boundaries = lise_prompt_rad::defaultStopLocationBoundaries();
        fillTable();
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
