#pragma once

#include <QMainWindow>

class QLabel;
class QTableView;

class DiffEntryModel;
class NetgenJsonParser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    bool loadFile(const QString &path);

private:
    void buildUi();
    void setSummary(int device, int net, int shorts, int opens, int totalDevices, int totalNets);

    DiffEntryModel *diffModel_{nullptr};
    QTableView *diffTable_{nullptr};
    QLabel *deviceMismatchLabel_{nullptr};
    QLabel *netMismatchLabel_{nullptr};
    QLabel *totalDevicesLabel_{nullptr};
    QLabel *totalNetsLabel_{nullptr};
    QLabel *shortsLabel_{nullptr};
    QLabel *opensLabel_{nullptr};
};
