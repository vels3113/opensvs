#include "parsers/NetgenJsonParser.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QHash>
#include <QSet>
#include <algorithm>
#include <qjsonarray.h>
#include <qjsonvalue.h>

NetgenJsonParser::Report NetgenJsonParser::parseFile(const QString &path) const
{
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
        report.error = QStringLiteral("JSON parse error: %1").arg(parseError.errorString());
        return report;
    }

    if (!doc.isArray()) {
        report.error = QStringLiteral("Root JSON is not an object");
        return report;
    }

    const QJsonArray arr = doc.array();
    if (arr.isEmpty()) {
        report.error = QStringLiteral("Root JSON array is empty or has no object");
        return report;
    }

    report.circuits.reserve(arr.size());
    int circuitIdx = 0;
    for (const QJsonValue &rootVal : arr) {
        if (!rootVal.isObject()) continue;
        const QJsonObject rootObj = rootVal.toObject();
        const QJsonValue namesVal = rootObj.value(QStringLiteral("name"));
        if (!namesVal.isArray()) continue;
        const QJsonArray namesArr = namesVal.toArray();
        if (namesArr.isEmpty()) continue;
        Report::Circuit sub;
        sub.index = circuitIdx;

        const QJsonArray netsArr = rootObj.value(QStringLiteral("nets")).toArray();
        if (!netsArr.isEmpty()) {
            sub.summary.totalNets = netsArr.first().toInt(0);
        }

        const QJsonArray devicesArr = rootObj.value(QStringLiteral("devices")).toArray();
        if (!devicesArr.isEmpty() && devicesArr.first().isArray()) {
            int total = 0;
            for (const QJsonValue &devVal : devicesArr.first().toArray()) {
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
                for (const QJsonValue &devVal : devicesArr.at(1).toArray()) {
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

        sub.layoutCell = namesArr.size() > 0 ? namesArr.at(0).toString() : QString();
        sub.schematicCell = namesArr.size() > 1 ? namesArr.at(1).toString() : QString();
        const QJsonArray propertiesArr = rootObj.value(QStringLiteral("properties")).toArray();
        for (const QJsonValue &val : propertiesArr) {
            const QJsonArray pairArr = val.toArray();
            if (pairArr.size() < 2) continue;
            const QJsonArray deviceA = pairArr.at(0).toArray();
            const QJsonArray deviceB = pairArr.at(1).toArray();
            if (deviceA.size() < 2 || deviceB.size() < 2) continue;
            const QString nameA = deviceA.at(0).toString();
            const QString nameB = deviceB.at(0).toString();
            const QJsonArray paramsA = deviceA.at(1).toArray();
            const QJsonArray paramsB = deviceB.at(1).toArray();
            const int maxParams = std::max(paramsA.size(), paramsB.size());
            for (int i = 0; i < maxParams; ++i) {
                const QJsonArray pA = i < paramsA.size() ? paramsA.at(i).toArray() : QJsonArray();
                const QJsonArray pB = i < paramsB.size() ? paramsB.at(i).toArray() : QJsonArray();
                const QString param = !pA.isEmpty() ? pA.at(0).toString()
                                     : (!pB.isEmpty() ? pB.at(0).toString() : QStringLiteral("unknown"));
                const QString valA = pA.size() > 1 ? pA.at(1).toString() : QStringLiteral("(missing)");
                const QString valB = pB.size() > 1 ? pB.at(1).toString() : QStringLiteral("(missing)");
                if (valA == valB) continue;
                DiffEntry entry;
                entry.type = DiffType::PropertyMismatch;
                entry.subtype = DiffEntry::Subtype::MissingParameter;
                entry.name = !nameA.isEmpty() ? nameA : nameB;
                entry.layoutCell = sub.layoutCell;
                entry.schematicCell = sub.schematicCell;
                entry.details = QStringLiteral("%1: %2 vs %3").arg(param, valA, valB);
                entry.circuitIndex = circuitIdx;
                sub.diffs.push_back(entry);
            }
        }

        QJsonArray badnetsArr = rootObj.value(QStringLiteral("badnets")).toArray();
        if (badnetsArr.size() == 1 && badnetsArr.at(0).isArray()) {
            badnetsArr = badnetsArr.at(0).toArray();
        }

        struct NetInfo {
            QString rawName;
            QStringList connections;
        };
        QHash<QString, NetInfo> netsA;
        QHash<QString, NetInfo> netsB;
        QStringList nameOrder;

        auto normalizeName = [](const QString &n) {
            const QString lower = n.trimmed().toLower();
            if (lower == QStringLiteral("gnd") || lower == QStringLiteral("0")) {
                return QStringLiteral("0");
            }
            return lower;
        };
        auto connectionList = [](const QJsonArray &arr) {
            QStringList parts;
            for (const QJsonValue &v : arr) {
                const QJsonArray conn = v.toArray();
                if (conn.size() >= 2) {
                    QString dev = conn.at(0).toString();
                    const QString port = conn.at(1).toString();
                    const int count = conn.size() > 2 ? conn.at(2).toInt() : 0;
                    parts << QStringLiteral("%1:%2 (%3)").arg(dev, port).arg(count);
                }
            }
            return parts;
        };
        auto captureNet = [&](const QJsonArray &netArr, QHash<QString, NetInfo> &dest) {
            if (netArr.size() < 2) return;
            NetInfo info;
            info.rawName = netArr.at(0).toString();
            if (info.rawName.contains(QStringLiteral("(no matching net)"), Qt::CaseInsensitive)) {
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

        if (badnetsArr.size() == 2 && badnetsArr.at(0).isArray() && badnetsArr.at(1).isArray()) {
            const QJsonArray netsListA = badnetsArr.at(0).toArray();
            const QJsonArray netsListB = badnetsArr.at(1).toArray();
            for (const QJsonValue &na : netsListA) {
                if (na.isArray()) captureNet(na.toArray(), netsA);
            }
            for (const QJsonValue &nb : netsListB) {
                if (nb.isArray()) captureNet(nb.toArray(), netsB);
            }
        } else if (badnetsArr.size() == 1 && badnetsArr.first().isArray()) {
            QJsonArray pairArr = badnetsArr.first().toArray();
            if (pairArr.size() == 2 && pairArr.at(0).isArray() && pairArr.at(1).isArray()) {
                const QJsonArray netsListA = pairArr.at(0).toArray();
                const QJsonArray netsListB = pairArr.at(1).toArray();
                for (const QJsonValue &na : netsListA) {
                    if (na.isArray()) captureNet(na.toArray(), netsA);
                }
                for (const QJsonValue &nb : netsListB) {
                    if (nb.isArray()) captureNet(nb.toArray(), netsB);
                }
            }
        } else {
            for (const QJsonValue &val : badnetsArr) {
                QJsonArray pairArr = val.toArray();
                if (pairArr.size() == 1 && pairArr.at(0).isArray()) {
                    pairArr = pairArr.at(0).toArray();
                }
                if (pairArr.size() < 2) continue;
                const QJsonArray netsListA = pairArr.at(0).toArray();
                const QJsonArray netsListB = pairArr.at(1).toArray();
                for (const QJsonValue &na : netsListA) {
                    if (na.isArray()) captureNet(na.toArray(), netsA);
                }
                for (const QJsonValue &nb : netsListB) {
                    if (nb.isArray()) captureNet(nb.toArray(), netsB);
                }
            }
        }

        for (const QString &name : nameOrder) {
            const bool hasA = netsA.contains(name);
            const bool hasB = netsB.contains(name);
            if (hasA && hasB) {
                const auto &a = netsA.value(name);
                const auto &b = netsB.value(name);
                const QSet<QString> setA(a.connections.begin(), a.connections.end());
                const QSet<QString> setB(b.connections.begin(), b.connections.end());
                QStringList onlyA;
                for (const auto &conn : a.connections) {
                    if (!setB.contains(conn)) {
                        onlyA.append(conn);
                    }
                }
                QStringList onlyB;
                for (const auto &conn : b.connections) {
                    if (!setA.contains(conn)) {
                        onlyB.append(conn);
                    }
                }
                if (!onlyA.isEmpty() || !onlyB.isEmpty()) {
                    DiffEntry entry;
                    entry.type = DiffType::NetMismatch;
                    entry.subtype = DiffEntry::Subtype::MissingConnection;
                    entry.name = !a.rawName.isEmpty() ? a.rawName : b.rawName;
                    entry.layoutCell = sub.layoutCell;
                    entry.schematicCell = sub.schematicCell;
                    QStringList parts;
                    if (!onlyA.isEmpty()) {
                        parts << QStringLiteral("The following nets are connected only in circuit A: %1")
                                     .arg(onlyA.join(QStringLiteral(", ")));
                    }
                    if (!onlyB.isEmpty()) {
                        parts << QStringLiteral("The following nets are connected only in circuit B: %1")
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
                const QString displayName = hasA ? netsA.value(name).rawName : netsB.value(name).rawName;
                entry.name = displayName.isEmpty() ? name : displayName;
                entry.layoutCell = sub.layoutCell;
                entry.schematicCell = sub.schematicCell;
                if (hasA) {
                    const auto &a = netsA.value(name);
                    entry.details = QStringLiteral("No matching net in circuit B for %1 (connected to %2)")
                            .arg(a.rawName, a.connections.join(QStringLiteral(", ")));
                } else {
                    const auto &b = netsB.value(name);
                    entry.details = QStringLiteral("No matching net in circuit A for %1 (connected to %2)")
                            .arg(b.rawName, b.connections.join(QStringLiteral(", ")));
                }
                entry.circuitIndex = circuitIdx;
                sub.diffs.push_back(entry);
                sub.summary.netMismatches += 1;
            }
        }

        const QJsonArray badElementsArr = rootObj.value(QStringLiteral("badelements")).toArray();
        auto processElementPair = [&](const QJsonArray &listA, const QJsonArray &listB) {
            const int maxCount = std::max(listA.size(), listB.size());
            for (int i = 0; i < maxCount; ++i) {
                const QJsonArray elemA = i < listA.size() ? listA.at(i).toArray() : QJsonArray();
                const QJsonArray elemB = i < listB.size() ? listB.at(i).toArray() : QJsonArray();
                const QString nameA = !elemA.isEmpty() ? elemA.at(0).toString() : QString();
                const QString nameB = !elemB.isEmpty() ? elemB.at(0).toString() : QString();
                const bool missingA = nameA.contains(QStringLiteral("(no matching instance)"), Qt::CaseInsensitive) || elemA.isEmpty();
                const bool missingB = nameB.contains(QStringLiteral("(no matching instance)"), Qt::CaseInsensitive) || elemB.isEmpty();

                if (missingA == missingB) continue;

                DiffEntry entry;
                entry.type = DiffType::DeviceMismatch;
                entry.subtype = DiffEntry::Subtype::MissingInstance;
                entry.name = missingA ? nameB : nameA;
                entry.layoutCell = sub.layoutCell;
                entry.schematicCell = sub.schematicCell;
                entry.details = missingA
                        ? QStringLiteral("The instance is present only in circuit B")
                        : QStringLiteral("The instance is present only in circuit A");
                entry.circuitIndex = circuitIdx;
                sub.diffs.push_back(entry);
                sub.summary.deviceMismatches += 1;
            }
        };

        if (!badElementsArr.isEmpty()) {
            if (badElementsArr.size() == 2 && badElementsArr.at(0).isArray() && badElementsArr.at(1).isArray()) {
                processElementPair(badElementsArr.at(0).toArray(), badElementsArr.at(1).toArray());
            } else if (badElementsArr.size() == 1 && badElementsArr.first().isArray()) {
                const QJsonArray first = badElementsArr.first().toArray();
                if (first.size() == 2 && first.at(0).isArray() && first.at(1).isArray()) {
                    processElementPair(first.at(0).toArray(), first.at(1).toArray());
                }
            } else {
                for (const QJsonValue &val : badElementsArr) {
                    if (!val.isArray()) continue;
                    QJsonArray pair = val.toArray();
                    if (pair.size() == 1 && pair.at(0).isArray()) {
                        pair = pair.at(0).toArray();
                    }
                    if (pair.size() == 2 && pair.at(0).isArray() && pair.at(1).isArray()) {
                        processElementPair(pair.at(0).toArray(), pair.at(1).toArray());
                    }
                }
            }
        }

        report.summary.deviceMismatches += sub.summary.deviceMismatches;
        report.summary.netMismatches += sub.summary.netMismatches;
        report.summary.shorts += sub.summary.shorts;
        report.summary.opens += sub.summary.opens;
        report.summary.totalDevices += sub.summary.totalDevices;
        report.summary.totalNets += sub.summary.totalNets;
        report.summary.layoutCell = sub.layoutCell;
        report.summary.schematicCell = sub.schematicCell;
        report.circuits.push_back(sub);
        ++circuitIdx;
    }

    // Build lookup maps
    QHash<QString, Report::Circuit*> layoutMap;
    QHash<QString, Report::Circuit*> schematicMap;
    for (auto &circuit : report.circuits) {
        if (!circuit.layoutCell.isEmpty()) layoutMap.insert(circuit.layoutCell, &circuit);
        if (!circuit.schematicCell.isEmpty()) schematicMap.insert(circuit.schematicCell, &circuit);
    }

    // Build parent-child links based on device list references
    for (auto &parent : report.circuits) {
        auto linkChild = [&](const QString &name) {
            if (name.isEmpty()) return;
            if (!parent.subcircuits.contains(name)) {
                Report::Circuit *child = layoutMap.value(name, nullptr);
                if (!child) child = schematicMap.value(name, nullptr);
                if (child && child != &parent) {
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

QString NetgenJsonParser::toTypeString(NetgenJsonParser::DiffType type)
{
    switch (type) {
    case NetgenJsonParser::DiffType::NetMismatch:
        return QStringLiteral("net_mismatch");
    case NetgenJsonParser::DiffType::DeviceMismatch:
        return QStringLiteral("device_mismatch");
    case NetgenJsonParser::DiffType::PropertyMismatch:
        return QStringLiteral("property_mismatch");
    case NetgenJsonParser::DiffType::Unknown:
    default:
        return QStringLiteral("unknown");
    }
}

QString NetgenJsonParser::toSubtypeString(NetgenJsonParser::DiffEntry::Subtype subtype)
{
    switch (subtype) {
    case DiffEntry::Subtype::MissingParameter:
        return QStringLiteral("missing parameter");
    case DiffEntry::Subtype::MissingConnection:
        return QStringLiteral("missing connection");
    case DiffEntry::Subtype::NoMatchingNet:
        return QStringLiteral("no matching net");
    case DiffEntry::Subtype::MissingInstance:
        return QStringLiteral("missing instance");
    case DiffEntry::Subtype::Unknown:
    default:
        return QStringLiteral("unknown");
    }
}
