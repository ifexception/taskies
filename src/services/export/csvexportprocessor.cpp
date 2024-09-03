// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Contact:
//     szymonwelgus at gmail dot com

#include "csvexportprocessor.h"

#include "../../utils/utils.h"

namespace tks::Services::Export
{
CsvExportProcessor::CsvExportProcessor(Services::Export::CsvExportOptions options)
    : mOptions(options)
{
}

void CsvExportProcessor::ProcessData(std::stringstream& data, std::string& value)
{
    TryProcessEmptyValues(value);
    TryProcessNewLines(value);
    TryApplyTextQualifier(data, value);
}

void CsvExportProcessor::TryProcessNewLines(std::string& value) const
{
    if (mOptions.NewLinesHandler == NewLines::Merge) {
        std::string newline = "\n";
        std::string space = " ";

        value = Utils::ReplaceAll(value, newline, space);
    }
}

void CsvExportProcessor::TryProcessEmptyValues(std::string& value) const
{
    if (value.empty()) {
        if (mOptions.EmptyValuesHandler == EmptyValues::Null) {
            value = "NULL";
        }
    }
}

void CsvExportProcessor::TryApplyTextQualifier(std::stringstream& data, std::string& value) const
{
    std::string quote = "\"";
    std::string doubleQuote = "\"\"";

    value = Utils::ReplaceAll(value, quote, doubleQuote);

    if (mOptions.TextQualifier != '\0' && value.find(mOptions.Delimiter) != std::string::npos) {
        data << mOptions.TextQualifier << value << mOptions.TextQualifier;
    } else {
        data << value;
    }
}
}
