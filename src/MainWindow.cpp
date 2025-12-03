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
#include <QPushButton>
#include <QDockWidget>
#include <QStatusBar>
#include <QStackedWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QDateTime>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>

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
    loadRecentFiles();
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

    // Track recent files
    if (!path.isEmpty()) {
        recentFiles_.removeAll(path);
        recentFiles_.prepend(path);
        const int maxRecent = 10;
        while (recentFiles_.size() > maxRecent) {
            recentFiles_.removeLast();
        }
        rebuildRecentFilesMenu();
        saveRecentFiles();
    }
    if (stack_ && contentPage_) {
        stack_->setCurrentWidget(contentPage_);
    }
    return true;
}

void MainWindow::buildUi()
{
    stack_ = new QStackedWidget(this);

    // Welcome page
    welcomePage_ = new QWidget(stack_);
    auto *welcomeLayout = new QVBoxLayout(welcomePage_);
    welcomeLayout->setContentsMargins(16, 16, 16, 16);
    welcomeLayout->setSpacing(12);
    auto *welcomeLabel = new QLabel(tr("OpenSVS\nLoad a netgen JSON report to begin."), welcomePage_);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    loadButton_ = new QPushButton(tr("Load file..."), welcomePage_);
    welcomeLayout->addStretch(1);
    welcomeLayout->addWidget(welcomeLabel, 0, Qt::AlignCenter);
    welcomeLayout->addWidget(loadButton_, 0, Qt::AlignHCenter);
    welcomeLayout->addStretch(2);
    stack_->addWidget(welcomePage_);

    // Content page
    contentPage_ = new QWidget(stack_);
    auto *layout = new QVBoxLayout(contentPage_);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto *filterRow = new QHBoxLayout();
    filterRow->setSpacing(8);
    typeFilter_ = new QComboBox(contentPage_);
    typeFilter_->setObjectName(QStringLiteral("typeFilter"));
    typeFilter_->addItems({tr("All"), tr("net_mismatch"), tr("device_mismatch")});
    searchField_ = new QLineEdit(contentPage_);
    searchField_->setObjectName(QStringLiteral("searchField"));
    searchField_->setPlaceholderText(tr("Search object/details"));
    filterRow->addWidget(new QLabel(tr("Type:"), contentPage_));
    filterRow->addWidget(typeFilter_);
    filterRow->addWidget(new QLabel(tr("Search:"), contentPage_));
    filterRow->addWidget(searchField_, 1);
    layout->addLayout(filterRow);

    auto *summaryGrid = new QGridLayout();
    summaryGrid->setHorizontalSpacing(12);
    summaryGrid->setVerticalSpacing(6);

    auto makeSummaryRow = [&](int row, const QString &labelText, QLabel **valueLabel, const char *objectName) {
        auto *label = new QLabel(labelText, contentPage_);
        auto *value = new QLabel(QStringLiteral("0"), contentPage_);
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

    proxyModel_->setSourceModel(diffModel_);

    diffTable_ = new QTableView(contentPage_);
    diffTable_->setObjectName(QStringLiteral("diffTableView"));
    diffTable_->setModel(proxyModel_);
    diffTable_->horizontalHeader()->setStretchLastSection(true);
    diffTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    diffTable_->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(diffTable_, 1);

    stack_->addWidget(contentPage_);
    stack_->setCurrentWidget(welcomePage_);
    setCentralWidget(stack_);

    connect(typeFilter_, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        proxyModel_->setTypeFilter(text);
    });
    connect(searchField_, &QLineEdit::textChanged, this, [this](const QString &text) {
        proxyModel_->setSearchTerm(text);
    });

    connect(loadButton_, &QPushButton::clicked, this, [this]() {
        const QString startDir = mostRecentFile().isEmpty()
            ? QStringLiteral("./resources/fixtures")
            : QFileInfo(mostRecentFile()).absolutePath();
        const QString path = QFileDialog::getOpenFileName(this,
                                                         tr("Open netgen JSON report"),
                                                         startDir,
                                                         tr("JSON files (*.json);;All files (*)"));
        if (!path.isEmpty()) {
            if (loadFile(path, true)) {
                stack_->setCurrentWidget(contentPage_);
            }
        }
    });

    updateRecentButtons();
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

    recentMenu_ = fileMenu->addMenu(tr("Recent Files"));
    rebuildRecentFilesMenu();

    auto *runMenu = menuBar()->addMenu(tr("&Run"));
    runMenu->addMenu(tr("LVS"));

    ensureLogDock();
    auto *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(logDock_->toggleViewAction());

    auto *helpMenu = menuBar()->addMenu(tr("&Help"));
    auto *aboutAction = new QAction(tr("About OpenSVS"), this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        const QString text = tr("<b>OpenSVS</b><br>Version: %1<br>Qt: %2<br><br>Qt6 GUI for viewing netgen JSON reports.")
                                .arg(QCoreApplication::applicationVersion())
                                .arg(QLatin1String(qVersion()));
        QMessageBox::about(this, tr("About OpenSVS"), text);
    });
    helpMenu->addAction(aboutAction);
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
    const QString stamped = QStringLiteral("[%1] %2")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs))
            .arg(msg);

    logLines_.prepend(stamped);
    const int maxLines = 50;
    while (logLines_.size() > maxLines) {
        logLines_.removeLast();
    }
    appendLogToDisk(stamped);
    refreshLogView();
}

