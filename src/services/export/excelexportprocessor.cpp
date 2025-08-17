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

#include "excelexportprocessor.h"

#include "../../utils/utils.h"

namespace tks::Services::Export
{
ExcelExportProcessor::ExcelExportProcessor(NewLines newLinesOption,
    BooleanHandler booleanHandlerOption)
    : mNewLinesOption(newLinesOption)
    , mBooleanHandlerOption(booleanHandlerOption)
{
}

void ExcelExportProcessor::TryApplyOptionsAndProcessExportData(std::string& value)
{
    TryProcessNewLines(value);
    TryProcessBooleanHandler(value);
}

void ExcelExportProcessor::TryProcessNewLines(std::string& value) const
{
    if (mNewLinesOption == NewLines::Merge) {
        // clang-format off
        value.erase(
            std::remove(
                value.begin(),
                value.end(),
                '\n'),
            value.end());
        // clang-format on
    } else if (mNewLinesOption == NewLines::MergeAndAddSpace) {
        value = Utils::ReplaceAll(value, "\n", " ");
    }
}

void ExcelExportProcessor::TryProcessBooleanHandler(std::string& value) const
{
    if (!value.empty() && value.size() == 1) {
        if (value == "0" || value == "1" && mBooleanHandlerOption != BooleanHandler::OneZero) {
            if (mBooleanHandlerOption == BooleanHandler::TrueFalseLowerCase) {
                if (value == "1") {
                    value = "true";
                } else if (value == "0") {
                    value = "false";
                }
            } else if (mBooleanHandlerOption == BooleanHandler::YesNoLowerCase) {
                if (value == "1") {
                    value = "yes";
                } else if (value == "0") {
                    value = "no";
                }
            } else if (mBooleanHandlerOption == BooleanHandler::TrueFalseTitleCase) {
                if (value == "1") {
                    value = "True";
                } else if (value == "0") {
                    value = "False";
                }
            } else if (mBooleanHandlerOption == BooleanHandler::YesNoTitleCase) {
                if (value == "1") {
                    value = "Yes";
                } else if (value == "0") {
                    value = "No";
                }
            }
        }
    }
}
} // namespace tks::Services::Export
