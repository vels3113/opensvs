#include "models/DiffEntryModel.hpp"

DiffEntryModel::DiffEntryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int DiffEntryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return diffs_.size();
}

int DiffEntryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return NUM_COLUMNS;
}

QVariant DiffEntryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return {};
    }
    if (index.row() < 0 || index.row() >= diffs_.size()) {
        return {};
    }

    const auto &entry = diffs_.at(index.row());
    switch (index.column()) {
    case 0:
        return NetgenJsonParser::toTypeString(entry.type);
    case 1:
        return entry.name;
    case 2:
        return entry.layoutCell;
    case 3:
        return entry.schematicCell;
    case 4:
        return entry.details;
    default:
        return {};
    }
}

QVariant DiffEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
    case 0: return QStringLiteral("Type");
    case 1: return QStringLiteral("Object");
    case 2: return QStringLiteral("Layout Cell");
    case 3: return QStringLiteral("Schematic Cell");
    case 4: return QStringLiteral("Details");
    default: return {};
    }
}

void DiffEntryModel::setDiffs(const QVector<NetgenJsonParser::DiffEntry> &diffs)
{
    beginResetModel();
    diffs_ = diffs;
    endResetModel();
}
