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

#include "categoriesdlg.h"

#include <algorithm>

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../core/environment.h"

#include "../../dao/categorydao.h"

#include "../../utils/utils.h"

#include "../events.h"
#include "../notificationclientdata.h"

namespace tks::UI::dlg
{
CategoriesDialog::CategoriesDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Add Categories",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pEnv(env)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pNameTextCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pColorPickerCtrl(nullptr)
    , pBillableCtrl(nullptr)
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
    sizer->Add(layoutSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Left Sizer */
    auto leftSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(leftSizer, wxSizerFlags().Expand().Proportion(1));

    /* Details Box */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    leftSizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Name Ctrl */
    auto categoryNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAME);
    pNameTextCtrl->SetHint("Category name");
    pNameTextCtrl->SetToolTip("Enter a name for a Category");

    pNameTextCtrl->SetValidator(NameValidator());

    /* Color Picker Ctrl */
    pColorPickerCtrl = new wxColourPickerCtrl(detailsBox, tksIDC_COLORPICKER);
    pColorPickerCtrl->SetToolTip("Pick a color to associate with the category");

    pBillableCtrl = new wxCheckBox(detailsBox, tksIDC_BILLABLE, "Billable");
    pBillableCtrl->SetToolTip("Indicates if a task captured with this category is billable");

    /* Details Grid Sizer */
    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(categoryNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pColorPickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pBillableCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Description Box */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Description (optional)");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    leftSizer->Add(descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Description Text Ctrl */
    pDescriptionTextCtrl = new wxTextCtrl(descriptionBox,
        tksIDC_DESCRIPTION,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for a category");
    descriptionBoxSizer->Add(pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Center Sizer */
    auto centerSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(centerSizer, 0);

    pAddButton = new wxButton(this, wxID_ADD, "Add >>");
    centerSizer->Add(pAddButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    pRemoveButton = new wxButton(this, wxID_REMOVE, "Remove");
    centerSizer->Add(pRemoveButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    pRemoveAllButton = new wxButton(this, wxID_DELETE, "Remove all");
    centerSizer->Add(pRemoveAllButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Right Sizer */
    auto rightSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(rightSizer, wxSizerFlags().Expand().Proportion(1));

    /* List Box */
    auto listStaticBox = new wxStaticBox(this, wxID_ANY, "Categories to add");
    auto listStaticBoxSizer = new wxStaticBoxSizer(listStaticBox, wxVERTICAL);
    rightSizer->Add(listStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* List Ctrl */
    pListCtrl = new wxListCtrl(listStaticBox, tksIDC_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES);
    pListCtrl->EnableCheckBoxes();

    wxListItem nameColumn;
    nameColumn.SetId(0);
    nameColumn.SetText("Name");
    nameColumn.SetWidth(wxLIST_AUTOSIZE_USEHEADER);
    pListCtrl->InsertColumn(0, nameColumn);

    listStaticBoxSizer->Add(pListCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Horizontal Line*/
    auto bottomSeparationLine = new wxStaticLine(this, wxID_ANY);
    sizer->Add(bottomSeparationLine, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* Bottom Sizer */
    auto layoutBottomSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(layoutBottomSizer, wxSizerFlags().Border(wxALL, 5).Expand());

    layoutBottomSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();
    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    layoutBottomSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    layoutBottomSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Set Dialog Sizer  */
    SetSizerAndFit(sizer);
}

void CategoriesDialog::FillControls()
{
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
    pBillableCtrl->SetValue(category.Billable);
    pDescriptionTextCtrl->SetValue(category.Description.has_value() ? category.Description.value() : "");
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
    if (TransferDataAndValidate()) {
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
}

void CategoriesDialog::OnEdit(wxCommandEvent& event)
{
    bEditFromListCtrl = true;
    auto name = ExtractNameFromListIndex(mListItemIndex);
    auto iterator = std::find_if(mCategoriesToAdd.begin(), mCategoriesToAdd.end(), [&](Model::CategoryModel& category) {
        return category.Name == name;
    });

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

    DAO::CategoryDao categoryDao(pLogger, mDatabaseFilePath);

    int ret = 0;
    std::string message = "";
    for (auto& category : mCategoriesToAdd) {
        std::int64_t categoryId = categoryDao.Create(category);
        ret = categoryId > 0 ? 1 : -1;
        if (ret == -1) {
            break;
        }
    }

    ret == -1 ? message = "Failed to create employer" : message = "Successfully created employer";

    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    if (ret == -1) {
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);

        pOkButton->Enable();
    } else {
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
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
    mListItemIndexes.erase(
        std::remove(mListItemIndexes.begin(), mListItemIndexes.end(), index), mListItemIndexes.end());

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
    pBillableCtrl->SetValue(false);
    pDescriptionTextCtrl->ChangeValue(wxEmptyString);
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

bool CategoriesDialog::TransferDataAndValidate()
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
    if (!description.empty() &&
        (description.length() < MIN_CHARACTER_COUNT || description.length() > MAX_CHARACTER_COUNT_DESCRIPTIONS)) {
        auto valMsg = fmt::format("Description must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_DESCRIPTIONS);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pDescriptionTextCtrl);
        return false;
    }

    mCategoryToAdd.Name = Utils::TrimWhitespace(name);
    mCategoryToAdd.Color = pColorPickerCtrl->GetColour().GetRGB();
    mCategoryToAdd.Billable = pBillableCtrl->IsChecked();
    mCategoryToAdd.Description = description.empty() ? std::nullopt : std::make_optional(description);

    return true;
}
} // namespace tks::UI::dlg
