#pragma once

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

#include "models/DiffEntryModel.hpp"
#include "models/DiffFilterProxyModel.hpp"
#include "parsers/NetgenJsonParser.hpp"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    bool loadFile(const QString &path, bool showError = false);

private:
    void buildUi();
    void buildMenus();
    void setSummary(int device, int net, int shorts, int opens, int totalDevices, int totalNets);
    void showStatus(const QString &msg);
    void logEvent(const QString &msg);
    void appendLogToDisk(const QString &line);
    void rebuildRecentFilesMenu();
    void openLogDialog();
    QString logFilePath() const;

    DiffEntryModel *diffModel_{nullptr};
    DiffFilterProxyModel *proxyModel_{nullptr};
    QTableView *diffTable_{nullptr};
    QComboBox *typeFilter_{nullptr};
    QLineEdit *searchField_{nullptr};
    QMenu *recentMenu_{nullptr};
    QStringList recentFiles_;
    QStringList logLines_;
    QLabel *deviceMismatchLabel_{nullptr};
    QLabel *netMismatchLabel_{nullptr};
    QLabel *totalDevicesLabel_{nullptr};
    QLabel *totalNetsLabel_{nullptr};
    QLabel *shortsLabel_{nullptr};
    QLabel *opensLabel_{nullptr};
};
