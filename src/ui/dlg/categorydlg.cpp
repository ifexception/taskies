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

#include "categorydlg.h"

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/constants.h"

#include "../../data/categorydata.h"

#include "../../utils/utils.h"

#include "errordlg.h"

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
    , pBillableCtrl(nullptr)
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

    pNameTextCtrl = new wxTextCtrl(detailsBox, IDC_NAME);
    pNameTextCtrl->SetHint("Category name");
    pNameTextCtrl->SetToolTip("Enter a name for a Category");

    wxTextValidator nameValidator(wxFILTER_ALPHANUMERIC | wxFILTER_INCLUDE_CHAR_LIST);
    wxArrayString allowedCharacters;
    allowedCharacters.Add(" ");
    allowedCharacters.Add("-");
    allowedCharacters.Add(":");
    allowedCharacters.Add(";");
    allowedCharacters.Add(".");
    allowedCharacters.Add("|");
    allowedCharacters.Add("(");
    allowedCharacters.Add(")");
    allowedCharacters.Add("+");
    nameValidator.SetIncludes(allowedCharacters);

    pNameTextCtrl->SetValidator(nameValidator);

    /* Color Picker Ctrl */
    pColorPickerCtrl = new wxColourPickerCtrl(detailsBox, IDC_COLORPICKER);
    pColorPickerCtrl->SetToolTip("Pick a color to associate with the category");

    pBillableCtrl = new wxCheckBox(detailsBox, IDC_BILLABLE, "Billable");
    pBillableCtrl->SetToolTip("Indicates if a task captured with associated category is billable");

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
    pDescriptionTextCtrl = new wxTextCtrl(
        descriptionBox, IDC_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for a category");
    descriptionBoxSizer->Add(pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

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

    pIsActiveCtrl = new wxCheckBox(metadataBox, IDC_ISACTIVE, "Is Active");
    pIsActiveCtrl->SetToolTip("Indicates if this category entry is being used");
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
    Data::CategoryData data(pLogger, mDatabaseFilePath);
    int rc = 0;

    rc = data.GetById(mCategoryId, model);
    if (rc != 0) {
        auto errorMessage = "An unexpected error occured and the specified action could not be completed. Please "
                            "check the logs for more information...";

        ErrorDialog errorDialog(this, pLogger, errorMessage);
        errorDialog.ShowModal();
    } else {
        pNameTextCtrl->ChangeValue(model.Name);
        pColorPickerCtrl->SetColour(model.Color);
        pBillableCtrl->SetValue(model.Billable);
        pDescriptionTextCtrl->SetValue(model.Description.has_value() ? model.Description.value() : "");
        pDateCreatedTextCtrl->SetValue(model.GetDateCreatedString());
        pDateModifiedTextCtrl->SetValue(model.GetDateModifiedString());
        pIsActiveCtrl->SetValue(model.IsActive);
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
    } else {
        pNameTextCtrl->Disable();
        pColorPickerCtrl->Disable();
        pBillableCtrl->Disable();
        pDescriptionTextCtrl->Disable();
    }
}

void CategoryDialog::OnOK(wxCommandEvent& event)
{
    pOkButton->Disable();

    if (TransferDataAndValidate()) {
        int ret = 0;
        Data::CategoryData data(pLogger, mDatabaseFilePath);

        if (pIsActiveCtrl->IsChecked()) {
            ret = data.Update(mModel);
        }
        if (!pIsActiveCtrl->IsChecked()) {
            ret = data.Delete(mCategoryId);
        }

        if (ret == -1) {
            pLogger->error("Failed to execute action with project. Check further logs for more information");
            auto errorMessage = "An unexpected error occured and the specified action could not be completed. Please "
                                "check logs for more information...";

            ErrorDialog errorDialog(this, pLogger, errorMessage);
            errorDialog.ShowModal();

            pOkButton->Enable();
        } else {
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

    mModel.CategoryId = mCategoryId;
    mModel.Name = name;
    mModel.Color = pColorPickerCtrl->GetColour().GetRGB();
    mModel.Billable = pBillableCtrl->IsChecked();
    mModel.Description = description.empty() ? std::nullopt : std::make_optional(description);

    return true;
}
} // namespace tks::UI::dlg
