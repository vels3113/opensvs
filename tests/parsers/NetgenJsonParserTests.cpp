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

    QCOMPARE(report.summary.deviceMismatches, 0);
    QCOMPARE(report.summary.netMismatches, 0);
    QCOMPARE(report.summary.shorts, 0);
    QCOMPARE(report.summary.opens, 0);
    QCOMPARE(report.summary.totalDevices, 4);
    QCOMPARE(report.summary.totalNets, 5);

    QCOMPARE(report.diffs.size(), 16);
    QCOMPARE(report.diffs[0].type, NetgenJsonParser::DiffType::PropertyMismatch);
    QCOMPARE(report.diffs[0].name, QStringLiteral("inverter:0/nfet:1001"));
    QCOMPARE(report.diffs[0].layoutCell, QStringLiteral("bufferA.spice"));
    QCOMPARE(report.diffs[0].schematicCell, QStringLiteral("bufferB.spice"));
    QVERIFY(report.diffs[0].details.contains(QStringLiteral("ps")));
    QVERIFY(report.diffs[0].details.contains(QStringLiteral("7.2e-6")));
    QVERIFY(report.diffs[0].details.contains(QStringLiteral("(no value)")));
}

void NetgenJsonParserTests::fails_on_invalid_json()
{
    NetgenJsonParser parser;

    QTemporaryFile tmpFile;
    QVERIFY(tmpFile.open());
    tmpFile.write("{ invalid json ");
    tmpFile.flush();

    auto report = parser.parseFile(tmpFile.fileName());
    QVERIFY(!report.ok);
    QVERIFY(!report.error.isEmpty());
}

QTEST_MAIN(NetgenJsonParserTests)
#include "NetgenJsonParserTests.moc"
