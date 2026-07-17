#include "d_promtRad_LocationFactors.h"

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPushButton>
#include <QSize>
#include <QStringList>
#include <QTableView>
#include <QVBoxLayout>

T_PradiationLocationFactorsDlg::T_PradiationLocationFactorsDlg(QWidget* parent)
    : QDialog(parent),
      m_model(new LocationFactorsModel(this)),
      m_table(new QTableView(this)),
      m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    setWindowTitle(tr("Prompt Radiation Location Factors"));
    setWindowIcon(QIcon(QStringLiteral(":/Icons/lisepp_small.png")));
    resize(820, 430);

    m_table->setModel(m_model);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QPushButton* resetButton = m_buttons->addButton(tr("Reset defaults"), QDialogButtonBox::ResetRole);
    QPushButton* copyButton = m_buttons->addButton(tr("Copy"), QDialogButtonBox::ActionRole);
    QPushButton* pasteButton = m_buttons->addButton(tr("Paste"), QDialogButtonBox::ActionRole);

    const QSize iconSize(18, 18);
    if (QPushButton* okButton = m_buttons->button(QDialogButtonBox::Ok)) {
        okButton->setIcon(QIcon(QStringLiteral(":/Icons/sign_ok.png")));
        okButton->setIconSize(iconSize);
    }
    if (QPushButton* cancelButton = m_buttons->button(QDialogButtonBox::Cancel)) {
        cancelButton->setIcon(QIcon(QStringLiteral(":/Icons/cancel.png")));
        cancelButton->setIconSize(iconSize);
    }
    copyButton->setIcon(QIcon(QStringLiteral(":/Icons/copy.png")));
    copyButton->setIconSize(iconSize);
    pasteButton->setIcon(QIcon(QStringLiteral(":/Icons/paste1.png")));
    pasteButton->setIconSize(iconSize);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_table);
    layout->addWidget(m_buttons);

    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(resetButton, &QPushButton::clicked, this, &T_PradiationLocationFactorsDlg::resetToDefaults);
    connect(copyButton, &QPushButton::clicked, this, &T_PradiationLocationFactorsDlg::copySelectionToClipboard);
    connect(pasteButton, &QPushButton::clicked, this, &T_PradiationLocationFactorsDlg::pasteFromClipboard);
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void T_PradiationLocationFactorsDlg::setFactors(const lise_prompt_rad::LocationFactorTable& factors)
{
    m_model->setFactors(factors);
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

lise_prompt_rad::LocationFactorTable T_PradiationLocationFactorsDlg::factors() const
{
    return m_model->factors();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void T_PradiationLocationFactorsDlg::resetToDefaults()
{
    const int answer = QMessageBox::question(
        this,
        tr("Reset location factors"),
        tr("Replace the table with the FY2027 template default values?"));

    if (answer == QMessageBox::Yes) {
        m_model->setFactors(lise_prompt_rad::defaultLocationFactors());
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void T_PradiationLocationFactorsDlg::copySelectionToClipboard() const
{
    const QModelIndexList selected = m_table->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        return;
    }

    int minRow = selected.first().row();
    int maxRow = minRow;
    int minCol = selected.first().column();
    int maxCol = minCol;

    for (const QModelIndex& idx : selected) {
        minRow = qMin(minRow, idx.row());
        maxRow = qMax(maxRow, idx.row());
        minCol = qMin(minCol, idx.column());
        maxCol = qMax(maxCol, idx.column());
    }

    QString text;
    for (int r = minRow; r <= maxRow; ++r) {
        QStringList row;
        for (int c = minCol; c <= maxCol; ++c) {
            row << m_model->data(m_model->index(r, c), Qt::DisplayRole).toString();
        }
        text += row.join('\t');
        if (r != maxRow) {
            text += '\n';
        }
    }

    QApplication::clipboard()->setText(text);
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void T_PradiationLocationFactorsDlg::pasteFromClipboard()
{
    const QString text = QApplication::clipboard()->text();
    if (text.trimmed().isEmpty()) {
        return;
    }

    QModelIndex start = m_table->currentIndex();
    if (!start.isValid()) {
        start = m_model->index(0, 0);
    }

    QString normalized = text;
    normalized.replace("\r\n", "\n");
    normalized.replace('\r', '\n');
    const QStringList lines = normalized.split('\n', Qt::SkipEmptyParts);
    for (int r = 0; r < lines.size(); ++r) {
        const QStringList cells = lines.at(r).split('\t');
        for (int c = 0; c < cells.size(); ++c) {
            const QModelIndex idx = m_model->index(start.row() + r, start.column() + c);
            if (idx.isValid()) {
                m_model->setData(idx, cells.at(c), Qt::EditRole);
            }
        }
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