void MainWindow::appendLogToDisk(const QString &line)
{
    const QString path = logFilePath();
    if (path.isEmpty()) return;

    QFile f(path);
    if (f.size() > 1024 * 1024) { // rotate at ~1MB
        f.remove(path + QStringLiteral(".1"));
        f.rename(path + QStringLiteral(".1"));
    }
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out << line << '\n';
    }
}

void MainWindow::rebuildRecentFilesMenu()
{
    if (!recentMenu_) return;
    recentMenu_->clear();
    if (recentFiles_.isEmpty()) {
        QAction *empty = recentMenu_->addAction(tr("No recent files"));
        empty->setEnabled(false);
        return;
    }
    for (const QString &path : recentFiles_) {
        QAction *act = recentMenu_->addAction(path);
        connect(act, &QAction::triggered, this, [this, path]() {
            loadFile(path, true);
        });
    }
}

void MainWindow::openLogDialog()
{
    ensureLogDock();
    refreshLogView();
    logDock_->show();
    logDock_->raise();
    logDock_->activateWindow();
}

QString MainWindow::logFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (dir.isEmpty()) {
        return {};
    }
    return dir + QStringLiteral("/opensvs.log");
}

void MainWindow::loadRecentFiles()
{
    const QString path = recentFilesPath();
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QStringList lines;
    QTextStream in(&f);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    }
    const int maxRecent = 10;
    while (lines.size() > maxRecent) {
        lines.removeLast();
    }
    recentFiles_ = lines;
}

void MainWindow::saveRecentFiles() const
{
    const QString path = recentFilesPath();
    if (path.isEmpty()) return;
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&f);
        for (const QString &p : recentFiles_) {
            out << p << '\n';
        }
    }
}

QString MainWindow::recentFilesPath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (dir.isEmpty()) return {};
    return dir + QStringLiteral("/opensvs_recent.txt");
}

void MainWindow::updateRecentButtons()
{
    // No buttons to toggle (load recent removed); kept for future hook.
}

QString MainWindow::mostRecentFile() const
{
    if (recentFiles_.isEmpty()) return {};
    return recentFiles_.front();
}

void MainWindow::ensureLogDock()
{
    if (logDock_) return;
    logDock_ = new QDockWidget(tr("Session Log"), this);
    logDock_->setObjectName(QStringLiteral("logDock"));
    logDock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    logDock_->setAllowedAreas(Qt::AllDockWidgetAreas);
    logView_ = new QPlainTextEdit(logDock_);
    logView_->setReadOnly(true);
    logDock_->setWidget(logView_);
    addDockWidget(Qt::BottomDockWidgetArea, logDock_);
    logDock_->hide();
}

void MainWindow::refreshLogView()
{
    if (logView_) {
        logView_->setPlainText(logLines_.join('\n'));
        logView_->moveCursor(QTextCursor::End);
    }
}
