#include <QtTest>
#include <QFile>
#include <QTemporaryFile>

#include "parsers/NetgenJsonParser.hpp"

class NetgenJsonParserTests : public QObject
{
    Q_OBJECT

private slots:
    void parses_sample_fixture();
    void parses_tut2_fixture();
    void fails_on_invalid_json();
};

void NetgenJsonParserTests::parses_sample_fixture()
{
    NetgenJsonParser parser;
    const QString fixturePath = QStringLiteral(FIXTURE_PATH);

    auto report = parser.parseFile(fixturePath);
    QVERIFY2(report.ok, qPrintable(QStringLiteral("Expected ok parse, got error: %1").arg(report.error)));

    QCOMPARE(report.circuits.size(), 1);
    const auto &sub = report.circuits.first();

    QCOMPARE(report.summary.deviceMismatches, 0);
    QCOMPARE(report.summary.netMismatches, 0);
    QCOMPARE(report.summary.shorts, 0);
    QCOMPARE(report.summary.opens, 0);
    QCOMPARE(report.summary.totalDevices, 4);
    QCOMPARE(report.summary.totalNets, 5);
    QCOMPARE(report.summary.layoutCell, QStringLiteral("bufferA.spice"));
    QCOMPARE(report.summary.schematicCell, QStringLiteral("bufferB.spice"));

    QCOMPARE(sub.summary.totalDevices, 4);
    QCOMPARE(sub.summary.totalNets, 5);
    QCOMPARE(sub.layoutCell, QStringLiteral("bufferA.spice"));
    QCOMPARE(sub.schematicCell, QStringLiteral("bufferB.spice"));
    QCOMPARE(sub.devicesA, QStringList({"pfet", "nfet"}));
    QCOMPARE(sub.devicesB, QStringList({"pfet", "nfet"}));
    QCOMPARE(sub.subcircuits.size(), 0);

    QCOMPARE(sub.diffs.size(), 16);
    QCOMPARE(sub.diffs[0].type, NetgenJsonParser::DiffType::PropertyMismatch);
    QCOMPARE(sub.diffs[0].name, QStringLiteral("inverter:0/nfet:1001"));
    QCOMPARE(sub.diffs[0].layoutCell, QStringLiteral("bufferA.spice"));
    QCOMPARE(sub.diffs[0].schematicCell, QStringLiteral("bufferB.spice"));
    QVERIFY(sub.diffs[0].details.contains(QStringLiteral("ps")));
    QVERIFY(sub.diffs[0].details.contains(QStringLiteral("7.2e-6")));
    QVERIFY(sub.diffs[0].details.contains(QStringLiteral("(no value)")));
}

void NetgenJsonParserTests::parses_tut2_fixture()
{
    NetgenJsonParser parser;
    const QString fixturePath = QStringLiteral(TUT2_PATH);

    auto report = parser.parseFile(fixturePath);
    QVERIFY2(report.ok, qPrintable(QStringLiteral("Expected ok parse, got error: %1").arg(report.error)));

    QCOMPARE(report.circuits.size(), 2);
    const auto &sub = report.circuits[1];

    QCOMPARE(sub.summary.totalDevices, 2);
    QCOMPARE(sub.summary.totalNets, 5);
    QCOMPARE(sub.layoutCell, QStringLiteral("/home/valerys/opensvs/resources/fixtures/netgen_tutorial/tut2/bufferA.spice"));
    QCOMPARE(sub.schematicCell, QStringLiteral("/home/valerys/opensvs/resources/fixtures/netgen_tutorial/tut2/bufferBx.spice"));
    QCOMPARE(sub.subcircuits.size(), 1);

    QCOMPARE(sub.diffs.size(), 6);
    QCOMPARE(sub.diffs[0].type, NetgenJsonParser::DiffType::NetMismatch);
    QCOMPARE(sub.diffs[0].name, QStringLiteral("Gnd"));
    QCOMPARE(sub.diffs[0].layoutCell, QStringLiteral("/home/valerys/opensvs/resources/fixtures/netgen_tutorial/tut2/bufferA.spice"));
    QCOMPARE(sub.diffs[0].schematicCell, QStringLiteral("/home/valerys/opensvs/resources/fixtures/netgen_tutorial/tut2/bufferBx.spice"));
    QVERIFY(sub.diffs[0].details.contains(QStringLiteral("The following nets are connected only in circuit A: inverted:Gnd (2)")));
    QCOMPARE(sub.diffs[1].name, QStringLiteral("Vdd"));
    QVERIFY(sub.diffs[1].details.contains(QStringLiteral("The following nets are connected only in circuit A: inverted:Vdd (2)")));
    QCOMPARE(sub.diffs[2].name, QStringLiteral("dummy_6"));
    QVERIFY(sub.diffs[2].details.contains(QStringLiteral("No matching net in circuit A for dummy_6 (connected to inverted:proxyVdd (1))")));
    QCOMPARE(sub.diffs[3].name, QStringLiteral("dummy_8"));
    QVERIFY(sub.diffs[3].details.contains(QStringLiteral("No matching net in circuit A for dummy_8 (connected to inverted:proxyVdd (1))")));
    QCOMPARE(sub.diffs[4].name, QStringLiteral("dummy_7"));
    QVERIFY(sub.diffs[4].details.contains(QStringLiteral("No matching net in circuit A for dummy_7 (connected to inverted:proxyGnd (1))")));
    QCOMPARE(sub.diffs[5].name, QStringLiteral("dummy_9"));
    QVERIFY(sub.diffs[5].details.contains(QStringLiteral("No matching net in circuit A for dummy_9 (connected to inverted:proxyGnd (1))")));
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
