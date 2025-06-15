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

#include "categoriesdlg.h"

#include <algorithm>
#include <optional>

#include <fmt/format.h>

#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../events.h"
#include "../notificationclientdata.h"

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../persistence/projectspersistence.h"
#include "../../persistence/categoriespersistence.h"

#include "../../models/projectmodel.h"

#include "../../ui/clientdata.h"

#include "../../utils/utils.h"

namespace tks::UI::dlg
{
CategoriesDialog::CategoriesDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "New Categories",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pNameTextCtrl(nullptr)
    , pColorPickerCtrl(nullptr)
    , pBillableCheckBoxCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pProjectChoiceCtrl(nullptr)
    , pListCtrl(nullptr)
    , pAddButton(nullptr)
    , pRemoveButton(nullptr)
    , pRemoveAllButton(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , bEditFromListCtrl(false)
    , mCategoryIndexEdit(-1)
    , mListItemIndex(-1)
    , mListItemIndexes()
    , mCategoryToAdd()
    , mCategoriesToAdd()
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Initialize();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void CategoriesDialog::Initialize()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();
}

void CategoriesDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Layout Sizer */
    auto layoutSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(layoutSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Left Sizer */
    auto leftSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(leftSizer, wxSizerFlags().Expand().Proportion(1));

    /* Details Box */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    leftSizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Name Ctrl */
    auto categoryNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAMETEXTCTRL);
    pNameTextCtrl->SetHint("Category name");
    pNameTextCtrl->SetToolTip("Enter a name for the category");

    pNameTextCtrl->SetValidator(NameValidator());

    /* Color Picker Ctrl */
    pColorPickerCtrl = new wxColourPickerCtrl(detailsBox, tksIDC_COLORPICKERCTRL);
    pColorPickerCtrl->SetToolTip("Pick a color to associate with the category");

    pBillableCheckBoxCtrl = new wxCheckBox(detailsBox, tksIDC_BILLABLECHECKBOXCTRL, "Billable");
    pBillableCheckBoxCtrl->SetToolTip("Indicates if a task captured with this category is billable");

