#include "parsers/NetgenJsonParser.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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

    if (!doc.isObject()) {
        report.error = QStringLiteral("Root JSON is not an object");
        return report;
    }

    const QJsonObject rootObj = doc.object();
    const QJsonObject summaryObj = rootObj.value(QStringLiteral("summary")).toObject();
    report.summary.deviceMismatches = summaryObj.value(QStringLiteral("device_mismatches")).toInt(0);
    report.summary.netMismatches = summaryObj.value(QStringLiteral("net_mismatches")).toInt(0);
    report.summary.shorts = summaryObj.value(QStringLiteral("shorts")).toInt(0);
    report.summary.opens = summaryObj.value(QStringLiteral("opens")).toInt(0);
    report.summary.totalDevices = summaryObj.value(QStringLiteral("total_devices")).toInt(0);
    report.summary.totalNets = summaryObj.value(QStringLiteral("total_nets")).toInt(0);

    const QJsonArray diffsArr = rootObj.value(QStringLiteral("diffs")).toArray();
    for (const QJsonValue &val : diffsArr) {
        const QJsonObject diffObj = val.toObject();
        DiffEntry entry;
        entry.type = parseType(diffObj.value(QStringLiteral("type")).toString());
        entry.name = diffObj.value(QStringLiteral("name")).toString();
        entry.layoutCell = diffObj.value(QStringLiteral("layout_cell")).toString();
        entry.schematicCell = diffObj.value(QStringLiteral("schematic_cell")).toString();
        entry.details = diffObj.value(QStringLiteral("details")).toString();
        report.diffs.push_back(entry);
    }

    report.ok = true;
    return report;
}
