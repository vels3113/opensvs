#include "models/DiffFilterProxyModel.hpp"
#include "models/DiffEntryCommon.hpp"

#include <QModelIndex>
#include <QString>

DiffFilterProxyModel::DiffFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void DiffFilterProxyModel::setTypeFilter(const QString &type) {
    typeFilter_ = type.trimmed();
    invalidateFilter();
}

void DiffFilterProxyModel::setSearchTerm(const QString &term) {
    searchTerm_ = term.trimmed();
    if (searchTerm_.isEmpty()) {
        searchRegexValid_ = false;
    } else {
        QRegularExpression::PatternOptions opts =
            QRegularExpression::CaseInsensitiveOption |
            QRegularExpression::UseUnicodePropertiesOption;
        searchRegex_ = QRegularExpression(searchTerm_, opts);
        searchRegexValid_ = searchRegex_.isValid();
    }
    invalidateFilter();
}

void DiffFilterProxyModel::setAllowedCircuits(const QSet<int> &circuits) {
    circuitFilter_ = circuits;
    invalidateFilter();
}

bool DiffFilterProxyModel::filterAcceptsRow(
    int source_row, const QModelIndex &source_parent) const {
    if (!sourceModel())
        return true;
    const int cols = sourceModel()->columnCount(source_parent);
    if (cols <= DiffEntryColumns::DETAILS)
        return true;
    if (source_row < 0 ||
        source_row >= sourceModel()->rowCount(source_parent)) {
        return false;
    }
    const QModelIndex typeIdx =
        sourceModel()->index(source_row, DiffEntryColumns::TYPE, source_parent);
    const QModelIndex objIdx = sourceModel()->index(
        source_row, DiffEntryColumns::OBJECT, source_parent);
    const QModelIndex detailsIdx = sourceModel()->index(
        source_row, DiffEntryColumns::DETAILS, source_parent);
    const QModelIndex circuitIdx =
        sourceModel()->index(source_row, DiffEntryColumns::TYPE, source_parent);

    if (!typeIdx.isValid() || !objIdx.isValid() || !detailsIdx.isValid()) {
        return true;
    }

    const QString type =
        sourceModel()->data(typeIdx, Qt::DisplayRole).toString();
    const QString object =
        sourceModel()->data(objIdx, Qt::DisplayRole).toString();
    const QString details =
        sourceModel()->data(detailsIdx, Qt::DisplayRole).toString();
    const int circuitId =
        circuitIdx.isValid()
            ? sourceModel()->data(circuitIdx, Qt::UserRole).toInt()
            : -1;

    if (!typeFilter_.isEmpty() &&
        typeFilter_.compare(QStringLiteral("All"), Qt::CaseInsensitive) != 0) {
        if (type.compare(typeFilter_, Qt::CaseInsensitive)) {
            return false;
        }
    }

    if (!circuitFilter_.isEmpty() && !circuitFilter_.contains(circuitId)) {
        return false;
    }

    if (!searchTerm_.isEmpty()) {
        if (searchRegexValid_) {
            if (!searchRegex_.match(object).hasMatch() &&
                !searchRegex_.match(details).hasMatch()) {
                return false;
            }
        } else {
            const QString needle = searchTerm_.toLower();
            if (!object.toLower().contains(needle) &&
                !details.toLower().contains(needle)) {
                return false;
            }
        }
    }

    return true;
}
