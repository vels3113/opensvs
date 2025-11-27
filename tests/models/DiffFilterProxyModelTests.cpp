#include <QtTest>

#include "models/DiffEntryModel.hpp"
#include "models/DiffFilterProxyModel.hpp"

class DiffFilterProxyModelTests : public QObject
{
    Q_OBJECT

private slots:
    void filters_by_type_and_search();
};

void DiffFilterProxyModelTests::filters_by_type_and_search()
{
    QVector<NetgenJsonParser::DiffEntry> diffs;
    NetgenJsonParser::DiffEntry first;
    first.type = NetgenJsonParser::DiffType::NetMismatch;
    first.name = QStringLiteral("net_vdd");
    first.details = QStringLiteral("Extra connection");
    diffs.push_back(first);

    NetgenJsonParser::DiffEntry second;
    second.type = NetgenJsonParser::DiffType::DeviceMismatch;
    second.name = QStringLiteral("M1");
    second.details = QStringLiteral("Parameter W differs");
    diffs.push_back(second);

    DiffEntryModel source;
    source.setDiffs(diffs);

    DiffFilterProxyModel proxy;
    proxy.setSourceModel(&source);

    QCOMPARE(proxy.rowCount(), 2);

    proxy.setTypeFilter(QStringLiteral("device_mismatch"));
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setSearchTerm(QStringLiteral("Extra"));
    QCOMPARE(proxy.rowCount(), 0); // no device_mismatch with "Extra"

    proxy.setTypeFilter(QStringLiteral("All"));
    QCOMPARE(proxy.rowCount(), 1); // search still applied

    // Regex search should work
    proxy.setSearchTerm(QStringLiteral("param.*W"));
    QCOMPARE(proxy.rowCount(), 1);

    // Invalid regex falls back to substring search
    proxy.setSearchTerm(QStringLiteral("[invalid"));
    QCOMPARE(proxy.rowCount(), 0);
}

QTEST_MAIN(DiffFilterProxyModelTests)
#include "DiffFilterProxyModelTests.moc"
