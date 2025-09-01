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

std::string GetQuickExportIconBundleName()
{
    return "QUICK_EXPORT_ICO";
}

std::string GetLicense()
{
    return "Taskies is a time tracking productivity tool.\n"
           "Copyright (C) 2025  Szymon Welgus\n"
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
std::vector<EnumClientData<DelimiterType>> DelimitersList()
{
    EnumClientData<DelimiterType> o1(",", DelimiterType::Comma);
    EnumClientData<DelimiterType> o2(";", DelimiterType::Semicolon);
    EnumClientData<DelimiterType> o3("|", DelimiterType::Pipe);
    EnumClientData<DelimiterType> o4("(tab)", DelimiterType::Tab);
    EnumClientData<DelimiterType> o5("(space)", DelimiterType::Space);

    return std::vector<EnumClientData<DelimiterType>>{ o1, o2, o3, o4, o5 };
}

std::vector<EnumClientData<TextQualifierType>> TextQualifiersList()
{
    EnumClientData<TextQualifierType> o1("(none)", TextQualifierType::None);
    EnumClientData<TextQualifierType> o2("\"", TextQualifierType::DoubleQuotes);
    EnumClientData<TextQualifierType> o3("'", TextQualifierType::SingleQuotes);

    return std::vector<EnumClientData<TextQualifierType>>{ o1, o2, o3 };
}

std::vector<EnumClientData<EmptyValues>> EmptyValuesList()
{
    EnumClientData<EmptyValues> o1("(blank)", EmptyValues::Blank);
    EnumClientData<EmptyValues> o2("NULL", EmptyValues::Null);

    return std::vector<EnumClientData<EmptyValues>>{ o1, o2 };
}

std::vector<EnumClientData<NewLines>> NewLinesList()
{
    EnumClientData<NewLines> o1("Preserve", NewLines::Preserve);
    EnumClientData<NewLines> o2("Merge", NewLines::Merge);
    EnumClientData<NewLines> o3("Merge (add space)", NewLines::MergeAndAddSpace);

    return std::vector<EnumClientData<NewLines>>{ o1, o2, o3 };
}

std::vector<EnumClientData<BooleanHandler>> BooleanHandlerList()
{
    EnumClientData<BooleanHandler> o1("1|0", BooleanHandler::OneZero);
    EnumClientData<BooleanHandler> o2("true|false", BooleanHandler::TrueFalseLowerCase);
    EnumClientData<BooleanHandler> o3("yes|no", BooleanHandler::YesNoLowerCase);
    EnumClientData<BooleanHandler> o4("True|False", BooleanHandler::TrueFalseTitleCase);
    EnumClientData<BooleanHandler> o5("Yes|No", BooleanHandler::YesNoTitleCase);

    return std::vector<EnumClientData<BooleanHandler>>{ o1, o2, o3, o4, o5 };
}
} // namespace Static
} // namespace tks::Common
