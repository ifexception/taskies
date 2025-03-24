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

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../core/environment.h"

#include "../../persistence/projectspersistence.h"
#include "../../persistence/categorypersistence.h"

#include "../../models/projectmodel.h"

#include "../../ui/clientdata.h"

#include "../../utils/utils.h"

#include "../events.h"
#include "../notificationclientdata.h"

namespace tks::UI::dlg
{
CategoryDialog::CategoryDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
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
    , pEnv(env)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pNameTextCtrl(nullptr)
    , pColorPickerCtrl(nullptr)
    , pBillableCtrl(nullptr)
    , pProjectChoiceCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pDateCreatedTextCtrl(nullptr)
    , pDateModifiedTextCtrl(nullptr)
    , pIsActiveCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , mCategoryId(categoryId)
    , mModel()
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

    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAME);
    pNameTextCtrl->SetHint("Category name");
    pNameTextCtrl->SetToolTip("Enter a name for a category");

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
    sizer->Add(descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

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

    /* Project Choice Box */
    auto selectionBox = new wxStaticBox(this, wxID_ANY, "Selection");
    auto selectionBoxSizer = new wxStaticBoxSizer(selectionBox, wxVERTICAL);
    sizer->Add(selectionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Project choice control */
    auto projectLabel = new wxStaticText(selectionBox, wxID_ANY, "Project");

    pProjectChoiceCtrl = new wxChoice(selectionBox, tksIDC_PROJECTCHOICE);
    pProjectChoiceCtrl->SetToolTip("Select an (optional) project to associate this category with");

    auto projectChoiceGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    projectChoiceGridSizer->AddGrowableCol(1, 1);

    projectChoiceGridSizer->Add(projectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    projectChoiceGridSizer->Add(pProjectChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    selectionBoxSizer->Add(projectChoiceGridSizer, wxSizerFlags().Expand().Proportion(1));

    auto metadataLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(3), FromDIP(3)));
    sizer->Add(metadataLine, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    auto metadataBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto metadataBoxSizer = new wxStaticBoxSizer(metadataBox, wxVERTICAL);
    sizer->Add(metadataBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    /* FlexGrid sizer */
    auto metadataFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    metadataBoxSizer->Add(metadataFlexGridSizer, wxSizerFlags().Expand().Proportion(1));
    metadataFlexGridSizer->AddGrowableCol(1, 1);

    /* Date Created */
    auto dateCreatedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Created");
    metadataFlexGridSizer->Add(dateCreatedLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());

    pDateCreatedTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, wxEmptyString);
    pDateCreatedTextCtrl->Disable();
    metadataFlexGridSizer->Add(pDateCreatedTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    /* Date Modified */
    auto dateModifiedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Modified");
    metadataFlexGridSizer->Add(dateModifiedLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());

    pDateModifiedTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, wxEmptyString);
    pDateModifiedTextCtrl->Disable();
    metadataFlexGridSizer->Add(pDateModifiedTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    /* Is Active checkbox control */
    metadataFlexGridSizer->Add(0, 0);

    pIsActiveCtrl = new wxCheckBox(metadataBox, tksIDC_ISACTIVE, "Is Active");
    pIsActiveCtrl->SetToolTip("Indicates if this category is being used");
    metadataFlexGridSizer->Add(pIsActiveCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)));

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

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

void CategoryDialog::FillControls()
{
    pProjectChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pProjectChoiceCtrl->SetSelection(0);
    pProjectChoiceCtrl->Disable();

    std::string defaultSearchTerm = "";
    std::vector<Model::ProjectModel> projects;
    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    int rc = projectPersistence.Filter(defaultSearchTerm, projects);
    if (rc != 0) {
        std::string message = "Failed to get projects";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        if (!projects.empty()) {
            if (!pProjectChoiceCtrl->IsEnabled()) {
                pProjectChoiceCtrl->Enable();
            }

            for (auto& project : projects) {
                pProjectChoiceCtrl->Append(project.DisplayName, new ClientData<std::int64_t>(project.ProjectId));
            }
        } else {
            pProjectChoiceCtrl->Disable();
        }
    }
}

// clang-format off
void CategoryDialog::ConfigureEventBindings()
{
    pIsActiveCtrl->Bind(
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
    Persistence::CategoryPersistence categoryPersistence(pLogger, mDatabaseFilePath);
    int rc = 0;

    rc = categoryPersistence.GetById(mCategoryId, model);
    if (rc != 0) {
        std::string message = "Failed to get category";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // We are editing, so pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
        wxQueueEvent(pParent->GetParent(), addNotificationEvent);
    } else {
        pNameTextCtrl->ChangeValue(model.Name);
        pColorPickerCtrl->SetColour(model.Color);
        pBillableCtrl->SetValue(model.Billable);
        pDescriptionTextCtrl->SetValue(model.Description.has_value() ? model.Description.value() : "");
        pDateCreatedTextCtrl->SetValue(model.GetDateCreatedString());
        pDateModifiedTextCtrl->SetValue(model.GetDateModifiedString());
        pIsActiveCtrl->SetValue(model.IsActive);

        if (model.ProjectId.has_value()) {
            Model::ProjectModel project;
            Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

            int rc = projectPersistence.GetById(model.ProjectId.value(), project);
            if (rc != 0) {
                std::string message = "Failed to get project";
                wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
                NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
                addNotificationEvent->SetClientObject(clientData);

                wxQueueEvent(pParent, addNotificationEvent);
            }

            pProjectChoiceCtrl->SetStringSelection(project.DisplayName);
        }
    }

    pOkButton->Enable();
}

void CategoryDialog::OnIsActiveCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pNameTextCtrl->Enable();
        pColorPickerCtrl->Enable();
        pBillableCtrl->Enable();
        pDescriptionTextCtrl->Enable();
        pProjectChoiceCtrl->Enable();

    } else {
        pNameTextCtrl->Disable();
        pColorPickerCtrl->Disable();
        pBillableCtrl->Disable();
        pDescriptionTextCtrl->Disable();
        pProjectChoiceCtrl->SetSelection(0);
        pProjectChoiceCtrl->Disable();
    }
}

void CategoryDialog::OnOK(wxCommandEvent& event)
{
    pOkButton->Disable();

    if (TransferDataAndValidate()) {
        Persistence::CategoryPersistence categoryPersistence(pLogger, mDatabaseFilePath);

        int ret = 0;
        std::string message = "";
        if (pIsActiveCtrl->IsChecked()) {
            ret = categoryPersistence.Update(mModel);

            ret == -1 ? message = "Failed to update category" : message = "Successfully updated category";
        }
        if (!pIsActiveCtrl->IsChecked()) {
            ret = categoryPersistence.Delete(mCategoryId);

            ret == -1 ? message = "Failed to delete category" : message = "Successfully deleted category";
        }

        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        if (ret == -1) {
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
            addNotificationEvent->SetClientObject(clientData);

            // We are editing, so pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
            wxQueueEvent(pParent->GetParent(), addNotificationEvent);

            pOkButton->Enable();
        } else {
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
            addNotificationEvent->SetClientObject(clientData);

            // We are editing, so pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
            wxQueueEvent(pParent->GetParent(), addNotificationEvent);

            EndModal(wxID_OK);
        }
    }
}

void CategoryDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool CategoryDialog::TransferDataAndValidate()
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

    if (pProjectChoiceCtrl->IsEnabled()) {
        int projectIndex = pProjectChoiceCtrl->GetSelection();
        ClientData<std::int64_t>* projectIdData =
            reinterpret_cast<ClientData<std::int64_t>*>(pProjectChoiceCtrl->GetClientObject(projectIndex));

        if (projectIdData->GetValue() > 0) {
            auto projectId = projectIdData->GetValue();
            mModel.ProjectId = std::make_optional(projectId);
        } else {
            mModel.ProjectId = std::nullopt;
        }
    }

    mModel.CategoryId = mCategoryId;
    mModel.Name = Utils::TrimWhitespace(name);
    mModel.Color = pColorPickerCtrl->GetColour().GetRGB();
    mModel.Billable = pBillableCtrl->IsChecked();
    mModel.Description = description.empty() ? std::nullopt : std::make_optional(description);

    return true;
}
} // namespace tks::UI::dlg
