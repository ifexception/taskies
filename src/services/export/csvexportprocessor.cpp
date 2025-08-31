// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

#include <algorithm>

#include "../../utils/utils.h"

namespace tks::Services::Export
{
CsvExportProcessor::CsvExportProcessor(const ExportOptions& options,
    const CsvMappedOptions& mappedOptions)
    : mOptions(options)
    , mMappedOptions(mappedOptions)
{
}

void CsvExportProcessor::ProcessData(std::string& value)
{
    TryProcessEmptyValues(value);
    TryProcessNewLines(value);
    TryProcessBooleanHandler(value);
    TryProcessTextQualifier(value);
}

void CsvExportProcessor::TryProcessNewLines(std::string& value) const
{
    if (mOptions.NewLinesHandler == NewLines::Merge) {
        // clang-format off
        value.erase(
            std::remove(
                value.begin(),
                value.end(),
                '\n'),
            value.end());
        // clang-format on
    } else if (mOptions.NewLinesHandler == NewLines::MergeAndAddSpace) {
        value = Utils::ReplaceAll(value, "\n", " ");
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

void CsvExportProcessor::TryProcessBooleanHandler(std::string& value) const
{
    if (!value.empty() && value.size() == 1) {
        if (value == "0" || value == "1" && mOptions.BooleanHandler != BooleanHandler::OneZero) {
            if (mOptions.BooleanHandler == BooleanHandler::TrueFalseLowerCase) {
                if (value == "1") {
                    value = "true";
                } else if (value == "0") {
                    value = "false";
                }
            } else if (mOptions.BooleanHandler == BooleanHandler::YesNoLowerCase) {
                if (value == "1") {
                    value = "yes";
                } else if (value == "0") {
                    value = "no";
                }
            } else if (mOptions.BooleanHandler == BooleanHandler::TrueFalseTitleCase) {
                if (value == "1") {
                    value = "True";
                } else if (value == "0") {
                    value = "False";
                }
            } else if (mOptions.BooleanHandler == BooleanHandler::YesNoTitleCase) {
                if (value == "1") {
                    value = "Yes";
                } else if (value == "0") {
                    value = "No";
                }
            }
        }
    }
}

void CsvExportProcessor::TryProcessTextQualifier(std::string& value) const
{
    std::string quote = "\"";

    if (mOptions.TextQualifier != TextQualifierType::None) {
        value =
            Utils::ReplaceAll(value, quote, MapTextQualifierEnumToValue(mOptions.TextQualifier));

        if (value.find(mMappedOptions.Delimiter) != std::string::npos) {
            value = mMappedOptions.TextQualifier + value + mMappedOptions.TextQualifier;
        }
    }
}
} // namespace tks::Services::Export
