#include "models/CircuitTreeModel.hpp"

#include <QString>

CircuitTreeModel::CircuitTreeModel(QObject *parent)
    : QAbstractItemModel(parent) {}

void CircuitTreeModel::clear() {
    roots_.clear();
    storage_.clear();
}

CircuitTreeModel::Node *
CircuitTreeModel::buildNode(NetgenJsonParser::Report::Circuit *circuit,
                            Node *parent) {
    auto holder = std::make_unique<Node>();
    holder->circuit = circuit;
    holder->parent = parent;
    Node *raw = holder.get();
    storage_.push_back(std::move(holder));
    for (auto *childCircuit : circuit->subcircuits) {
        raw->children.append(buildNode(childCircuit, raw));
    }
    return raw;
}

void CircuitTreeModel::setCircuits(
    QVector<NetgenJsonParser::Report::Circuit> *circuits) {
    beginResetModel();
    clear();
    circuits_ = circuits;
    if (circuits_) {
        for (auto &circuit : *circuits_) {
            if (circuit.isTopLevel) {
                roots_.append(buildNode(&circuit, nullptr));
            }
        }
    }
    endResetModel();
}

QModelIndex CircuitTreeModel::index(int row, int column,
                                    const QModelIndex &parentIdx) const {
    if (column != 0)
        return {};
    Node *parentNode = nodeFromIndex(parentIdx);
    const auto &vec = parentNode ? parentNode->children : roots_;
    if (row < 0 || row >= vec.size())
        return {};
    return createIndex(row, column, vec.at(row));
}

QModelIndex CircuitTreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid())
        return {};
    Node *node = static_cast<Node *>(child.internalPointer());
    if (!node || !node->parent)
        return {};
    Node *parentNode = node->parent;
    Node *grand = parentNode->parent;
    const auto &siblings = grand ? grand->children : roots_;
    const int row = siblings.indexOf(parentNode);
    if (row < 0)
        return {};
    return createIndex(row, 0, parentNode);
}

int CircuitTreeModel::rowCount(const QModelIndex &parentIdx) const {
    if (parentIdx.column() > 0)
        return 0;
    Node *parentNode = nodeFromIndex(parentIdx);
    return parentNode ? parentNode->children.size() : roots_.size();
}

int CircuitTreeModel::columnCount(const QModelIndex &) const { return 1; }

QVariant CircuitTreeModel::data(const QModelIndex &idx, int role) const {
    if (!idx.isValid() || role != Qt::DisplayRole)
        return {};
    Node *node = static_cast<Node *>(idx.internalPointer());
    if (!node || !node->circuit)
        return {};
    const auto *c = node->circuit;
    return QStringLiteral("%1 vs %2").arg(c->layoutCell, c->schematicCell);
}

NetgenJsonParser::Report::Circuit *
CircuitTreeModel::circuitForIndex(const QModelIndex &idx) const {
    Node *node = nodeFromIndex(idx);
    return node ? node->circuit : nullptr;
}

CircuitTreeModel::Node *
CircuitTreeModel::nodeFromIndex(const QModelIndex &idx) const {
    if (!idx.isValid())
        return nullptr;
    return static_cast<Node *>(idx.internalPointer());
}
