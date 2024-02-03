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

#include "applicationoptionsconnection.h"

#include <wx/taskbarbutton.h>

#include "../ui/mainframe.h"

namespace tks
{
namespace IPC
{
ApplicationOptionsConnection::ApplicationOptionsConnection(UI::MainFrame* frame)
    : pFrame(frame)
{
}

bool ApplicationOptionsConnection::OnExecute(const wxString& topic, const void* data, size_t size, wxIPCFormat format)
{
    std::string textData = GetTextFromData(data, size, format).ToStdString();
    pFrame->GetEventHandler()->CallAfter([this, textData]() {
        pFrame->MSWGetTaskBarButton()->Show();

        if (pFrame->IsIconized()) {
            pFrame->Restore();
        }

        pFrame->Raise();
        pFrame->Show();
    });

    return true;
}
} // namespace IPC
} // namespace tks
