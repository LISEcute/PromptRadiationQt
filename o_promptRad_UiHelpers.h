#pragma once

#include "o_promptRad_Detailed.h"

#include <QString>
#include <QStringList>

#include <vector>

class QTextEdit;

namespace prompt_rad_ui {

QString locationFactorKey(int monitorIndex, int locationIndex);
QString legacyLocationFactorKey(int monitorIndex, int locationIndex);
QString findDefaultInputFile(const QString& fileName);
QString signedNumber(double value);

std::vector<lise_prompt_rad::LiseDetailedInput> readYieldInputFile(const QString& filePath,
                                                                   QStringList& messages);
std::vector<lise_prompt_rad::TemplatePosition> readPositionInputFile(const QString& filePath,
                                                                     QStringList& messages);

void appendMonitorTotals(QTextEdit* log, const lise_prompt_rad::DetailedDoseResult& result);
void appendTopContributors(QTextEdit* log, const lise_prompt_rad::DetailedDoseResult& result);

} // namespace prompt_rad_ui
