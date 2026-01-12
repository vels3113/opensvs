#pragma once

#include <QAbstractTableModel>
#include <QVector>

#include "parsers/NetgenJsonParser.hpp"

class DiffEntryModel : public QAbstractTableModel {
    Q_OBJECT
  public:
    explicit DiffEntryModel(QObject *parent = nullptr);

    auto
    rowCount(const QModelIndex &parent = QModelIndex()) const -> int override;
    auto columnCount(const QModelIndex &parent = QModelIndex()) const
        -> int override;
    auto data(const QModelIndex &index,
              int role = Qt::DisplayRole) const -> QVariant override;
    auto headerData(int section, Qt::Orientation orientation,
                    int role = Qt::DisplayRole) const -> QVariant override;

    void setDiffs(const QVector<NetgenJsonParser::DiffEntry> &diffs);

  private:
    QVector<NetgenJsonParser::DiffEntry> diffs_;
};
