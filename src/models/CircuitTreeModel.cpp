#include "models/CircuitTreeModel.hpp"

#include <QString>

CircuitTreeModel::CircuitTreeModel(QObject *parent)
    : QAbstractItemModel(parent) {}

void CircuitTreeModel::clear() {
    roots_.clear();
    storage_.clear();
}

auto CircuitTreeModel::buildNode(NetgenJsonParser::Report::Circuit *circuit,
                                 Node *parent) -> CircuitTreeModel::Node * {
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
    if (circuits_ != nullptr) {
        for (auto &circuit : *circuits_) {
            if (circuit.isTopLevel) {
                roots_.append(buildNode(&circuit, nullptr));
            }
        }
    }
    endResetModel();
}

auto CircuitTreeModel::index(
    int row, int column, const QModelIndex &parentIdx) const -> QModelIndex {
    if (column != 0) {
        return {};
    }
    Node *parentNode = nodeFromIndex(parentIdx);
    const auto &vec = (parentNode != nullptr) ? parentNode->children : roots_;
    if (row < 0 || row >= vec.size()) {
        return {};
    }
    return createIndex(row, column, vec.at(row));
}

auto CircuitTreeModel::parent(const QModelIndex &child) const -> QModelIndex {
    if (!child.isValid()) {
        return {};
    }
    Node *node = static_cast<Node *>(child.internalPointer());
    if ((node == nullptr) || (node->parent == nullptr)) {
        return {};
    }
    Node *parentNode = node->parent;
    Node *grand = parentNode->parent;
    const auto &siblings = (grand != nullptr) ? grand->children : roots_;
    const int row = siblings.indexOf(parentNode);
    if (row < 0) {
        return {};
    }
    return createIndex(row, 0, parentNode);
}

auto CircuitTreeModel::rowCount(const QModelIndex &parentIdx) const -> int {
    if (parentIdx.column() > 0) {
        return 0;
    }
    Node *parentNode = nodeFromIndex(parentIdx);
    return (parentNode != nullptr) ? parentNode->children.size()
                                   : roots_.size();
}

auto CircuitTreeModel::columnCount(const QModelIndex & /*parent*/) const
    -> int {
    return 1;
}

auto CircuitTreeModel::data(const QModelIndex &idx,
                            int role) const -> QVariant {
    if (!idx.isValid() || role != Qt::DisplayRole) {
        return {};
    }
    Node *node = static_cast<Node *>(idx.internalPointer());
    if ((node == nullptr) || (node->circuit == nullptr)) {
        return {};
    }
    const auto *c = node->circuit;
    return QStringLiteral("%1 vs %2").arg(c->layoutCell, c->schematicCell);
}

auto CircuitTreeModel::circuitForIndex(const QModelIndex &idx) const
    -> NetgenJsonParser::Report::Circuit * {
    Node *node = nodeFromIndex(idx);
    return (node != nullptr) ? node->circuit : nullptr;
}

CircuitTreeModel::Node *
CircuitTreeModel::nodeFromIndex(const QModelIndex &idx) {
    if (!idx.isValid()) {
        return nullptr;
    }
    return static_cast<Node *>(idx.internalPointer());
}
