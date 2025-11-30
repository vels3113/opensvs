#include "MainWindow.hpp"

#include <QAbstractItemView>
#include <QAbstractItemView>
#include <QAction>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTableView>
#include <QVBoxLayout>

#include "models/DiffEntryModel.hpp"
#include "models/DiffFilterProxyModel.hpp"
#include "parsers/NetgenJsonParser.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , diffModel_(new DiffEntryModel(this))
    , proxyModel_(new DiffFilterProxyModel(this))
{
    setWindowTitle(tr("OpenSVS"));
    setMinimumSize(800, 600);
    buildUi();
    buildMenus();
    showStatus(tr("Use File -> Open to load a JSON report."));
    logEvent(tr("Application started"));
}

bool MainWindow::loadFile(const QString &path, bool showError)
{
    NetgenJsonParser parser;
    auto report = parser.parseFile(path);
    if (!report.ok) {
        if (showError) {
            QMessageBox::critical(this, tr("Failed to load"), report.error);
        }
        return false;
    }

    diffModel_->setDiffs(report.diffs);
    proxyModel_->invalidate();
    setSummary(report.summary.deviceMismatches,
               report.summary.netMismatches,
               report.summary.shorts,
               report.summary.opens,
               report.summary.totalDevices,
               report.summary.totalNets);
    const QString msg = tr("Loaded %1 diffs from %2").arg(report.diffs.size()).arg(path);
    showStatus(msg);
    logEvent(msg);
    return true;
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto *summaryGrid = new QGridLayout();
    summaryGrid->setHorizontalSpacing(12);
    summaryGrid->setVerticalSpacing(6);

    auto makeSummaryRow = [&](int row, const QString &labelText, QLabel **valueLabel, const char *objectName) {
        auto *label = new QLabel(labelText, central);
        auto *value = new QLabel(QStringLiteral("0"), central);
        value->setObjectName(QString::fromLatin1(objectName));
        summaryGrid->addWidget(label, row, 0);
        summaryGrid->addWidget(value, row, 1);
        *valueLabel = value;
    };

    makeSummaryRow(0, tr("Device mismatches:"), &deviceMismatchLabel_, "summary_device_mismatches");
    makeSummaryRow(1, tr("Net mismatches:"), &netMismatchLabel_, "summary_net_mismatches");
    makeSummaryRow(2, tr("Shorts:"), &shortsLabel_, "summary_shorts");
    makeSummaryRow(3, tr("Opens:"), &opensLabel_, "summary_opens");
    makeSummaryRow(4, tr("Total devices:"), &totalDevicesLabel_, "summary_total_devices");
    makeSummaryRow(5, tr("Total nets:"), &totalNetsLabel_, "summary_total_nets");

    layout->addLayout(summaryGrid);

    auto *filterRow = new QHBoxLayout();
    filterRow->setSpacing(8);
    typeFilter_ = new QComboBox(central);
    typeFilter_->setObjectName(QStringLiteral("typeFilter"));
    typeFilter_->addItems({tr("All"), tr("net_mismatch"), tr("device_mismatch")});
    searchField_ = new QLineEdit(central);
    searchField_->setObjectName(QStringLiteral("searchField"));
    searchField_->setPlaceholderText(tr("Search object/details"));
    filterRow->addWidget(new QLabel(tr("Type:"), central));
    filterRow->addWidget(typeFilter_);
    filterRow->addWidget(new QLabel(tr("Search:"), central));
    filterRow->addWidget(searchField_, 1);
    layout->addLayout(filterRow);

    proxyModel_->setSourceModel(diffModel_);

    diffTable_ = new QTableView(central);
    diffTable_->setObjectName(QStringLiteral("diffTableView"));
    diffTable_->setModel(proxyModel_);
    diffTable_->horizontalHeader()->setStretchLastSection(true);
    diffTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    diffTable_->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(diffTable_, 1);

    setCentralWidget(central);

    connect(typeFilter_, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        proxyModel_->setTypeFilter(text);
    });
    connect(searchField_, &QLineEdit::textChanged, this, [this](const QString &text) {
        proxyModel_->setSearchTerm(text);
    });
}

void MainWindow::buildMenus()
{
    auto *fileMenu = menuBar()->addMenu(tr("&File"));
    auto *openAction = new QAction(tr("&Open JSON..."), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this,
                                                         tr("Open netgen JSON report"),
                                                         QStringLiteral("./resources/fixtures"),
                                                         tr("JSON files (*.json);;All files (*)"));
        if (!path.isEmpty()) {
            loadFile(path, true);
        }
    });
    fileMenu->addAction(openAction);
    auto *logAction = new QAction(tr("View Log"), this);
    connect(logAction, &QAction::triggered, this, [this]() {
        openLogDialog();
    });
    fileMenu->addAction(logAction);
}

void MainWindow::setSummary(int device, int net, int shorts, int opens, int totalDevices, int totalNets)
{
    deviceMismatchLabel_->setText(QString::number(device));
    netMismatchLabel_->setText(QString::number(net));
    shortsLabel_->setText(QString::number(shorts));
    opensLabel_->setText(QString::number(opens));
    totalDevicesLabel_->setText(QString::number(totalDevices));
    totalNetsLabel_->setText(QString::number(totalNets));
}

void MainWindow::showStatus(const QString &msg)
{
    if (QStatusBar *sb = statusBar()) {
        sb->showMessage(msg, 5000);
    }
}

void MainWindow::logEvent(const QString &msg)
{
    logLines_.prepend(msg);
    const int maxLines = 50;
    while (logLines_.size() > maxLines) {
        logLines_.removeLast();
    }
}


void MainWindow::openLogDialog()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Session Log"));
    auto *layout = new QVBoxLayout(&dlg);
    auto *text = new QPlainTextEdit(&dlg);
    text->setReadOnly(true);
    text->setPlainText(logLines_.join('\n'));
    layout->addWidget(text);
    dlg.resize(500, 300);
    dlg.exec();
}
