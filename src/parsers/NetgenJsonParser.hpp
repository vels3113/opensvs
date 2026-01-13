#pragma once

#include <QHash>
#include <QString>
#include <QVector>

class NetgenJsonParser {
  public:
    enum class DiffType : char {
        Unknown,
        NetMismatch,
        DeviceMismatch,
        InstanceMismatch,
        PropertyMismatch
    };

    struct Summary {
        int deviceMismatches = 0;
        int netMismatches = 0;
        int shorts = 0;
        int opens = 0;
        int totalDevices = 0;
        int totalNets = 0;
        QString layoutCell;
        QString schematicCell;
    };

    struct DiffEntry {
        DiffType type = DiffType::Unknown;
        enum class Subtype : char {
            Unknown,
            MissingParameter,
            MissingConnection,
            UnmatchedConnections,
            NoMatchingNet,
            MissingInstance,
            NoMatchingInstance
        };
        Subtype subtype = Subtype::Unknown;
        QString name;
        QString layoutCell;
        QString schematicCell;
        QString details;
        long long circuitIndex = -1;
    };

    struct Report {
        bool ok = false;
        QString error;
        Summary summary;
        struct Circuit {
            Summary summary;
            QString layoutCell;
            QString schematicCell;
            QStringList devicesA;
            QStringList devicesB;
            QVector<DiffEntry> diffs;
            bool isTopLevel{true};
            QHash<QString, Circuit *> subcircuits;
            long long index = -1;
        };
        QVector<Circuit> circuits;
    };

    static auto parseFile(const QString &path) -> Report;

    static auto toTypeString(DiffType type) -> QString;
    static auto toSubtypeString(DiffEntry::Subtype subtype) -> QString;
};
