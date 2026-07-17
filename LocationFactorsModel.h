#pragma once

#include <QAbstractTableModel>

#include "PromptRadiationDetailed.h"

class LocationFactorsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit LocationFactorsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    lise_prompt_rad::LocationFactorTable factors() const;
    void setFactors(const lise_prompt_rad::LocationFactorTable& factors);

private:
    lise_prompt_rad::LocationFactorTable m_factors{};
};
