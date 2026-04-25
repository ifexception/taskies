// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#pragma once

#include <string>

namespace tks::Messages
{
const std::string ConfigurationFileNotFoundHeaderMessage =
    "Your configuration file could not be found";
const std::string ConfigurationFileNotExistUserMessage =
    "Configuration failed to load. Please check that the configuration file (\"{0}\") is present";
const std::string ConfigurationFileNotExistErrorMessage =
    "Configuration file does not exist at location \"{0}\"";

const std::string ConfigurationFileOpenHeaderMessage =
    "Your configuration file could not be opened";
const std::string ConfigurationFileOpenUserMessage =
    "The configuration file is missing or contains an error. To fix this, try restoring your last "
    "saved settings or resetting to defaults";
const std::string ConfigurationFileOpenErrorMessage =
    "Configuration file error encountered: \"{0}\"";

const std::string ConfigurationFileParseHeaderMessage = "Your configuration file could not be read";
const std::string CongfigurationFileParseUserMessage =
    "The configuration file contains an error and cannot be parsed";
const std::string CongfigurationFileParseErrorMessage = "Unable to parse \"{0}\". Error {1}";
} // namespace tks::Messages
