// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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
    return "aTASKIES_ICO";
}

std::string GetExitIconBundleName()
{
    return "EXIT_ICO";
}

std::string GetAddTaskIconBundleName()
{
    return "ADD_TASK_ICO";
}

std::string GetLicense()
{
    return "Taskies is a time tracking productivity tool.\n"
           "Copyright (C) 2023  Szymon Welgus\n"
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
} // namespace Resources
} // namespace tks::Common
