#include "models/DiffEntryModel.hpp"
#include "models/DiffEntryCommon.hpp"

DiffEntryModel::DiffEntryModel(QObject *parent) : QAbstractTableModel(parent) {}

int DiffEntryModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return diffs_.size();
}

int DiffEntryModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return DiffEntryColumns::NUM_COLUMNS;
}

QVariant DiffEntryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return {};
    }
    if (index.row() < 0 || index.row() >= diffs_.size()) {
        return {};
    }

    const auto &entry = diffs_.at(index.row());
    if (role == Qt::UserRole) {
        return entry.circuitIndex;
    }
    if (role != Qt::DisplayRole) {
        return {};
    }
    switch (index.column()) {
    case DiffEntryColumns::TYPE:
        return NetgenJsonParser::toTypeString(entry.type);
    case DiffEntryColumns::SUBTYPE:
        return NetgenJsonParser::toSubtypeString(entry.subtype);
    case DiffEntryColumns::OBJECT:
        return entry.name;
    case DiffEntryColumns::LAYOUT_CELL:
        return entry.layoutCell;
    case DiffEntryColumns::SCHEMATIC_CELL:
        return entry.schematicCell;
    case DiffEntryColumns::DETAILS:
        return entry.details;
    default:
        return {};
    }
}

QVariant DiffEntryModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
    case DiffEntryColumns::TYPE:
        return QStringLiteral("Type");
    case DiffEntryColumns::SUBTYPE:
        return QStringLiteral("Subtype");
    case DiffEntryColumns::OBJECT:
        return QStringLiteral("Object");
    case DiffEntryColumns::LAYOUT_CELL:
        return QStringLiteral("Layout Cell");
    case DiffEntryColumns::SCHEMATIC_CELL:
        return QStringLiteral("Schematic Cell");
    case DiffEntryColumns::DETAILS:
        return QStringLiteral("Details");
    default:
        return {};
    }
}

void DiffEntryModel::setDiffs(
    const QVector<NetgenJsonParser::DiffEntry> &diffs) {
    beginResetModel();
    diffs_ = diffs;
    endResetModel();
}
