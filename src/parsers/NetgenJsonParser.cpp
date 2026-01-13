#include "parsers/NetgenJsonParser.hpp"

#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QStringList>
#include <algorithm>
#include <qjsonarray.h>
#include <qjsonvalue.h>

auto NetgenJsonParser::parseFile(const QString &path)
    -> NetgenJsonParser::Report {
    Report report;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        report.error = QStringLiteral("Failed to open file: %1").arg(path);
        return report;
    }

    const QByteArray data = file.readAll();
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        report.error = QStringLiteral("JSON parse error: %1")
                           .arg(parseError.errorString());
        return report;
    }

    if (!doc.isArray()) {
        report.error = QStringLiteral("Root JSON is not an object");
        return report;
    }

    const QJsonArray arr = doc.array();
    if (arr.isEmpty()) {
        report.error =
            QStringLiteral("Root JSON array is empty or has no object");
        return report;
    }

    report.circuits.reserve(arr.size());
    int circuitIdx = 0;
    for (const QJsonValueConstRef &rootVal : arr) {
        if (!rootVal.isObject()) {
            continue;
        }
        const QJsonObject rootObj = rootVal.toObject();
        const QJsonValue namesVal = rootObj.value(QStringLiteral("name"));
        if (!namesVal.isArray()) {
            continue;
        }
        const QJsonArray namesArr = namesVal.toArray();
        if (namesArr.isEmpty()) {
            continue;
        }
        Report::Circuit sub;
        sub.index = circuitIdx;

        const QJsonArray netsArr =
            rootObj.value(QStringLiteral("nets")).toArray();
        if (!netsArr.isEmpty()) {
            sub.summary.totalNets = netsArr.first().toInt(0);
        }

        const QJsonArray devicesArr =
            rootObj.value(QStringLiteral("devices")).toArray();
        if (!devicesArr.isEmpty() && devicesArr.first().isArray()) {
            int total = 0;
            for (const QJsonValueConstRef &devVal :
                 devicesArr.first().toArray()) {
                if (devVal.isArray()) {
                    const QJsonArray pair = devVal.toArray();
                    if (pair.size() > 1) {
                        if (pair.at(0).isString()) {
                            sub.devicesA.append(pair.at(0).toString());
                        }
                        total += pair.at(1).toInt(0);
                    }
                }
            }
            sub.summary.totalDevices = total;
            if (devicesArr.size() > 1) {
                int totalB = 0;
                for (const QJsonValueConstRef &devVal :
                     devicesArr.at(1).toArray()) {
                    if (devVal.isArray()) {
                        const QJsonArray pair = devVal.toArray();
                        if (pair.size() > 1) {
                            if (pair.at(0).isString()) {
                                sub.devicesB.append(pair.at(0).toString());
                            }
                            totalB += pair.at(1).toInt(0);
                        }
                    }
                }
                if (total != totalB) {
                    sub.summary.deviceMismatches = 1;
                }
            }
        }

        sub.layoutCell =
            !namesArr.empty() ? namesArr.at(0).toString() : QString();
        sub.schematicCell =
            namesArr.size() > 1 ? namesArr.at(1).toString() : QString();
        const QJsonArray propertiesArr =
            rootObj.value(QStringLiteral("properties")).toArray();
        for (const QJsonValueConstRef &val : propertiesArr) {
            const QJsonArray pairArr = val.toArray();
            if (pairArr.size() < 2) {
                continue;
            }
            const QJsonArray deviceA = pairArr.at(0).toArray();
            const QJsonArray deviceB = pairArr.at(1).toArray();
            if (deviceA.size() < 2 || deviceB.size() < 2) {
                continue;
            }
            const QString nameA = deviceA.at(0).toString();
            const QString nameB = deviceB.at(0).toString();
            const QJsonArray paramsA = deviceA.at(1).toArray();
            const QJsonArray paramsB = deviceB.at(1).toArray();
            const int maxParams = std::max(static_cast<int>(paramsA.size()),
                                           static_cast<int>(paramsB.size()));
            for (int i = 0; i < maxParams; ++i) {
                const QJsonArray paramA =
                    i < paramsA.size() ? paramsA.at(i).toArray() : QJsonArray();
                const QJsonArray paramB =
                    i < paramsB.size() ? paramsB.at(i).toArray() : QJsonArray();
                const QString paramNameA =
                    !paramA.isEmpty() ? paramA.at(0).toString() : QString();
                const QString paramNameB =
                    !paramB.isEmpty() ? paramB.at(0).toString() : QString();
                const bool missingParamA = paramNameA.contains(
                    QStringLiteral("(no matching parameter)"),
                    Qt::CaseSensitive);
                const bool missingParamB = paramNameB.contains(
                    QStringLiteral("(no matching parameter)"),
                    Qt::CaseSensitive);

                QString param = QStringLiteral("unknown");
                QString valA = QStringLiteral("(missing)");
                QString valB = QStringLiteral("(missing)");

                if (missingParamA && !missingParamB) {
                    if (!paramNameB.isEmpty()) {
                        param = paramNameB;
                    }
                } else if (missingParamB && !missingParamA) {
                    if (!paramNameA.isEmpty()) {
                        param = paramNameA;
                    }
                } else {
                    if (!paramNameA.isEmpty()) {
                        param = paramNameA;
                    } else if (!paramNameB.isEmpty()) {
                        param = paramNameB;
                    }
                }

                if (paramA.size() > 1) {
                    valA = paramA.at(1).toString();
                }
                if (paramB.size() > 1) {
                    valB = paramB.at(1).toString();
                }

                if (valA == valB) {
                    continue;
                }
                DiffEntry entry;
                entry.type = DiffType::PropertyMismatch;
                entry.subtype = DiffEntry::Subtype::MissingParameter;
                entry.name = !nameA.isEmpty() ? nameA : nameB;
                entry.layoutCell = sub.layoutCell;
                entry.schematicCell = sub.schematicCell;
                entry.details =
                    QStringLiteral("%1: %2 vs %3").arg(param, valA, valB);
                entry.circuitIndex = circuitIdx;
                sub.diffs.push_back(entry);
            }
        }

        const QJsonArray badnetsArr =
            rootObj.value(QStringLiteral("badnets")).toArray();

        struct NetInfo {
            QString rawName;
            QStringList connections;
        };
        QHash<QString, NetInfo> netsA;
        QHash<QString, NetInfo> netsB;
        QStringList nameOrder;

        auto normalizeName = [](const QString &n) {
            QString lower = n.trimmed().toLower();
            if (lower == QStringLiteral("gnd") ||
                lower == QStringLiteral("0")) {
                return QStringLiteral("0");
            }
            return lower;
        };
        auto connectionList = [](const QJsonArray &arr) {
            QStringList parts;
            for (const QJsonValueConstRef &ref_conn : arr) {
                const QJsonArray conn = ref_conn.toArray();
                if (conn.size() >= 2) {
                    QString dev = conn.at(0).toString();
                    const QString port = conn.at(1).toString();
                    const int count = conn.size() > 2 ? conn.at(2).toInt() : 0;
                    parts << QStringLiteral("%1:%2 (%3)")
                                 .arg(dev, port)
                                 .arg(count);
                }
            }
            return parts;
        };
        auto captureNet = [&](const QJsonArray &netArr,
                              QHash<QString, NetInfo> &dest) {
            if (netArr.size() < 2) {
                return;
            }
            NetInfo info;
            info.rawName = netArr.at(0).toString();
            if (info.rawName.contains(QStringLiteral("(no matching net)"),
                                      Qt::CaseInsensitive)) {
                return;
            }
            info.connections = connectionList(netArr.at(1).toArray());
            const QString key = normalizeName(info.rawName);
            if (!dest.contains(key)) {
                dest.insert(key, info);
                if (!nameOrder.contains(key)) {
                    nameOrder.append(key);
                }
            } else {
                dest[key] = info;
            }
        };

        for (const QJsonValueConstRef &val : badnetsArr) {
            if (!val.isArray()) {
                continue;
            }
            QJsonArray pairArr = val.toArray();
            if (pairArr.size() == 1 && pairArr.at(0).isArray()) {
                pairArr = pairArr.at(0).toArray();
            }
            if (pairArr.size() == 2 && pairArr.at(0).isArray() &&
                pairArr.at(1).isArray()) {
                const QJsonArray netsListA = pairArr.at(0).toArray();
                const QJsonArray netsListB = pairArr.at(1).toArray();
                for (const QJsonValueConstRef &netA : netsListA) {
                    if (netA.isArray()) {
                        captureNet(netA.toArray(), netsA);
                    }
                }
                for (const QJsonValueConstRef &netB : netsListB) {
                    if (netB.isArray()) {
                        captureNet(netB.toArray(), netsB);
                    }
                }
            }
        }

        for (const QString &name : nameOrder) {
            const bool hasA = netsA.contains(name);
            const bool hasB = netsB.contains(name);
            if (hasA && hasB) {
                const auto &netA = netsA.value(name);
                const auto &netB = netsB.value(name);
                const QSet<QString> setA(netA.connections.begin(),
                                         netA.connections.end());
                const QSet<QString> setB(netB.connections.begin(),
                                         netB.connections.end());
                QStringList onlyA;
                for (const auto &conn : netA.connections) {
                    if (!setB.contains(conn)) {
                        onlyA.append(conn);
                    }
                }
                QStringList onlyB;
                for (const auto &conn : netB.connections) {
                    if (!setA.contains(conn)) {
                        onlyB.append(conn);
                    }
                }
                if (!onlyA.isEmpty() || !onlyB.isEmpty()) {
                    DiffEntry entry;
                    entry.type = DiffType::NetMismatch;
                    const bool bothSides = !onlyA.isEmpty() && !onlyB.isEmpty();
                    entry.subtype =
                        bothSides ? DiffEntry::Subtype::UnmatchedConnections
                                  : DiffEntry::Subtype::MissingConnection;
                    entry.name =
                        !netA.rawName.isEmpty() ? netA.rawName : netB.rawName;
                    entry.layoutCell = sub.layoutCell;
                    entry.schematicCell = sub.schematicCell;
                    QStringList parts;
                    if (!onlyA.isEmpty()) {
                        parts << QStringLiteral(
                                     "The following pins are connected only in "
                                     "Layout circuit: %1")
                                     .arg(onlyA.join(QStringLiteral(", ")));
                    }
                    if (!onlyB.isEmpty()) {
                        parts << QStringLiteral(
                                     "The following pins are connected only in "
                                     "Schematics circuit: %1")
                                     .arg(onlyB.join(QStringLiteral(", ")));
                    }
                    entry.details = parts.join(QStringLiteral(" | "));
                    entry.circuitIndex = circuitIdx;
                    sub.diffs.push_back(entry);
                    sub.summary.netMismatches += 1;
                }
            } else {
                DiffEntry entry;
                entry.type = DiffType::NetMismatch;
                entry.subtype = DiffEntry::Subtype::NoMatchingNet;
                const QString displayName = hasA ? netsA.value(name).rawName
                                                 : netsB.value(name).rawName;
                entry.name = displayName.isEmpty() ? name : displayName;
                entry.layoutCell = sub.layoutCell;
                entry.schematicCell = sub.schematicCell;
                if (hasA) {
                    const auto &netA = netsA.value(name);
                    entry.details =
                        QStringLiteral("No matching net in Schematics circuit "
                                       "for %1 (connected to %2)")
                            .arg(netA.rawName,
                                 netA.connections.join(QStringLiteral(", ")));
                } else {
                    const auto &netB = netsB.value(name);
                    entry.details =
                        QStringLiteral("No matching net in Layout circuit for "
                                       "%1 (connected to %2)")
                            .arg(netB.rawName,
                                 netB.connections.join(QStringLiteral(", ")));
                }
                entry.circuitIndex = circuitIdx;
                sub.diffs.push_back(entry);
                sub.summary.netMismatches += 1;
            }
        }

        const QJsonArray badElementsArr =
            rootObj.value(QStringLiteral("badelements")).toArray();
        auto processElementPair = [&](const QJsonArray &listA,
                                      const QJsonArray &listB) {
            const int maxCount = std::max(static_cast<int>(listA.size()),
                                          static_cast<int>(listB.size()));
            for (int i = 0; i < maxCount; ++i) {
                const QJsonArray elemA =
                    i < listA.size() ? listA.at(i).toArray() : QJsonArray();
                const QJsonArray elemB =
                    i < listB.size() ? listB.at(i).toArray() : QJsonArray();
                const QString instanceNameA =
                    !elemA.isEmpty() ? elemA.at(0).toString() : QString();
                const QString instanceNameB =
                    !elemB.isEmpty() ? elemB.at(0).toString() : QString();
                const bool missingA =
                    instanceNameA.contains(
                        QStringLiteral("(no matching instance)"),
                        Qt::CaseInsensitive) ||
                    elemA.isEmpty();
                const bool missingB =
                    instanceNameB.contains(
                        QStringLiteral("(no matching instance)"),
                        Qt::CaseInsensitive) ||
                    elemB.isEmpty();

                if (missingA != missingB) {
                    DiffEntry entry;
                    entry.type = DiffType::InstanceMismatch;
                    entry.subtype = DiffEntry::Subtype::MissingInstance;
                    entry.name = missingA ? instanceNameB : instanceNameA;
                    entry.layoutCell = sub.layoutCell;
                    entry.schematicCell = sub.schematicCell;
                    entry.details =
                        missingA ? QStringLiteral("The instance is present "
                                                  "only in Schematics circuit")
                                 : QStringLiteral("The instance is present "
                                                  "only in Layout circuit");
                    entry.circuitIndex = circuitIdx;
                    sub.diffs.push_back(entry);
                    sub.summary.deviceMismatches += 1;
                } else if (!(missingA || missingB)) {
                    DiffEntry entryA;
                    entryA.type = DiffType::InstanceMismatch;
                    entryA.subtype = DiffEntry::Subtype::NoMatchingInstance;
                    entryA.name =
                        instanceNameA.split(QStringLiteral(":")).first();
                    entryA.layoutCell = sub.layoutCell;
                    entryA.schematicCell = sub.schematicCell;
                    entryA.details =
                        QStringLiteral("Instance %1 present in Layout circuit "
                                       "has no matching instance")
                            .arg(instanceNameA);
                    entryA.circuitIndex = circuitIdx;
                    sub.diffs.push_back(entryA);
                    sub.summary.deviceMismatches += 1;

                    DiffEntry entryB;
                    entryB.type = DiffType::InstanceMismatch;
                    entryB.subtype = DiffEntry::Subtype::NoMatchingInstance;
                    entryB.name =
                        instanceNameB.split(QStringLiteral(":")).first();
                    ;
                    entryB.layoutCell = sub.layoutCell;
                    entryB.schematicCell = sub.schematicCell;
                    entryB.details =
                        QStringLiteral("Instance %1 present in Schematics "
                                       "circuit has no matching instance")
                            .arg(instanceNameB);
                    entryB.circuitIndex = circuitIdx;
                    sub.diffs.push_back(entryB);
                    sub.summary.deviceMismatches += 1;
                }
            }
        };

        if (!badElementsArr.isEmpty()) {
            if (badElementsArr.size() == 2 && badElementsArr.at(0).isArray() &&
                badElementsArr.at(1).isArray()) {
                processElementPair(badElementsArr.at(0).toArray(),
                                   badElementsArr.at(1).toArray());
            } else if (badElementsArr.size() == 1 &&
                       badElementsArr.first().isArray()) {
                const QJsonArray first = badElementsArr.first().toArray();
                if (first.size() == 2 && first.at(0).isArray() &&
                    first.at(1).isArray()) {
                    processElementPair(first.at(0).toArray(),
                                       first.at(1).toArray());
                }
            } else {
                for (const QJsonValueConstRef &val : badElementsArr) {
                    if (!val.isArray()) {
                        continue;
                    }
                    QJsonArray pair = val.toArray();
                    if (pair.size() == 1 && pair.at(0).isArray()) {
                        pair = pair.at(0).toArray();
                    }
                    if (pair.size() == 2 && pair.at(0).isArray() &&
                        pair.at(1).isArray()) {
                        processElementPair(pair.at(0).toArray(),
                                           pair.at(1).toArray());
                    }
                }
            }
        }

        report.circuits.push_back(sub);
        ++circuitIdx;
    }

    // Build lookup maps
    QHash<QString, Report::Circuit *> layoutMap;
    QHash<QString, Report::Circuit *> schematicMap;
    for (auto &circuit : report.circuits) {
        if (!circuit.layoutCell.isEmpty()) {
            layoutMap.insert(circuit.layoutCell, &circuit);
        }
        if (!circuit.schematicCell.isEmpty()) {
            schematicMap.insert(circuit.schematicCell, &circuit);
        }
    }

    // Build parent-child links based on device list references
    for (auto &parent : report.circuits) {
        auto linkChild = [&](const QString &name) {
            if (name.isEmpty()) {
                return;
            }
            if (!parent.subcircuits.contains(name)) {
                Report::Circuit *child = layoutMap.value(name, nullptr);
                if (child == nullptr) {
                    child = schematicMap.value(name, nullptr);
                }
                if ((child != nullptr) && child != &parent) {
                    child->isTopLevel = false;
                    parent.subcircuits.insert(name, child);
                }
            }
        };

        for (const QString &name : parent.devicesA) {
            linkChild(name);
        }
    }

    // Determine which circuits to keep: those with diffs or with descendant
    // diffs.
    QHash<Report::Circuit *, bool> hasDiffsCache;
    std::function<bool(Report::Circuit *)> hasDiffsRecursive =
        [&](Report::Circuit *cir) -> bool {
        if (cir == nullptr) {
            return false;
        }
        if (hasDiffsCache.contains(cir)) {
            return hasDiffsCache.value(cir);
        }
        bool has = !cir->diffs.isEmpty();
        for (auto *child : cir->subcircuits) {
            if (hasDiffsRecursive(child)) {
                has = true;
            }
        }
        hasDiffsCache.insert(cir, has);
        return has;
    };

    QVector<Report::Circuit> kept;
    kept.reserve(report.circuits.size());
    for (auto &cir : report.circuits) {
        if (hasDiffsRecursive(&cir)) {
            kept.push_back(cir);
        }
    }

    // Rebuild circuits, summaries, and parent-child links for kept circuits.
    report.summary = Summary{};
    report.circuits.clear();
    report.circuits.reserve(kept.size());
    for (auto &cir : kept) {
        cir.subcircuits.clear();
        cir.isTopLevel = true;
        cir.index = report.circuits.size();
        for (auto &entry : cir.diffs) {
            entry.circuitIndex = cir.index;
        }
        report.summary.deviceMismatches += cir.summary.deviceMismatches;
        report.summary.netMismatches += cir.summary.netMismatches;
        report.summary.shorts += cir.summary.shorts;
        report.summary.opens += cir.summary.opens;
        report.summary.totalDevices += cir.summary.totalDevices;
        report.summary.totalNets += cir.summary.totalNets;
        report.summary.layoutCell = cir.layoutCell;
        report.summary.schematicCell = cir.schematicCell;
        report.circuits.push_back(cir);
    }

    layoutMap.clear();
    schematicMap.clear();
    for (auto &circuit : report.circuits) {
        if (!circuit.layoutCell.isEmpty()) {
            layoutMap.insert(circuit.layoutCell, &circuit);
        }
        if (!circuit.schematicCell.isEmpty()) {
            schematicMap.insert(circuit.schematicCell, &circuit);
        }
    }
    for (auto &parent : report.circuits) {
        auto linkChild = [&](const QString &name) {
            if (name.isEmpty()) {
                return;
            }
            if (!parent.subcircuits.contains(name)) {
                Report::Circuit *child = layoutMap.value(name, nullptr);
                if (child == nullptr) {
                    child = schematicMap.value(name, nullptr);
                }
                if ((child != nullptr) && child != &parent) {
                    child->isTopLevel = false;
                    parent.subcircuits.insert(name, child);
                }
            }
        };
        for (const QString &name : parent.devicesA) {
            linkChild(name);
        }
    }

    report.ok = true;
    return report;
}

