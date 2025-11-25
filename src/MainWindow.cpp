#include "MainWindow.h"

#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("OpenSVS"));
    setMinimumSize(800, 600);
    buildUi();
}

void MainWindow::buildUi()
{
    placeholderLabel_ = new QLabel(tr("OpenSVS placeholder"), this);
    placeholderLabel_->setAlignment(Qt::AlignCenter);
    setCentralWidget(placeholderLabel_);
}
