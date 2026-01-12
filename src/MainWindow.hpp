#pragma once

#include <QDir>
#include <QMainWindow>
#include <QStringList>

class QLabel;
class QTableView;
class QAction;
class QMenu;
class QStatusBar;
class QComboBox;
class QLineEdit;
class QMenu;
class QStackedWidget;
class QPushButton;
class QDockWidget;
class QPlainTextEdit;
class QLineEdit;
class QTreeView;

#include "models/CircuitTreeModel.hpp"
#include "models/DiffEntryModel.hpp"
#include "models/DiffFilterProxyModel.hpp"
#include "parsers/NetgenJsonParser.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);

    bool loadFile(const QString &path, bool showError = false);

  private:
    void buildUi();
    void buildMenus();
    void setSummary(int device, int net, int shorts, int opens,
                    int totalDevices, int totalNets, const QString &layoutCell,
                    const QString &schematicCell);
    void showStatus(const QString &msg);
    void logEvent(const QString &msg);
    void appendLogToDisk(const QString &line);
    void rebuildRecentFilesMenu();
    void openLogDialog();
    QString logFilePath() const;
    void loadRecentFiles();
    void saveRecentFiles() const;
    QString recentFilesPath() const;
    void updateRecentButtons();
    QString mostRecentFile() const;
    void ensureLogDock();
    void refreshLogView();
    void openLvsDialog();
    void ensureLvsDock();
    void applyCircuitFilter(const QModelIndex &index);

    DiffEntryModel *diffModel_{nullptr};
    DiffFilterProxyModel *proxyModel_{nullptr};
    CircuitTreeModel *circuitTreeModel_{nullptr};
    QTableView *diffTable_{nullptr};
    QTreeView *circuitTree_{nullptr};
    QComboBox *typeFilter_{nullptr};
    QLineEdit *searchField_{nullptr};
    QMenu *recentMenu_{nullptr};
    QStringList recentFiles_;
    QStringList logLines_;
    QStackedWidget *stack_{nullptr};
    QWidget *contentPage_{nullptr};
    QWidget *welcomePage_{nullptr};
    QPushButton *loadButton_{nullptr};
    QLabel *deviceMismatchLabel_{nullptr};
    QLabel *netMismatchLabel_{nullptr};
    QLabel *totalDevicesLabel_{nullptr};
    QLabel *totalNetsLabel_{nullptr};
    QLabel *shortsLabel_{nullptr};
    QLabel *opensLabel_{nullptr};
    QLabel *layoutCellLabel_{nullptr};
    QLabel *schematicCellLabel_{nullptr};
    QDockWidget *logDock_{nullptr};
    QPlainTextEdit *logView_{nullptr};
    QDockWidget *lvsDock_{nullptr};
    QLineEdit *lvsLayoutEdit_{nullptr};
    QLineEdit *lvsSchematicEdit_{nullptr};
    QLineEdit *lvsRulesEdit_{nullptr};
    QVector<NetgenJsonParser::Report::Circuit> circuits_;
    QString lvsLastDir_{QDir::currentPath()};
};
