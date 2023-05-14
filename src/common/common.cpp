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
    return "TASKIES_ICO";
}

std::string GetExitIconBundleName()
{
    return "EXIT_ICO";
}

std::string GetLicense()
{
    return "Copyright(C) 2023 Szymon Welgus\n"
           "This program is free software : you can redistribute it and /\n"
           "or modify it under the terms of the GNU General Public License as published\n"
           "by the Free Software Foundation\n"
           ", either version 3 of the License\n"
           ", or (at your option) any later version.\n"
           "This program is distributed in the hope that it will be useful\n"
           ", but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the\n"
           "GNU General Public License for more details.\n"
           "You should have received a copy of the GNU General Public License\n"
           "along with this program.If not, see <https://www.gnu.org/licenses/>.";
}
} // namespace tks::Common
