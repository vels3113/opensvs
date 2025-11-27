#include <QtTest>

#include <QLabel>
#include <QTableView>

#include "MainWindow.hpp"

class MainWindowSmokeTests : public QObject
{
    Q_OBJECT

private slots:
    void loads_fixture_and_populates_ui();
};

void MainWindowSmokeTests::loads_fixture_and_populates_ui()
{
    MainWindow window;
    const QString fixturePath = QStringLiteral(FIXTURE_PATH);

    QVERIFY2(window.loadFile(fixturePath, false), "Expected fixture load to succeed");

    auto *table = window.findChild<QTableView*>(QStringLiteral("diffTableView"));
    QVERIFY(table);
    QVERIFY(table->model());
    QCOMPARE(table->model()->rowCount(), 2);

    auto devices_mismatches = window.findChild<QLabel*>(QStringLiteral("summary_device_mismatches"));
    auto nets_mismatches = window.findChild<QLabel*>(QStringLiteral("summary_net_mismatches"));
    auto totalDevices = window.findChild<QLabel*>(QStringLiteral("summary_total_devices"));
    auto totalNets = window.findChild<QLabel*>(QStringLiteral("summary_total_nets"));
    QVERIFY(devices_mismatches && nets_mismatches && totalDevices && totalNets);

    QCOMPARE(devices_mismatches->text(), QStringLiteral("1"));
    QCOMPARE(nets_mismatches->text(), QStringLiteral("1"));
    QCOMPARE(totalDevices->text(), QStringLiteral("10"));
    QCOMPARE(totalNets->text(), QStringLiteral("12"));
}

QTEST_MAIN(MainWindowSmokeTests)
#include "MainWindowSmokeTests.moc"
