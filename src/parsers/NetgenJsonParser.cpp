#include "parsers/NetgenJsonParser.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <qjsonarray.h>
#include <qjsonvalue.h>

namespace {

NetgenJsonParser::DiffType parseType(const QString &typeStr)
{
    if (typeStr.compare(QStringLiteral("net_mismatch"), Qt::CaseInsensitive) == 0) {
        return NetgenJsonParser::DiffType::NetMismatch;
    }
    if (typeStr.compare(QStringLiteral("device_mismatch"), Qt::CaseInsensitive) == 0) {
        return NetgenJsonParser::DiffType::DeviceMismatch;
    }
    return NetgenJsonParser::DiffType::Unknown;
}

} // namespace

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

    for (const QJsonValue &rootVal : arr) {
        if (!rootVal.isObject()) continue;
        const QJsonObject rootObj = rootVal.toObject();
        Report::Circuit sub;

        const QJsonArray netsArr = rootObj.value(QStringLiteral("nets")).toArray();
        if (!netsArr.isEmpty()) {
            sub.summary.totalNets = netsArr.first().toInt(0);
            if (netsArr.size() > 1 && netsArr.at(0).toInt() != netsArr.at(1).toInt()) {
                sub.summary.netMismatches = 1;
            }
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

        const QJsonArray namesArr = rootObj.value(QStringLiteral("name")).toArray();
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
                entry.name = !nameA.isEmpty() ? nameA : nameB;
                entry.layoutCell = sub.layoutCell;
                entry.schematicCell = sub.schematicCell;
                entry.details = QStringLiteral("%1: %2 vs %3").arg(param, valA, valB);
                sub.diffs.push_back(entry);
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
