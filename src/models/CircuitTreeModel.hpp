#pragma once

#include <QAbstractItemModel>
#include <QVector>
#include <memory>
#include <vector>

#include "parsers/NetgenJsonParser.hpp"

class CircuitTreeModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    explicit CircuitTreeModel(QObject *parent = nullptr);

    void setCircuits(QVector<NetgenJsonParser::Report::Circuit> *circuits);
    auto index(int row, int column, const QModelIndex &parent = QModelIndex())
        const -> QModelIndex override;
    auto parent(const QModelIndex &child) const -> QModelIndex override;
    auto
    rowCount(const QModelIndex &parent = QModelIndex()) const -> int override;
    auto columnCount(const QModelIndex &parent = QModelIndex()) const
        -> int override;
    auto data(const QModelIndex &idx,
              int role = Qt::DisplayRole) const -> QVariant override;

    static auto circuitForIndex(const QModelIndex &idx)
        -> NetgenJsonParser::Report::Circuit *;

  private:
    struct Node {
        NetgenJsonParser::Report::Circuit *circuit = nullptr;
        Node *parent = nullptr;
        QVector<Node *> children;
    };

    QVector<NetgenJsonParser::Report::Circuit> *circuits_{nullptr};
    QVector<Node *> roots_;
    mutable std::vector<std::unique_ptr<Node>> storage_;

    void clear();
    auto buildNode(NetgenJsonParser::Report::Circuit *circuit,
                   Node *parent) -> Node *;
    static auto nodeFromIndex(const QModelIndex &idx) -> Node *;
};
