#include "d_promtRad_MonitorLocations.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QIcon>
#include <QPushButton>
#include <QSize>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

struct MonitorLocationRow {
    const char* monitor;
    const char* location;
    const char* description;
};
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

const MonitorLocationRow kMonitorLocationRows[] = {
    {"N0318",  "VPS roof / inside",                         "Main prompt-radiation reference monitor; also shown in the local DB1/VPS schematic."},
    {"N0306",  "Ground-level / upper-side VPS area",        "Expanded PHITS/VPS location figure; upper-side/corridor-side monitor location."},
    {"N0319",  "Ground-level / upper-right VPS area",       "Expanded detector-location figure and Target Hall/VPS monitor map."},
    {"N0325A", "VPS roof / inside",                         "Upstream/left-side inside roof monitor near the N0318 region."},
    {"N0325B", "VPS roof / inside",                         "Downstream/right-side inside roof monitor near the N0318/DB0/SCD3 region."},
    {"N0304B", "DB1 / VPS nearby monitor",                  "One of the two monitors shown around DB1, together with N0318."},
    {"N0314",  "Ground-level Target Hall / VPS area",        "Shown together with N0315 in the expanded monitor-location figure."},
    {"N0305",  "Ground-level Target Hall / ARIS-VPS area",   "Visible on the radiation-survey and ARIS/Transfer Hall monitor maps."},
    {"N0308B", "Ground-level ARIS / VPS upstream area",      "Upstream/left-side VPS monitor in the expanded monitor-location figure."},
    {"N0322",  "Target Hall / VPS monitor cluster",          "Visible in the general Target Hall/VPS monitor-location map, upper part of the VPS cluster."},
    {"N0323",  "Target Hall / VPS monitor cluster",          "Visible near N0322, N0325A, and N0318 in the Target Hall/VPS monitor map."},
    {"N0324",  "VPS roof near DB0 / SCD3",                   "One of the four VPS roof monitors; located near the DB0/SCD3 region."},
    {"N0304",  "Ground-level ARIS / Transfer Hall area",     "Lower-left ARIS/Transfer Hall monitor map region."},
    {"N0315",  "Ground-level Target Hall / VPS area",        "Shown together/near N0314 in the expanded monitor-location figure; separate utility row."}
};
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

constexpr int kMonitorLocationRowCount = static_cast<int>(sizeof(kMonitorLocationRows) / sizeof(kMonitorLocationRows[0]));

} // namespace

//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
T_PradiationMonitorLocationsDlg::T_PradiationMonitorLocationsDlg(QWidget* parent)
    : QDialog(parent),
      m_table(new QTableWidget(kMonitorLocationRowCount, 3, this)),
      m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok, this))
{
    setWindowTitle(tr("Prompt Radiation Monitor Location Table"));
    setWindowIcon(QIcon(QStringLiteral(":/Icons/lisepp_small.png")));
    resize(980, 470);

    m_table->setHorizontalHeaderLabels({tr("Monitor"), tr("Location"), tr("Interpretation / description")});

    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_table->verticalHeader()->setMinimumSectionSize(22);
    m_table->verticalHeader()->setDefaultSectionSize(26);

    m_table->setWordWrap(false);
    m_table->setTextElideMode(Qt::ElideRight);
    m_table->setAlternatingRowColors(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    if (QPushButton* okButton = m_buttons->button(QDialogButtonBox::Ok)) {
            okButton->setIcon(QIcon(QStringLiteral(":/Icons/sign_ok.png")));
            okButton->setIconSize(QSize(18, 18));
        }

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_table);
    layout->addWidget(m_buttons);

    fillTable();

    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void T_PradiationMonitorLocationsDlg::fillTable()
{
    for (int r = 0; r < kMonitorLocationRowCount; ++r)
        {
            const MonitorLocationRow& row = kMonitorLocationRows[r];

            auto* monitorItem = new QTableWidgetItem(QString::fromLatin1(row.monitor));
            monitorItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(r, 0, monitorItem);

            auto* locationItem = new QTableWidgetItem(QString::fromLatin1(row.location));
            locationItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(r, 1, locationItem);

            auto* descriptionItem = new QTableWidgetItem(QString::fromLatin1(row.description));
            descriptionItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            m_table->setItem(r, 2, descriptionItem);
        }

    for (int r = 0; r < kMonitorLocationRowCount; ++r) {
            m_table->setRowHeight(r, 26);
        }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
