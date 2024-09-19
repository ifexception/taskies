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

#include "common.h"

namespace tks::Common
{
std::string GetProgramName()
{
    return "Taskies";
}

std::string GetProgramIconBundleName()
{
    return "aaTASKIES_ICO";
}

std::string GetExitIconBundleName()
{
    return "EXIT_ICO";
}

std::string GetAddTaskIconBundleName()
{
    return "ADD_TASK_ICO";
}

std::string GetAboutIconBundleName()
{
    return "ABOUT_ICO";
}

std::string GetPreferencesIconBundleName()
{
    return "PREFERENCES_ICO";
}

std::string GetLicense()
{
    return "Taskies is a time tracking productivity tool.\n"
           "Copyright (C) 2024  Szymon Welgus\n"
           "\n"
           "This program is free software: you can redistribute it and/or modify\n"
           "it under the terms of the GNU General Public License as published by\n"
           "the Free Software Foundation, either version 3 of the License, or\n"
           "(at your option) any later version.\n"
           "\n"
           "This program is distributed in the hope that it will be useful,\n"
           "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
           "GNU General Public License for more details.\n"
           "\n"
           "You should have received a copy of the GNU General Public License\n"
           "along with this program.  If not, see <https://www.gnu.org/licenses/>.";
}
namespace Resources
{
std::string Bell()
{
    return "bell16x16.png";
}

std::string BellNotification()
{
    return "bellnotification16x16.png";
}

std::string Wizard()
{
    return "install-wizard.svg";
}
} // namespace Resources
namespace Static
{
std::vector<std::pair<std::string, char>> DelimiterList()
{
    auto o1 = std::make_pair(",", ',');
    auto o2 = std::make_pair(";", ';');
    auto o3 = std::make_pair("|", '|');
    auto o4 = std::make_pair("(tab)", '\t');
    auto o5 = std::make_pair("(space)", ' ');

    return std::vector<std::pair<std::string, char>>{ o1, o2, o3, o4, o5 };
}

std::vector<std::string> TextQualifierList()
{
    return std::vector<std::string>{ "(none)", "\"", "'" };
}

std::vector<std::string> EmptyValueHandlerList()
{
    return std::vector<std::string>{ "(blank)", "NULL" };
}

std::vector<std::string> NewLinesHandlerList()
{
    return std::vector<std::string>{ "Preserve", "Merge" };
}
} // namespace Static
} // namespace tks::Common
