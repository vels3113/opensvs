#pragma once

#include <QString>
#include <QVector>

class NetgenJsonParser
{
public:
    enum class DiffType {
        Unknown,
        NetMismatch,
        DeviceMismatch
    };

    struct Summary {
        int deviceMismatches = 0;
        int netMismatches = 0;
        int shorts = 0;
        int opens = 0;
        int totalDevices = 0;
        int totalNets = 0;
    };

    struct DiffEntry {
        DiffType type = DiffType::Unknown;
        QString name;
        QString layoutCell;
        QString schematicCell;
        QString details;
    };

    struct Report {
        bool ok = false;
        QString error;
        Summary summary;
        QVector<DiffEntry> diffs;
    };

    Report parseFile(const QString &path) const;

    static QString toTypeString(DiffType type);
};
