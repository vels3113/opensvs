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
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    NetgenJsonParser::Report::Circuit *
    circuitForIndex(const QModelIndex &idx) const;

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
    Node *buildNode(NetgenJsonParser::Report::Circuit *circuit, Node *parent);
    Node *nodeFromIndex(const QModelIndex &idx) const;
};
