#include <QtTest>

#include "parsers/NetgenJsonParser.hpp"
#include "models/DiffEntryModel.hpp"

class DiffEntryModelTests : public QObject
{
    Q_OBJECT

private slots:
    void populates_rows_and_columns();
};

void DiffEntryModelTests::populates_rows_and_columns()
{
    QVector<NetgenJsonParser::DiffEntry> diffs;
    NetgenJsonParser::DiffEntry first;
    first.type = NetgenJsonParser::DiffType::NetMismatch;
    first.name = QStringLiteral("net_vdd");
    first.layoutCell = QStringLiteral("top_layout");
    first.schematicCell = QStringLiteral("top_schem");
    first.details = QStringLiteral("Extra connection on layout side");
    diffs.push_back(first);

    NetgenJsonParser::DiffEntry second;
    second.type = NetgenJsonParser::DiffType::DeviceMismatch;
    second.name = QStringLiteral("M1");
    second.layoutCell = QStringLiteral("top_layout");
    second.schematicCell = QStringLiteral("top_schem");
    second.details = QStringLiteral("Parameter W differs");
    diffs.push_back(second);

    DiffEntryModel model;
    model.setDiffs(diffs);

    QCOMPARE(model.rowCount(), diffs.size());
    QCOMPARE(model.columnCount(), 6);

    const QModelIndex idx = model.index(0, 0);
    QCOMPARE(model.data(idx, Qt::DisplayRole).toString(), QStringLiteral("net_mismatch"));
    QCOMPARE(model.data(model.index(0, 2), Qt::DisplayRole).toString(), QStringLiteral("net_vdd"));
    QCOMPARE(model.data(model.index(0, 3), Qt::DisplayRole).toString(), QStringLiteral("top_layout"));
    QCOMPARE(model.data(model.index(0, 4), Qt::DisplayRole).toString(), QStringLiteral("top_schem"));
    QVERIFY(model.data(model.index(0, 5), Qt::DisplayRole).toString().contains(QStringLiteral("Extra connection")));
}

QTEST_MAIN(DiffEntryModelTests)
#include "DiffEntryModelTests.moc"
