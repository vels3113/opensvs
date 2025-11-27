#pragma once

#include <QSortFilterProxyModel>
#include <QRegularExpression>

class DiffFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit DiffFilterProxyModel(QObject *parent = nullptr);

    void setTypeFilter(const QString &type); // empty or "All" means no type filter
    void setSearchTerm(const QString &term); // case-insensitive substring on object/details

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString typeFilter_;
    QString searchTerm_;
    QRegularExpression searchRegex_;
    bool searchRegexValid_{false};
};
