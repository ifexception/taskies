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

#include "categorydlg.h"

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
CategoryDialog::CategoryDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    std::int64_t categoryId,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Edit Category",
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
    , pProjectChoiceCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pDateCreatedReadonlyTextCtrl(nullptr)
    , pDateModifiedReadonlyTextCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , mCategoryId(categoryId)
    , mCategoryModel()
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Initialize();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void CategoryDialog::Initialize()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();
    DataToControls();
}

void CategoryDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Details Box */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    sizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Name Ctrl */
    auto categoryNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAMETEXTCTRL);
    pNameTextCtrl->SetHint("Category name");
    pNameTextCtrl->SetToolTip("Enter a name for a category");

    pNameTextCtrl->SetValidator(NameValidator());

    /* Color Picker Ctrl */
    pColorPickerCtrl = new wxColourPickerCtrl(detailsBox, tksIDC_COLORPICKERCTRL);
    pColorPickerCtrl->SetToolTip("Pick a color to associate with the category");

    pBillableCheckBoxCtrl = new wxCheckBox(detailsBox, tksIDC_BILLABLECHECKBOXCTRL, "Billable");
    pBillableCheckBoxCtrl->SetToolTip(
        "Indicates if a task captured with this category is billable");

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
    sizer->Add(
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
        pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Project choice control */
    auto projectLabel = new wxStaticText(this, wxID_ANY, "Project");

    pProjectChoiceCtrl = new wxChoice(this, tksIDC_PROJECTCHOICECTRL);
    pProjectChoiceCtrl->SetToolTip("Select an (optional) project to associate this category with");

    sizer->Add(projectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    sizer->Add(pProjectChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Begin edit metadata controls */

    /* Horizontal Line */
    auto line1 = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line1, wxSizerFlags().Border(wxTOP | wxBOTTOM, FromDIP(4)).Expand());

    /* Date Created text control */
    auto dateCreatedLabel = new wxStaticText(this, wxID_ANY, "Date Created");

    pDateCreatedReadonlyTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
    pDateCreatedReadonlyTextCtrl->Disable();

    /* Date Modified text control */
    auto dateModifiedLabel = new wxStaticText(this, wxID_ANY, "Date Modified");

    pDateModifiedReadonlyTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
    pDateModifiedReadonlyTextCtrl->Disable();

    /* Is Active checkbox control */
    pIsActiveCheckBoxCtrl = new wxCheckBox(this, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Toggle the deleted state of an employer");
    pIsActiveCheckBoxCtrl->Disable();

    /* Metadata flex grid sizer */
    auto metadataFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    sizer->Add(metadataFlexGridSizer, wxSizerFlags().Expand());
    metadataFlexGridSizer->AddGrowableCol(1, 1);

    metadataFlexGridSizer->Add(
        dateCreatedLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    metadataFlexGridSizer->Add(
        pDateCreatedReadonlyTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    metadataFlexGridSizer->Add(
        dateModifiedLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    metadataFlexGridSizer->Add(
        pDateModifiedReadonlyTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    metadataFlexGridSizer->Add(0, 0);
    metadataFlexGridSizer->Add(pIsActiveCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* End of edit metadata controls */

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();
    pOkButton->Disable();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(sizer);
}

void CategoryDialog::FillControls()
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
}

// clang-format off
void CategoryDialog::ConfigureEventBindings()
{
    pIsActiveCheckBoxCtrl->Bind(
            wxEVT_CHECKBOX,
            &CategoryDialog::OnIsActiveCheck,
            this
        );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &CategoryDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &CategoryDialog::OnCancel,
        this,
        wxID_CANCEL
    );
}
// clang-format on

void CategoryDialog::DataToControls()
{
    Model::CategoryModel model;
    Persistence::CategoriesPersistence categoryPersistence(pLogger, mDatabaseFilePath);
    int rc = 0;

    rc = categoryPersistence.GetById(mCategoryId, model);
    if (rc != 0) {
        std::string message = "Failed to get category";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // We are editing, so pParent is EditListDlg. We need to get parent of pParent and then we
        // have wxFrame
        wxQueueEvent(pParent->GetParent(), addNotificationEvent);
    } else {
        pNameTextCtrl->ChangeValue(model.Name);

        pColorPickerCtrl->SetColour(model.Color);
        pBillableCheckBoxCtrl->SetValue(model.Billable);

        pDescriptionTextCtrl->SetValue(
            model.Description.has_value() ? model.Description.value() : "");

        pDateCreatedReadonlyTextCtrl->SetValue(model.GetDateCreatedString());
        pDateModifiedReadonlyTextCtrl->SetValue(model.GetDateModifiedString());
        pIsActiveCheckBoxCtrl->SetValue(model.IsActive);
        pIsActiveCheckBoxCtrl->Enable();

        if (model.ProjectId.has_value()) {
            for (unsigned int i = 0; i < pProjectChoiceCtrl->GetCount(); i++) {
                auto* data = reinterpret_cast<ClientData<std::int64_t>*>(
                    pProjectChoiceCtrl->GetClientObject(i));
                if (model.ProjectId.value() == data->GetValue()) {
                    pProjectChoiceCtrl->SetSelection(i);
                    break;
                }
            }
        }
    }

    pOkButton->Enable();
}

void CategoryDialog::OnIsActiveCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pNameTextCtrl->Enable();
        pColorPickerCtrl->Enable();
        pBillableCheckBoxCtrl->Enable();
        pDescriptionTextCtrl->Enable();
        pProjectChoiceCtrl->Enable();

    } else {
        pNameTextCtrl->Disable();
        pColorPickerCtrl->Disable();
        pBillableCheckBoxCtrl->Disable();
        pDescriptionTextCtrl->Disable();

        pProjectChoiceCtrl->SetSelection(0);
        pProjectChoiceCtrl->Disable();
    }
}

void CategoryDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    pOkButton->Disable();

    TransferDataFromControls();

    Persistence::CategoriesPersistence categoryPersistence(pLogger, mDatabaseFilePath);

    int ret = 0;
    std::string message = "";
    if (pIsActiveCheckBoxCtrl->IsChecked()) {
        ret = categoryPersistence.Update(mCategoryModel);

        ret == -1 ? message = "Failed to update category"
                  : message = "Successfully updated category";
    }
    if (!pIsActiveCheckBoxCtrl->IsChecked()) {
        ret = categoryPersistence.Delete(mCategoryId);

        ret == -1 ? message = "Failed to delete category"
                  : message = "Successfully deleted category";
    }

    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    if (ret == -1) {
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // We are editing, so pParent is EditListDlg. We need to get parent of pParent and then
        // we have wxFrame
        wxQueueEvent(pParent->GetParent(), addNotificationEvent);

        pOkButton->Enable();
    } else {
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        // We are editing, so pParent is EditListDlg. We need to get parent of pParent and then
        // we have wxFrame
        wxQueueEvent(pParent->GetParent(), addNotificationEvent);

        EndModal(wxID_OK);
    }
}

void CategoryDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool CategoryDialog::Validate()
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

    return true;
}

void CategoryDialog::TransferDataFromControls()
{
    mCategoryModel.CategoryId = mCategoryId;

    auto name = pNameTextCtrl->GetValue().ToStdString();
    mCategoryModel.Name = Utils::TrimWhitespace(name);

    mCategoryModel.Color = pColorPickerCtrl->GetColour().GetRGB();
    mCategoryModel.Billable = pBillableCheckBoxCtrl->IsChecked();

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    mCategoryModel.Description =
        description.empty() ? std::nullopt : std::make_optional(description);

    if (pProjectChoiceCtrl->IsEnabled()) {
        int projectIndex = pProjectChoiceCtrl->GetSelection();
        ClientData<std::int64_t>* projectIdData = reinterpret_cast<ClientData<std::int64_t>*>(
            pProjectChoiceCtrl->GetClientObject(projectIndex));

        if (projectIdData->GetValue() > 0) {
            auto projectId = projectIdData->GetValue();
            mCategoryModel.ProjectId = std::make_optional(projectId);
        } else {
            mCategoryModel.ProjectId = std::nullopt;
        }
    }
}
} // namespace tks::UI::dlg
