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

    QJsonObject rootObj;
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        if (arr.isEmpty() || !arr.first().isObject()) {
            report.error = QStringLiteral("Root JSON array is empty or has no object");
            return report;
        }
        rootObj = arr.first().toObject();
    } else {
        report.error = QStringLiteral("Root JSON is not an object");
        return report;
    }

    const QJsonArray netsArr = rootObj.value(QStringLiteral("nets")).toArray();
    if (!netsArr.isEmpty()) {
        report.summary.totalNets = netsArr.first().toInt(0);
        if (netsArr.size() > 1 && netsArr.at(0).toInt() != netsArr.at(1).toInt()) {
            report.summary.netMismatches = 1;
        }
    }

    const QJsonArray devicesArr = rootObj.value(QStringLiteral("devices")).toArray();
    if (!devicesArr.isEmpty() && devicesArr.first().isArray()) {
        int total = 0;
        for (const QJsonValue &devVal : devicesArr.first().toArray()) {
            if (devVal.isArray()) {
                const QJsonArray pair = devVal.toArray();
                if (pair.size() > 1) {
                    total += pair.at(1).toInt(0);
                }
            }
        }
        report.summary.totalDevices = total;
        if (devicesArr.size() > 1) {
            int totalB = 0;
            for (const QJsonValue &devVal : devicesArr.at(1).toArray()) {
                if (devVal.isArray()) {
                    const QJsonArray pair = devVal.toArray();
                    if (pair.size() > 1) {
                        totalB += pair.at(1).toInt(0);
                    }
                }
            }
            if (total != totalB) {
                report.summary.deviceMismatches = 1;
            }
        }
    }

    const QJsonArray namesArr = rootObj.value(QStringLiteral("name")).toArray();
    const QString layoutCell = namesArr.size() > 0 ? namesArr.at(0).toString() : QString();
    const QString schematicCell = namesArr.size() > 1 ? namesArr.at(1).toString() : QString();
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
            DiffEntry entry;
            entry.type = DiffType::PropertyMismatch;
            entry.name = !nameA.isEmpty() ? nameA : nameB;
            entry.layoutCell = layoutCell;
            entry.schematicCell = schematicCell;
            entry.details = QStringLiteral("%1: %2 vs %3").arg(param, valA, valB);
            report.diffs.push_back(entry);
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