    /* Details Grid Sizer */
    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(
        categoryNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(
        pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pColorPickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pBillableCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Description Box */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Description");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    leftSizer->Add(
        descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Description Text Ctrl */
    pDescriptionTextCtrl = new wxTextCtrl(descriptionBox,
        tksIDC_DESCRIPTIONTEXTCTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for a category");
    descriptionBoxSizer->Add(
        pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Project choice control */
    auto projectLabel = new wxStaticText(this, wxID_ANY, "Project");

    pProjectChoiceCtrl = new wxChoice(this, tksIDC_PROJECTCHOICECTRL);
    pProjectChoiceCtrl->SetToolTip("Select an (optional) project to associate this category with");

    leftSizer->Add(projectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    leftSizer->Add(pProjectChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Center Sizer */
    auto centerSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(centerSizer, 0);

    pAddButton = new wxButton(this, wxID_ADD, "Add >>");
    centerSizer->Add(pAddButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    pRemoveButton = new wxButton(this, wxID_REMOVE, "Remove");
    centerSizer->Add(pRemoveButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    pRemoveAllButton = new wxButton(this, wxID_DELETE, "Remove all");
    centerSizer->Add(pRemoveAllButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Right Sizer */
    auto rightSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(rightSizer, wxSizerFlags().Expand().Proportion(1));

    /* List Box */
    auto listStaticBox = new wxStaticBox(this, wxID_ANY, "Categories to add");
    auto listStaticBoxSizer = new wxStaticBoxSizer(listStaticBox, wxVERTICAL);
    rightSizer->Add(
        listStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* List Ctrl */
    pListCtrl = new wxListCtrl(listStaticBox,
        tksIDC_LISTCTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES);
    pListCtrl->EnableCheckBoxes();

    wxListItem nameColumn;
    nameColumn.SetId(0);
    nameColumn.SetText("Name");
    nameColumn.SetWidth(wxLIST_AUTOSIZE_USEHEADER);
    pListCtrl->InsertColumn(0, nameColumn);

    listStaticBoxSizer->Add(
        pListCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Horizontal Line*/
    auto bottomSeparationLine = new wxStaticLine(this, wxID_ANY);
    sizer->Add(bottomSeparationLine, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();
    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Set Dialog Sizer  */
    SetSizerAndFit(sizer);
}

void CategoriesDialog::FillControls()
{
    pProjectChoiceCtrl->Append("Select a project", new ClientData<std::int64_t>(-1));
    pProjectChoiceCtrl->SetSelection(0);
    pProjectChoiceCtrl->Disable();

    std::string defaultSearchTerm = "";
    std::vector<Model::ProjectModel> projects;
    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    int rc = projectPersistence.Filter(defaultSearchTerm, projects);
    if (rc != 0) {
        std::string message = "Failed to get projects";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        if (!projects.empty()) {
            if (!pProjectChoiceCtrl->IsEnabled()) {
                pProjectChoiceCtrl->Enable();
            }

            for (auto& project : projects) {
                pProjectChoiceCtrl->Append(
                    project.DisplayName, new ClientData<std::int64_t>(project.ProjectId));
            }
        }
    }

    pRemoveButton->Disable();
    pRemoveAllButton->Disable();
}

// clang-format off
void CategoriesDialog::ConfigureEventBindings()
{
    pAddButton->Bind(
        wxEVT_BUTTON,
        &CategoriesDialog::OnAdd,
        this,
        wxID_ADD
    );

    pRemoveButton->Bind(
        wxEVT_BUTTON,
        &CategoriesDialog::OnRemove,
        this,
        wxID_REMOVE
    );

    pRemoveAllButton->Bind(
        wxEVT_BUTTON,
        &CategoriesDialog::OnRemoveAll,
        this,
        wxID_DELETE
    );

    pListCtrl->Bind(
        wxEVT_LIST_ITEM_CHECKED,
        &CategoriesDialog::OnItemChecked,
        this
    );

    pListCtrl->Bind(
        wxEVT_LIST_ITEM_UNCHECKED,
        &CategoriesDialog::OnItemUnchecked,
        this
    );

    pListCtrl->Bind(
        wxEVT_LIST_ITEM_RIGHT_CLICK,
        &CategoriesDialog::OnItemRightClick,
        this
    );

    Bind(
        wxEVT_MENU,
        &CategoriesDialog::OnEdit,
        this,
        wxID_EDIT
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &CategoriesDialog::OnCancel,
        this,
        wxID_CANCEL
    );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &CategoriesDialog::OnOK,
        this,
        wxID_OK
    );
}
// clang-format on

void CategoriesDialog::FillControls(const Model::CategoryModel& category)
{
    pNameTextCtrl->ChangeValue(category.Name);
    pColorPickerCtrl->SetColour(category.Color);
    pBillableCheckBoxCtrl->SetValue(category.Billable);
    pDescriptionTextCtrl->SetValue(
        category.Description.has_value() ? category.Description.value() : "");
    if (!category.Description.has_value()) {
        pDescriptionTextCtrl->SetHint("Description (optional)");
    }

    if (category.ProjectId.has_value()) {
        for (unsigned int i = 0; i < pProjectChoiceCtrl->GetCount(); i++) {
            auto* data =
                reinterpret_cast<ClientData<std::int64_t>*>(pProjectChoiceCtrl->GetClientObject(i));
            if (category.ProjectId.value() == data->GetValue()) {
                pProjectChoiceCtrl->SetSelection(i);
                break;
            }
        }
    }
}

void CategoriesDialog::Append(Model::CategoryModel category)
{
    int listIndex = 0;
    int columnIndex = 0;

    listIndex = pListCtrl->InsertItem(columnIndex++, category.Name);
    pListCtrl->SetItemBackgroundColour(listIndex, category.Color);
}

void CategoriesDialog::Update(Model::CategoryModel category)
{
    int columnIndex = 0;

    pListCtrl->SetItem(mListItemIndex, columnIndex++, category.Name);
    pListCtrl->SetItemBackgroundColour(mListItemIndex, category.Color);

    mListItemIndex = -1;
}

void CategoriesDialog::OnAdd(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    TransferDataFromControls();

    if (!bEditFromListCtrl) {
        Append(mCategoryToAdd);
        mCategoriesToAdd.push_back(mCategoryToAdd);
        pRemoveAllButton->Enable();
    } else {
        mCategoriesToAdd[mCategoryIndexEdit] = mCategoryToAdd;
        Update(mCategoryToAdd);
    }

    mCategoryToAdd = Model::CategoryModel();
    mCategoryIndexEdit = -1;
    ResetControlValues();
}

void CategoriesDialog::OnEdit(wxCommandEvent& event)
{
    bEditFromListCtrl = true;
    auto name = ExtractNameFromListIndex(mListItemIndex);
    auto iterator = std::find_if(mCategoriesToAdd.begin(),
        mCategoriesToAdd.end(),
        [&](Model::CategoryModel& category) { return category.Name == name; });

    int index = std::distance(mCategoriesToAdd.begin(), iterator);
    auto& category = mCategoriesToAdd[index];
    mCategoryIndexEdit = index;

    FillControls(category);
}

void CategoriesDialog::OnRemove(wxCommandEvent& event)
{
    for (long index : mListItemIndexes) {
        auto nameAtIndex = ExtractNameFromListIndex(index);

        mCategoriesToAdd.erase(std::remove_if(mCategoriesToAdd.begin(),
            mCategoriesToAdd.end(),
            [&](Model::CategoryModel& category) { return category.Name == nameAtIndex; }));

        pListCtrl->DeleteItem(index);
    }

    pAddButton->Enable();
    pRemoveButton->Disable();
    mListItemIndexes.clear();
}

void CategoriesDialog::OnRemoveAll(wxCommandEvent& event)
{
    mCategoriesToAdd.clear();
    pListCtrl->DeleteAllItems();
    pRemoveAllButton->Disable();

    mCategoryToAdd = Model::CategoryModel();
    mCategoryIndexEdit = -1;
    ResetControlValues();
}

void CategoriesDialog::OnOK(wxCommandEvent& event)
{
    pOkButton->Disable();

    Persistence::CategoriesPersistence categoryPersistence(pLogger, mDatabaseFilePath);

    int ret = 0;
    std::string message = "";
    for (auto& category : mCategoriesToAdd) {
        std::int64_t categoryId = categoryPersistence.Create(category);
        ret = categoryId > 0 ? 1 : -1;
        if (ret == -1) {
            break;
        }
    }

    ret == -1 ? message = "Failed to create categories"
              : message = "Successfully created categories";

    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    if (ret == -1) {
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);

        pOkButton->Enable();
    } else {
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);

        EndModal(wxID_OK);
    }
}

void CategoriesDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void CategoriesDialog::OnItemChecked(wxListEvent& event)
{
    long index = event.GetIndex();
    mListItemIndexes.push_back(index);

    if (mListItemIndexes.size() >= 1) {
        pAddButton->Disable();
        pRemoveButton->Enable();
    }
}

void CategoriesDialog::OnItemUnchecked(wxListEvent& event)
{
    long index = event.GetIndex();
    mListItemIndexes.erase(std::remove(mListItemIndexes.begin(), mListItemIndexes.end(), index),
        mListItemIndexes.end());

    if (mListItemIndexes.size() == 0) {
        pAddButton->Enable();
        pRemoveButton->Disable();
    }
}

void CategoriesDialog::OnItemRightClick(wxListEvent& event)
{
    mListItemIndex = event.GetIndex();

    wxMenu menu;
    menu.Append(wxID_EDIT, "Edit");

    PopupMenu(&menu);
}

void CategoriesDialog::ResetControlValues()
{
    pNameTextCtrl->ChangeValue(wxEmptyString);
    pColorPickerCtrl->SetColour(*wxBLACK);
    pBillableCheckBoxCtrl->SetValue(false);
    pDescriptionTextCtrl->ChangeValue(wxEmptyString);
    pDescriptionTextCtrl->SetHint("Description (optional)");

    pProjectChoiceCtrl->SetSelection(0);
}

std::string CategoriesDialog::ExtractNameFromListIndex(long itemIndex)
{
    assert(itemIndex != -1);

    std::string name;

    wxListItem item;
    item.m_itemId = itemIndex;
    item.m_col = 0;
    item.m_mask = wxLIST_MASK_TEXT;
    pListCtrl->GetItem(item);

    name = item.GetText().ToStdString();

    return name;
}

bool CategoriesDialog::Validate()
{
    auto name = pNameTextCtrl->GetValue().ToStdString();
    if (name.empty()) {
        auto valMsg = "Name is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pNameTextCtrl);
        return false;
    }

    if (name.length() < MIN_CHARACTER_COUNT || name.length() > MAX_CHARACTER_COUNT_NAMES) {
        auto valMsg = fmt::format("Name must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pNameTextCtrl);
        return false;
    }

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    if (!description.empty() && (description.length() < MIN_CHARACTER_COUNT ||
                                    description.length() > MAX_CHARACTER_COUNT_DESCRIPTIONS)) {
        auto valMsg =
            fmt::format("Description must be at minimum {0} or maximum {1} characters long",
                MIN_CHARACTER_COUNT,
                MAX_CHARACTER_COUNT_DESCRIPTIONS);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pDescriptionTextCtrl);
        return false;
    }

    if (pProjectChoiceCtrl->IsEnabled()) {
        int projectIndex = pProjectChoiceCtrl->GetSelection();
        ClientData<std::int64_t>* projectIdData = reinterpret_cast<ClientData<std::int64_t>*>(
            pProjectChoiceCtrl->GetClientObject(projectIndex));

        if (projectIdData->GetValue() > 0) {
            auto projectId = projectIdData->GetValue();
            mCategoryToAdd.ProjectId = std::make_optional(projectId);
        } else {
            mCategoryToAdd.ProjectId = std::nullopt;
        }
    }

    return true;
}

void CategoriesDialog::TransferDataFromControls()
{
    auto name = pNameTextCtrl->GetValue().ToStdString();
    mCategoryToAdd.Name = Utils::TrimWhitespace(name);

    mCategoryToAdd.Color = pColorPickerCtrl->GetColour().GetRGB();
    mCategoryToAdd.Billable = pBillableCheckBoxCtrl->IsChecked();

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    mCategoryToAdd.Description =
        description.empty() ? std::nullopt : std::make_optional(description);

    if (pProjectChoiceCtrl->IsEnabled()) {
        int projectIndex = pProjectChoiceCtrl->GetSelection();
        ClientData<std::int64_t>* projectIdData = reinterpret_cast<ClientData<std::int64_t>*>(
            pProjectChoiceCtrl->GetClientObject(projectIndex));

        if (projectIdData->GetValue() > 0) {
            auto projectId = projectIdData->GetValue();
            mCategoryToAdd.ProjectId = std::make_optional(projectId);
        } else {
            mCategoryToAdd.ProjectId = std::nullopt;
        }
    }
}
} // namespace tks::UI::dlg