auto NetgenJsonParser::toTypeString(NetgenJsonParser::DiffType type)
    -> QString {
    switch (type) {
    case NetgenJsonParser::DiffType::NetMismatch:
        return QStringLiteral("net_mismatch");
    case NetgenJsonParser::DiffType::DeviceMismatch:
        return QStringLiteral("device_mismatch");
    case NetgenJsonParser::DiffType::InstanceMismatch:
        return QStringLiteral("instance_mismatch");
    case NetgenJsonParser::DiffType::PropertyMismatch:
        return QStringLiteral("property_mismatch");
    case NetgenJsonParser::DiffType::Unknown:
    default:
        return QStringLiteral("unknown");
    }
}

auto NetgenJsonParser::toSubtypeString(
    NetgenJsonParser::DiffEntry::Subtype subtype) -> QString {
    switch (subtype) {
    case DiffEntry::Subtype::MissingParameter:
        return QStringLiteral("missing parameter");
    case DiffEntry::Subtype::MissingConnection:
        return QStringLiteral("missing connection");
    case DiffEntry::Subtype::UnmatchedConnections:
        return QStringLiteral("unmatched connections");
    case DiffEntry::Subtype::NoMatchingNet:
        return QStringLiteral("no matching net");
    case DiffEntry::Subtype::MissingInstance:
        return QStringLiteral("missing instance");
    case DiffEntry::Subtype::NoMatchingInstance:
        return QStringLiteral("no matching instance");
    case DiffEntry::Subtype::Unknown:
    default:
        return QStringLiteral("unknown");
    }
}
