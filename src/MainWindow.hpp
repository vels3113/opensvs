#pragma once

#include <QMainWindow>

class QLabel;
class QTableView;
class QAction;
class QMenu;
class QStatusBar;

class DiffEntryModel;
class NetgenJsonParser;


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

    DiffEntryModel *diffModel_{nullptr};
    QTableView *diffTable_{nullptr};
    QLabel *deviceMismatchLabel_{nullptr};
    QLabel *netMismatchLabel_{nullptr};
    QLabel *totalDevicesLabel_{nullptr};
    QLabel *totalNetsLabel_{nullptr};
    QLabel *shortsLabel_{nullptr};
    QLabel *opensLabel_{nullptr};
}; 
