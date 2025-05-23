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

#include "editlistdlg.h"

#include <vector>

#include <wx/artprov.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "employerdlg.h"
#include "clientdlg.h"
#include "projectdlg.h"
#include "categorydlg.h"

#include "attributes/attributegroupdlg.h"
#include "attributes/attributedlg.h"
#include "attributes/staticattributevaluesdlg.h"

#include "../events.h"
#include "../notificationclientdata.h"

#include "../../common/common.h"

#include "../../persistence/employerspersistence.h"
#include "../../persistence/ClientsPersistence.h"
#include "../../persistence/projectspersistence.h"
#include "../../persistence/categoriespersistence.h"
#include "../../persistence/attributegroupspersistence.h"
#include "../../persistence/attributespersistence.h"
#include "../../persistence/staticattributevaluespersistence.h"

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"
#include "../../models/projectmodel.h"
#include "../../models/categorymodel.h"
#include "../../models/attributegroupmodel.h"
#include "../../models/attributemodel.h"
#include "../../models/staticattributevaluemodel.h"

#include "../../utils/utils.h"

namespace tks::UI::dlg
{
ListCtrlData::ListCtrlData(std::int64_t entityId, std::string entityName)
    : EntityId(entityId)
    , EntityName(entityName)
{
}

EditListDialog::EditListDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    EditListEntityType editListEntityType,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          wxEmptyString,
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , mType(editListEntityType)
    , pSearchTextCtrl(nullptr)
    , pSearchButton(nullptr)
    , pResetButton(nullptr)
    , pListCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , mSearchTerm()
    , mEntityId(-1)
{
    SetTitle(GetEditTitle());
    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

std::string EditListDialog::GetEditTitle()
{
    switch (mType) {
    case EditListEntityType::Employers:
        return "Find Employers";
    case EditListEntityType::Clients:
        return "Find Clients";
    case EditListEntityType::Projects:
        return "Find Projects";
    case EditListEntityType::Categories:
        return "Find Categories";
    case EditListEntityType::AttributeGroups:
        return "Find Attribute Groups";
    case EditListEntityType::Attributes:
        return "Find Attributes";
    case EditListEntityType::StaticAttributeValues:
        return "Find Static Attribute Groups and Values";
    default:
        return "Find [Not Found]";
    }
}

void EditListDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    DataToControls();
}

void EditListDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Search */
    auto searchBox = new wxStaticBox(this, wxID_ANY, "Search");
    auto searchBoxSizer = new wxStaticBoxSizer(searchBox, wxHORIZONTAL);
    sizer->Add(searchBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Search Text Control */
    pSearchTextCtrl = new wxTextCtrl(searchBox,
        tksIDC_SEARCHTEXTCTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_LEFT /* | wxTE_PROCESS_ENTER*/);
    pSearchTextCtrl->SetHint(GetSearchHintText());
    pSearchTextCtrl->SetToolTip("Enter a search term");
    searchBoxSizer->Add(
        pSearchTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Search Button */
    auto providedFindBitmap = wxArtProvider::GetBitmapBundle(
        wxART_FIND, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pSearchButton = new wxBitmapButton(searchBox, tksIDC_SEARCHBUTTON, providedFindBitmap);
    pSearchButton->SetToolTip("Search for an entity by entered criteria");
    searchBoxSizer->Add(pSearchButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Reset Button */
    auto providedCloseBitmap = wxArtProvider::GetBitmapBundle(
        wxART_CLOSE, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pResetButton = new wxBitmapButton(searchBox, tksIDC_RESETBUTTON, providedCloseBitmap);
    pResetButton->SetToolTip("Reset search term");
    searchBoxSizer->Add(pResetButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* List Control */
    pListCtrl = new wxListCtrl(this,
        tksIDC_LISTRESULTS,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_HRULES | wxLC_REPORT | wxLC_SINGLE_SEL);
    sizer->Add(pListCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    wxListItem nameColumn;
    nameColumn.SetId(0);
    nameColumn.SetText("Name");
    nameColumn.SetWidth(wxLIST_AUTOSIZE_USEHEADER);
    pListCtrl->InsertColumn(0, nameColumn);

    /* Horizontal Line */
    auto line2 = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line2, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();
    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    pOkButton->Disable();

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(sizer);
}

// clang-format off
void EditListDialog::ConfigureEventBindings()
{
    pSearchTextCtrl->Bind(
        wxEVT_TEXT,
        &EditListDialog::OnSearchTextChange,
        this
    );

    pSearchButton->Bind(
        wxEVT_BUTTON,
        &EditListDialog::OnSearch,
        this,
        tksIDC_SEARCHBUTTON
    );

    pResetButton->Bind(
        wxEVT_BUTTON,
        &EditListDialog::OnReset,
        this,
        tksIDC_RESETBUTTON
    );

    pListCtrl->Bind(
        wxEVT_LIST_ITEM_ACTIVATED,
        &EditListDialog::OnItemDoubleClick,
        this,
        tksIDC_LISTRESULTS
    );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &EditListDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &EditListDialog::OnCancel,
        this,
        wxID_CANCEL
    );
}
// clang-format on

void EditListDialog::DataToControls()
{
    switch (mType) {
    case EditListEntityType::Employers:
        EmployerDataToControls();
        break;
    case EditListEntityType::Clients:
        ClientDataToControls();
        break;
    case EditListEntityType::Projects:
        ProjectDataToControls();
        break;
    case EditListEntityType::Categories:
        CategoryDataToControls();
        break;
    case EditListEntityType::AttributeGroups:
        AttributeGroupDataToControls();
        break;
    case EditListEntityType::Attributes:
        AttributeDataToControls();
        break;
    default:
        break;
    }

    pOkButton->Enable();
}

void EditListDialog::EmployerDataToControls()
{
    std::vector<Model::EmployerModel> employers;
    std::vector<ListCtrlData> entries;
    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    int rc = employerPersistence.Filter(mSearchTerm, employers);
    if (rc == -1) {
        std::string message = "Failed to filter employers";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (auto& employer : employers) {
            ListCtrlData data(employer.EmployerId, employer.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }
}

void EditListDialog::ClientDataToControls()
{
    std::vector<Model::ClientModel> clients;
    std::vector<ListCtrlData> entries;
    Persistence::ClientsPersistence ClientsPersistence(pLogger, mDatabaseFilePath);

    int rc = ClientsPersistence.Filter(mSearchTerm, clients);
    if (rc == -1) {
        std::string message = "Failed to filter clients";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (auto& client : clients) {
            ListCtrlData data(client.ClientId, client.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }
}

void EditListDialog::ProjectDataToControls()
{
    std::vector<Model::ProjectModel> projects;
    std::vector<ListCtrlData> entries;
    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    int rc = projectPersistence.Filter(mSearchTerm, projects);
    if (rc == -1) {
        std::string message = "Failed to filter projects";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (const auto& project : projects) {
            ListCtrlData data(project.ProjectId, project.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }
}

void EditListDialog::CategoryDataToControls()
{
    std::vector<Model::CategoryModel> categories;
    std::vector<ListCtrlData> entries;
    Persistence::CategoriesPersistence categoryPersistence(pLogger, mDatabaseFilePath);

    int rc = categoryPersistence.Filter(mSearchTerm, categories);
    if (rc == -1) {
        std::string message = "Failed to filter categories";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (const auto& category : categories) {
            ListCtrlData data(category.CategoryId, category.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }
}

void EditListDialog::AttributeGroupDataToControls()
{
    std::vector<Model::AttributeGroupModel> attributeGroups;
    std::vector<ListCtrlData> entries;

    Persistence::AttributeGroupsPersistence attributeGroupPersistence(pLogger, mDatabaseFilePath);

    int rc = attributeGroupPersistence.Filter(mSearchTerm, attributeGroups);
    if (rc == -1) {
        std::string message = "Failed to filter attribute groups";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (auto& attributeGroup : attributeGroups) {
            ListCtrlData data(attributeGroup.AttributeGroupId, attributeGroup.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }
}

void EditListDialog::AttributeDataToControls()
{
    std::vector<Model::AttributeModel> attributes;
    std::vector<ListCtrlData> entries;

    Persistence::AttributesPersistence attributesPersistence(pLogger, mDatabaseFilePath);

    int rc = attributesPersistence.Filter(mSearchTerm, attributes);
    if (rc == -1) {
        std::string message = "Failed to filter attributes";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (auto& attribute : attributes) {
            ListCtrlData data(attribute.AttributeId, attribute.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }
}

void EditListDialog::StaticAttributeValueDataToControls() {}

void EditListDialog::SetDataToControls(const std::vector<ListCtrlData>& entries)
{
    int listIndex = 0;
    int columnIndex = 0;
    for (auto& entry : entries) {
        listIndex = pListCtrl->InsertItem(columnIndex++, entry.EntityName);
        pListCtrl->SetItemPtrData(listIndex, static_cast<wxUIntPtr>(entry.EntityId));
        columnIndex++;
    }
}

void EditListDialog::OnSearchTextChange(wxCommandEvent& event)
{
    std::string value = pSearchTextCtrl->GetValue().ToStdString();
    mSearchTerm = Utils::TrimWhitespace(value);
}

void EditListDialog::OnSearch(wxCommandEvent& event)
{
    if (mSearchTerm.length() < 3) {
        wxRichToolTip toolTip("", "Please enter 3 or more characters to search");
        toolTip.ShowFor(pSearchTextCtrl);
    } else {
        Search();
    }
}

void EditListDialog::OnReset(wxCommandEvent& event)
{
    mSearchTerm = "";
    pSearchTextCtrl->ChangeValue(wxEmptyString);
    Search();
}

void EditListDialog::OnItemDoubleClick(wxListEvent& event)
{
    mEntityId = static_cast<std::int64_t>(event.GetData());
    switch (mType) {
    case EditListEntityType::Employers: {
        EmployerDialog employerDlg(this, pLogger, mDatabaseFilePath, true, mEntityId);
        employerDlg.ShowModal();
        break;
    }
    case EditListEntityType::Clients: {
        ClientDialog clientDlg(this, pLogger, mDatabaseFilePath, true, mEntityId);
        clientDlg.ShowModal();
        break;
    }
    case EditListEntityType::Projects: {
        ProjectDialog projectDlg(this, pLogger, mDatabaseFilePath, true, mEntityId);
        projectDlg.ShowModal();
        break;
    }
    case EditListEntityType::Categories: {
        CategoryDialog categoryDlg(this, pLogger, mDatabaseFilePath, mEntityId);
        categoryDlg.ShowModal();
        break;
    }
    case EditListEntityType::AttributeGroups: {
        AttributeGroupDialog attributeGroupDlg(this, pLogger, mDatabaseFilePath, true, mEntityId);
        attributeGroupDlg.ShowModal();
        break;
    }
    case EditListEntityType::Attributes: {
        AttributeDialog attributeDlg(this, pLogger, mDatabaseFilePath, true, mEntityId);
        attributeDlg.ShowModal();
        break;
    }
    default:
        break;
    }

    mEntityId = -1;
    mSearchTerm = "";
    pSearchTextCtrl->ChangeValue(wxEmptyString);
    pListCtrl->DeleteAllItems();
    Search();
}

void EditListDialog::OnOK(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

void EditListDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void EditListDialog::Search()
{
    switch (mType) {
    case EditListEntityType::Employers:
        SearchEmployers();
        break;
    case EditListEntityType::Clients:
        SearchClients();
        break;
    case EditListEntityType::Projects:
        SearchProjects();
        break;
    case EditListEntityType::Categories:
        SearchCategories();
        break;
    case EditListEntityType::AttributeGroups:
        SearchAttributeGroups();
        break;
    case EditListEntityType::Attributes:
        SearchAttributes();
        break;
    default:
        break;
    }
}

void EditListDialog::SearchEmployers()
{
    pOkButton->Disable();
    pListCtrl->DeleteAllItems();

    std::vector<Model::EmployerModel> employers;
    std::vector<ListCtrlData> entries;
    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    int rc = employerPersistence.Filter(mSearchTerm, employers);
    if (rc == -1) {
        std::string message = "Failed to filter employers";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (auto& employer : employers) {
            ListCtrlData data(employer.EmployerId, employer.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }

    pOkButton->Enable();
}

void EditListDialog::SearchClients()
{
    pOkButton->Disable();
    pListCtrl->DeleteAllItems();

    std::vector<Model::ClientModel> clients;
    std::vector<ListCtrlData> entries;
    Persistence::ClientsPersistence ClientsPersistence(pLogger, mDatabaseFilePath);

    int rc = ClientsPersistence.Filter(mSearchTerm, clients);
    if (rc == -1) {
        std::string message = "Failed to filter clients";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (auto& client : clients) {
            ListCtrlData data(client.ClientId, client.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }

    pOkButton->Enable();
}

void EditListDialog::SearchProjects()
{
    pOkButton->Disable();
    pListCtrl->DeleteAllItems();

    std::vector<Model::ProjectModel> projects;
    std::vector<ListCtrlData> entries;
    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    int rc = projectPersistence.Filter(mSearchTerm, projects);
    if (rc == -1) {
        std::string message = "Failed to filter projects";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (const auto& project : projects) {
            ListCtrlData data(project.ProjectId, project.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }

    pOkButton->Enable();
}

void EditListDialog::SearchCategories()
{
    pOkButton->Disable();
    pListCtrl->DeleteAllItems();

    std::vector<Model::CategoryModel> categories;
    std::vector<ListCtrlData> entries;
    Persistence::CategoriesPersistence categoryPersistence(pLogger, mDatabaseFilePath);

    int rc = categoryPersistence.Filter(mSearchTerm, categories);
    if (rc == -1) {
        std::string message = "Failed to filter categories";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (const auto& category : categories) {
            ListCtrlData data(category.CategoryId, category.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }

    pOkButton->Enable();
}

void EditListDialog::SearchAttributeGroups()
{
    pOkButton->Disable();
    pListCtrl->DeleteAllItems();

    std::vector<Model::AttributeGroupModel> attributeGroups;
    std::vector<ListCtrlData> entries;
    Persistence::AttributeGroupsPersistence attributeGroupsPersistence(pLogger, mDatabaseFilePath);

    int rc = attributeGroupsPersistence.Filter(mSearchTerm, attributeGroups);
    if (rc == -1) {
        std::string message = "Failed to filter attribute groups";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (auto& attributeGroup : attributeGroups) {
            ListCtrlData data(attributeGroup.AttributeGroupId, attributeGroup.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }

    pOkButton->Enable();
}

void EditListDialog::SearchAttributes()
{
    pOkButton->Disable();
    pListCtrl->DeleteAllItems();

    std::vector<Model::AttributeModel> attributes;
    std::vector<ListCtrlData> entries;
    Persistence::AttributesPersistence attributesPersistence(pLogger, mDatabaseFilePath);

    int rc = attributesPersistence.Filter(mSearchTerm, attributes);
    if (rc == -1) {
        std::string message = "Failed to filter attributes";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (auto& attribute : attributes) {
            ListCtrlData data(attribute.AttributeId, attribute.Name);
            entries.push_back(data);
        }

        SetDataToControls(entries);
    }

    pOkButton->Enable();
}

void EditListDialog::SearchStaticAttributeGroups() {}

std::string EditListDialog::GetSearchHintText()
{
    switch (mType) {
    case EditListEntityType::Employers:
        return "Search employers...";
    case EditListEntityType::Clients:
        return "Search clients...";
    case EditListEntityType::Projects:
        return "Search projects...";
    case EditListEntityType::Categories:
        return "Search categories...";
    case EditListEntityType::AttributeGroups:
        return "Search attribute groups...";
    case EditListEntityType::Attributes:
        return "Search attributes...";
    default:
        return "";
    }
}
} // namespace tks::UI::dlg
