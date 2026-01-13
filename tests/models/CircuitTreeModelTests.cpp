#include <QtTest>

#include "models/CircuitTreeModel.hpp"
#include "models/DiffEntryModel.hpp"
#include "models/DiffFilterProxyModel.hpp"

class CircuitTreeModelTests : public QObject {
    Q_OBJECT

  private slots:
    static void builds_tree_and_filters();
};

void CircuitTreeModelTests::builds_tree_and_filters() {
    // Prepare circuits hierarchy: root -> child.
    NetgenJsonParser::Report::Circuit root;
    root.layoutCell = QStringLiteral("rootA");
    root.schematicCell = QStringLiteral("rootB");
    root.index = 0;
    NetgenJsonParser::Report::Circuit child;
    child.layoutCell = QStringLiteral("childA");
    child.schematicCell = QStringLiteral("childB");
    child.index = 1;
    child.isTopLevel = false;
    root.subcircuits.insert(QStringLiteral("childA"), &child);
    QVector<NetgenJsonParser::Report::Circuit> circuits;
    circuits.append(root);
    circuits.append(child);

    CircuitTreeModel treeModel;
    treeModel.setCircuits(&circuits);

    QCOMPARE(treeModel.rowCount(), 1); // one top-level circuit
    QModelIndex rootIdx = treeModel.index(0, 0);
    QVERIFY(rootIdx.isValid());
    QVERIFY(treeModel.circuitForIndex(rootIdx) != nullptr);
    QCOMPARE(treeModel.rowCount(rootIdx), 1); // one child
    QModelIndex childIdx = treeModel.index(0, 0, rootIdx);
    QVERIFY(childIdx.isValid());
    auto *childPtr = CircuitTreeModel::circuitForIndex(childIdx);
    QVERIFY(childPtr);
    QCOMPARE(childPtr->layoutCell, QStringLiteral("childA"));

    // Check proxy filtering by circuit index.
    DiffEntryModel diffModel;
    QVector<NetgenJsonParser::DiffEntry> diffs;
    NetgenJsonParser::DiffEntry diff1;
    diff1.name = QStringLiteral("d1");
    diff1.circuitIndex = 0;
    NetgenJsonParser::DiffEntry diff2;
    diff2.name = QStringLiteral("d2");
    diff2.circuitIndex = 1;
    diffs << diff1 << diff2;
    diffModel.setDiffs(diffs);

    DiffFilterProxyModel proxy;
    proxy.setSourceModel(&diffModel);

    // No circuit filter -> both rows.
    QCOMPARE(proxy.rowCount(), 2);
    // Filter to root only.
    proxy.setAllowedCircuits(QSet<int>({0}));
    QCOMPARE(proxy.rowCount(), 1);
    // Filter to child only.
    proxy.setAllowedCircuits(QSet<int>({1}));
    QCOMPARE(proxy.rowCount(), 1);
    // Filter to both again.
    proxy.setAllowedCircuits(QSet<int>({0, 1}));
    QCOMPARE(proxy.rowCount(), 2);
}

QTEST_MAIN(CircuitTreeModelTests)
#include "CircuitTreeModelTests.moc"
