#include <QtTest>

#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QStackedWidget>

#include "MainWindow.hpp"

class MainWindowSmokeTests : public QObject
{
    Q_OBJECT

private slots:
    void shows_welcome_and_populates_after_load();
};

void MainWindowSmokeTests::shows_welcome_and_populates_after_load()
{
    MainWindow window;
    auto *stack = window.findChild<QStackedWidget*>(QString(), Qt::FindChildrenRecursively);
    QVERIFY(stack);
    QCOMPARE(stack->currentIndex(), 0); // welcome page
    auto buttons = window.findChildren<QPushButton*>(QString(), Qt::FindChildrenRecursively);
    QVERIFY(!buttons.isEmpty());

    const QString fixturePath = QStringLiteral(FIXTURE_PATH);

    QVERIFY2(window.loadFile(fixturePath, false), "Expected fixture load to succeed");

    auto *table = window.findChild<QTableView*>(QStringLiteral("diffTableView"));
    QVERIFY(table);
    QVERIFY(table->model());
    QCOMPARE(table->model()->rowCount(), 16);

    auto devices_mismatches = window.findChild<QLabel*>(QStringLiteral("summary_device_mismatches"));
    auto nets_mismatches = window.findChild<QLabel*>(QStringLiteral("summary_net_mismatches"));
    auto totalDevices = window.findChild<QLabel*>(QStringLiteral("summary_total_devices"));
    auto totalNets = window.findChild<QLabel*>(QStringLiteral("summary_total_nets"));
    QVERIFY(devices_mismatches && nets_mismatches && totalDevices && totalNets);

    QCOMPARE(devices_mismatches->text(), QStringLiteral("0"));
    QCOMPARE(nets_mismatches->text(), QStringLiteral("0"));
    QCOMPARE(totalDevices->text(), QStringLiteral("4"));
    QCOMPARE(totalNets->text(), QStringLiteral("5"));

    auto *typeFilter = window.findChild<QComboBox*>(QStringLiteral("typeFilter"));
    auto *searchField = window.findChild<QLineEdit*>(QStringLiteral("searchField"));
    QVERIFY(typeFilter && searchField);

    typeFilter->setCurrentText(QStringLiteral("device_mismatch"));
    QCOMPARE(table->model()->rowCount(), 0);

    searchField->setText(QStringLiteral("nope"));
    QCOMPARE(table->model()->rowCount(), 0);
}

QTEST_MAIN(MainWindowSmokeTests)
#include "MainWindowSmokeTests.moc"
