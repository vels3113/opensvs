#include <QtTest>
#include <QFile>
#include <QTemporaryFile>

#include "parsers/NetgenJsonParser.hpp"

class NetgenJsonParserTests : public QObject
{
    Q_OBJECT

private slots:
    void parses_sample_fixture();
    void fails_on_invalid_json();
};

void NetgenJsonParserTests::parses_sample_fixture()
{
    NetgenJsonParser parser;
    const QString fixturePath = QStringLiteral(FIXTURE_PATH);

    auto report = parser.parseFile(fixturePath);
    QVERIFY2(report.ok, qPrintable(QStringLiteral("Expected ok parse, got error: %1").arg(report.error)));

    QCOMPARE(report.summary.deviceMismatches, 1);
    QCOMPARE(report.summary.netMismatches, 1);
    QCOMPARE(report.summary.shorts, 0);
    QCOMPARE(report.summary.opens, 0);
    QCOMPARE(report.summary.totalDevices, 10);
    QCOMPARE(report.summary.totalNets, 12);

    QCOMPARE(report.diffs.size(), 2);
    QCOMPARE(report.diffs[0].type, NetgenJsonParser::DiffType::NetMismatch);
    QCOMPARE(report.diffs[0].name, QStringLiteral("net_vdd"));
    QCOMPARE(report.diffs[0].layoutCell, QStringLiteral("top_layout"));
    QCOMPARE(report.diffs[0].schematicCell, QStringLiteral("top_schem"));
    QVERIFY(report.diffs[0].details.contains(QStringLiteral("Extra connection")));
}

void NetgenJsonParserTests::fails_on_invalid_json()
{
    NetgenJsonParser parser;

    QTemporaryFile tmpFile;
    QVERIFY(tmpFile.open());
    // Write malformed JSON
    tmpFile.write("{ invalid json ");
    tmpFile.flush();

    auto report = parser.parseFile(tmpFile.fileName());
    QVERIFY(!report.ok);
    QVERIFY(!report.error.isEmpty());
}

QTEST_MAIN(NetgenJsonParserTests)
#include "NetgenJsonParserTests.moc"
