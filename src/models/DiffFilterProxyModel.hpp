#pragma once

#include <QRegularExpression>
#include <QSet>
#include <QSortFilterProxyModel>

class DiffFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
  public:
    explicit DiffFilterProxyModel(QObject *parent = nullptr);

    void
    setTypeFilter(const QString &type); // empty or "All" means no type filter
    void setSearchTerm(
        const QString &term); // case-insensitive substring on object/details
    void setAllowedCircuits(
        const QSet<int> &circuits); // empty set means no circuit filter

  protected:
    bool filterAcceptsRow(int source_row,
                          const QModelIndex &source_parent) const override;

  private:
    QString typeFilter_;
    QString searchTerm_;
    QRegularExpression searchRegex_;
    bool searchRegexValid_{false};
    QSet<int> circuitFilter_;
};
