#include "o_promptRad_UiHelpers.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QObject>
#include <QStringList>
#include <QTextEdit>
#include <QTextStream>

#include <algorithm>

namespace {

QString sourceDirectoryPath()
{
#ifdef PROMPT_RADIATION_SOURCE_DIR
    return QString::fromLocal8Bit(PROMPT_RADIATION_SOURCE_DIR);
#else
    return QString();
#endif
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

QStringList splitInputLine(const QString& line)
{
    QStringList parts = line.split(QLatin1Char('\t'), Qt::KeepEmptyParts);
    if (parts.size() < 6) {
        parts = line.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }

    for (QString& part : parts) {
        part = part.trimmed();
    }

    return parts;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

bool textToInt(const QString& text, int& value)
{
    bool ok = false;
    value = text.trimmed().toInt(&ok);
    return ok;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

bool textToDouble(const QString& text, double& value)
{
    bool ok = false;
    value = text.trimmed().toDouble(&ok);
    return ok;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

} // namespace

namespace prompt_rad_ui {

QString locationFactorKey(int monitorIndex, int locationIndex)
{
    QString location = QString::fromLatin1(lise_prompt_rad::kLocationNames[locationIndex]);
    location.replace(QLatin1Char('/'), QString()); // P2/P3 -> P2P3, easier to edit in INI

    return QString::fromLatin1("%1_%2")
        .arg(QString::fromLatin1(lise_prompt_rad::kMonitorNames[monitorIndex]), location);
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

QString legacyLocationFactorKey(int monitorIndex, int locationIndex)
{
    // Older test version used '/' in the key. Keep this reader so existing INI files still work.
    return QString::fromLatin1("%1/%2")
        .arg(QString::fromLatin1(lise_prompt_rad::kMonitorNames[monitorIndex]),
             QString::fromLatin1(lise_prompt_rad::kLocationNames[locationIndex]));
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

QString findDefaultInputFile(const QString& fileName)
{
    QStringList candidates;
    candidates << QDir(QCoreApplication::applicationDirPath()).filePath(fileName);
    candidates << QDir::current().filePath(fileName);

    const QString sourceDir = sourceDirectoryPath();
    if (!sourceDir.isEmpty()) {
        candidates << QDir(sourceDir).filePath(fileName);
    }

    for (const QString& candidate : candidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile() && info.isReadable()) {
            return info.absoluteFilePath();
        }
    }

    return QFileInfo(candidates.first()).absoluteFilePath();
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

QString signedNumber(double value)
{
    QString s = QString::number(value, 'g', 12);
    if (value > 0.0) {
        s.prepend(QLatin1Char('+'));
    }
    return s;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

std::vector<lise_prompt_rad::LiseDetailedInput> readYieldInputFile(const QString& filePath,
                                                                   QStringList& messages)
{
    std::vector<lise_prompt_rad::LiseDetailedInput> rows;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        messages << QObject::tr("Cannot open yield input file:\n  %1").arg(filePath);
        return rows;
    }

    QTextStream in(&file);
    int skipped = 0;

    while (!in.atEnd()) {
        const QString line = in.readLine();

        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QStringList parts = splitInputLine(line);
        if (parts.size() < 6) {
            ++skipped;
            continue;
        }

        int A = 0;
        int Z = 0;
        double ratePps = 0.0;
        double energyMeVu = 0.0;

        if (!textToInt(parts[1], A) ||
            !textToInt(parts[2], Z) ||
            !textToDouble(parts[4], ratePps) ||
            !textToDouble(parts[5], energyMeVu)) {
            ++skipped;
            continue;
        }

        lise_prompt_rad::LiseDetailedInput row;
        row.fragment = parts[0].toStdString();
        row.A = A;
        row.Z = Z;
        row.mechanism = parts[3].toStdString();
        row.ratePps = ratePps;
        row.energyMeVu = energyMeVu;
        rows.push_back(row);
    }

    messages << QObject::tr("Yield rows accepted/skipped: %1/%2")
                .arg(static_cast<int>(rows.size()))
                .arg(skipped);
    return rows;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

std::vector<lise_prompt_rad::TemplatePosition> readPositionInputFile(const QString& filePath,
                                                                     QStringList& messages)
{
    std::vector<lise_prompt_rad::TemplatePosition> rows;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        messages << QObject::tr("Cannot open position input file:\n  %1").arg(filePath);
        return rows;
    }

    QTextStream in(&file);
    int skipped = 0;

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QStringList parts = splitInputLine(line);
        if (parts.size() < 2) {
            ++skipped;
            continue;
        }

        double db0xMm = 0.0;
        if (!textToDouble(parts[1], db0xMm)) {
            ++skipped;
            continue;
        }

        lise_prompt_rad::TemplatePosition row;
        row.fragment = parts[0].toStdString();
        row.db0xMm = db0xMm;
        rows.push_back(row);
    }

    messages << QObject::tr("Position rows accepted/skipped: %1/%2")
                .arg(static_cast<int>(rows.size()))
                .arg(skipped);
    return rows;
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void appendMonitorTotals(QTextEdit* log, const lise_prompt_rad::DetailedDoseResult& result)
{
    for (int i = 0; i < lise_prompt_rad::kMonitorCount; ++i) {
        log->append(QString::fromLatin1("  %1 : %2 mrem/h")
                    .arg(QString::fromLatin1(lise_prompt_rad::kMonitorNames[i]), -7)
                    .arg(result.totalsMremPerHr[i], 0, 'g', 8));
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

void appendTopContributors(QTextEdit* log, const lise_prompt_rad::DetailedDoseResult& result)
{
    struct Contributor {
        QString name;
        QString location;
        double ratePps = 0.0;
        double db0xMm = 0.0;
        bool hasDb0x = false;
        double n0318 = 0.0;
    };

    std::vector<Contributor> contributors;
    contributors.reserve(result.rows.size() + result.lowIonRows.size());

    for (const auto& row : result.rows) {
        Contributor item;
        item.name = QString::fromStdString(row.input.fragment);
        item.location = QString::fromStdString(lise_prompt_rad::stopLocationToString(row.location));
        item.ratePps = row.input.ratePps;
        item.db0xMm = row.db0xMm;
        item.hasDb0x = row.hasDb0x;
        item.n0318 = row.doseMremPerHr[0];
        contributors.push_back(item);
    }

    for (const auto& row : result.lowIonRows) {
        Contributor item;
        item.name = QString::fromStdString(row.input.fragment);
        item.location = QString::fromStdString(lise_prompt_rad::stopLocationToString(row.location));
        item.ratePps = row.input.ratePps;
        item.n0318 = row.doseMremPerHr[0];
        contributors.push_back(item);
    }

    std::sort(contributors.begin(), contributors.end(), [](const Contributor& a, const Contributor& b) {
        return a.n0318 > b.n0318;
    });

    log->append(QObject::tr("\nTop N0318 contributors:"));
    const int maxRows = std::min<int>(10, static_cast<int>(contributors.size()));
    for (int i = 0; i < maxRows; ++i) {
        const Contributor& item = contributors[static_cast<std::size_t>(i)];
        const QString db0Text = item.hasDb0x
            ? QString::number(item.db0xMm, 'g', 8)
            : QStringLiteral("--");

        log->append(QString::fromLatin1("  %1  %2  loc=%3  DB0.x=%4  rate=%5  N0318=%6")
                    .arg(i + 1, 2)
                    .arg(item.name, -8)
                    .arg(item.location, -5)
                    .arg(db0Text, 10)
                    .arg(item.ratePps, 0, 'g', 8)
                    .arg(item.n0318, 0, 'g', 8));
    }
}
//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

} // namespace prompt_rad_ui
