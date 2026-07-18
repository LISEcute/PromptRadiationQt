#include "o_promptRad_LocationFactorsModel.h"

#include <QLocale>

LocationFactorsModel::LocationFactorsModel(QObject* parent)
    : QAbstractTableModel(parent),
      m_factors(lise_prompt_rad::locationFactors())
{
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

int LocationFactorsModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : lise_prompt_rad::kMonitorCount;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

int LocationFactorsModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : lise_prompt_rad::kLocationCount;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

QVariant LocationFactorsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() < 0 || index.row() >= lise_prompt_rad::kMonitorCount ||
        index.column() < 0 || index.column() >= lise_prompt_rad::kLocationCount) {
        return QVariant();
    }

    const double value = m_factors[index.row()][index.column()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return QLocale::c().toString(value, 'g', 6);
    }

    if (role == Qt::TextAlignmentRole) {
        const Qt::Alignment alignment = Qt::AlignRight | Qt::AlignVCenter;
        return QVariant::fromValue(alignment);
    }

    return QVariant();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

QVariant LocationFactorsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        if (section >= 0 && section < lise_prompt_rad::kLocationCount) {
            return QString::fromLatin1(lise_prompt_rad::kLocationNames[section]);
        }
    } else {
        if (section >= 0 && section < lise_prompt_rad::kMonitorCount) {
            return QString::fromLatin1(lise_prompt_rad::kMonitorNames[section]);
        }
    }

    return QVariant();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

Qt::ItemFlags LocationFactorsModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.isValid()) {
        f |= Qt::ItemIsEditable;
    }
    return f;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

bool LocationFactorsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole || !index.isValid()) {
        return false;
    }
    if (index.row() < 0 || index.row() >= lise_prompt_rad::kMonitorCount ||
        index.column() < 0 || index.column() >= lise_prompt_rad::kLocationCount) {
        return false;
    }

    bool ok = false;
    const double v = QLocale::c().toDouble(value.toString().trimmed(), &ok);
    if (!ok) {
        return false;
    }

    m_factors[index.row()][index.column()] = v;
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

lise_prompt_rad::LocationFactorTable LocationFactorsModel::factors() const
{
    return m_factors;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void LocationFactorsModel::setFactors(const lise_prompt_rad::LocationFactorTable& factors)
{
    beginResetModel();
    m_factors = factors;
    endResetModel();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
