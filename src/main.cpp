#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QLoggingCategory>

#include "MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("opensvs"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.3"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("OpenSVS netgen JSON viewer"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(
        QStringLiteral("file"),
        QStringLiteral("Optional netgen JSON report to load on startup."));
    parser.process(app);

    MainWindow window;

    const QStringList positional = parser.positionalArguments();
    if (!positional.isEmpty()) {
        const QString filePath =
            QFileInfo(positional.first()).absoluteFilePath();
        if (!window.loadFile(filePath, /*showError=*/true)) {
            qWarning() << "Failed to load file:" << filePath;
        }
    }

    window.show();
    return app.exec();
}
