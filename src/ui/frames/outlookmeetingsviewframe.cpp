// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#include "outlookmeetingsviewframe.h"

#include <algorithm>

#include <date/date.h>

#include <wx/artprov.h>
#include <wx/richmsgdlg.h>
#include <wx/statline.h>

#include "../events.h"

#include "../common/clientdata.h"

#include "../dlg/taskdlg.h"

#include "../../common/common.h"
#include "../../common/enums.h"

#include "../../common/messages/persistencemessages.h"

#include "../../core/configuration.h"
#include "../../core/environment.h"

#include "../../models/employermodel.h"
#include "../../models/projectmodel.h"

#include "../../persistence/attendedmeetingspersistence.h"
#include "../../persistence/employerspersistence.h"
#include "../../persistence/projectspersistence.h"

#include "../../services/categories/categoryviewmodel.h"
#include "../../services/categories/categoryservice.h"
#include "../../services/outlook/outlookclassicservice.h"

#include "../../utils/utils.h"

namespace tks::UI::frames
{
OutlookMeetingsViewFrame::OutlookMeetingsViewFrame(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool isMainFrameMaximized,
    const wxString& name)
    : wxFrame(parent,
          wxID_ANY,
          "Outlook Meetings",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pCfg(cfg)
    , pEnv(env)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pParent(parent)
    , pThisPanel(nullptr)
    , pMainSizer(nullptr)
    , pDatePickerCtrl(nullptr)
    , pRefreshButton(nullptr)
    , pEmployerChoiceCtrl(nullptr)
    , pAccountsChoiceCtrl(nullptr)
    , pFeedbackLabel(nullptr)
    , pScrolledWindow(nullptr)
    , pScrolledWindowSizer(nullptr)
    , pActiveMeetingsPanel(nullptr)
    , mSelectedAccount()
    , mSelectedDate()
    , mMeetingModels()
    , bIsMainFrameMaximized(isMainFrameMaximized)
    , mEmployerId(-1)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);

    auto todaysDate = date::floor<date::days>(std::chrono::system_clock::now());
    mSelectedDate = date::format("%Y/%m/%d", todaysDate);
}

OutlookMeetingsViewFrame::~OutlookMeetingsViewFrame() {}

void OutlookMeetingsViewFrame::Create()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();

    wxSize parentWindowSize = pParent->GetSize();
    SPDLOG_LOGGER_TRACE(pLogger,
        "PARENT SIZE ({0},{1})",
        parentWindowSize.GetHeight(),
        parentWindowSize.GetWidth());
    wxSize dialogMaxSize;
    dialogMaxSize.SetHeight(parentWindowSize.GetHeight());
    dialogMaxSize.SetWidth(-1);
    SetMaxSize(dialogMaxSize);

    if (!bIsMainFrameMaximized) {
        SetSize(dialogMaxSize);

        wxPoint screenPosition = pParent->GetScreenPosition();
        int screenX = screenPosition.x + parentWindowSize.x;
        int screenY = screenPosition.y;
        SPDLOG_LOGGER_TRACE(pLogger, "PARENT POSITION ({0},{1})", screenX, screenY);
        wxPoint topRightScreen(screenX, screenY);
        SetPosition(topRightScreen);

        SetMinSize(wxSize(240, 180));
    } else {
        CenterOnScreen();
    }
}

void OutlookMeetingsViewFrame::CreateControls()
{
    /* Main dialog sizer for controls */
    pMainSizer = new wxBoxSizer(wxVERTICAL);

    /* Main panel for the frame */
    pThisPanel = new wxPanel(this, wxID_ANY);
    pThisPanel->SetSizer(pMainSizer);

    /* Date picker and refresh button sizer */
    auto datePickerAndButtonHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    pMainSizer->Add(datePickerAndButtonHorizontalSizer, wxSizerFlags().Expand());

    /* Date picker ctrl */
    pDatePickerCtrl = new wxDatePickerCtrl(pThisPanel, tksIDC_DATEPICKERCTRL);
    pDatePickerCtrl->SetToolTip("Filter Outlook meetings by date");
    pDatePickerCtrl->Disable();

    /* Refresh button */
    auto providedRefreshBitmap = wxArtProvider::GetBitmapBundle(
        wxART_REFRESH, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pRefreshButton = new wxBitmapButton(pThisPanel, tksIDC_REFRESH_BUTTON, providedRefreshBitmap);
    pRefreshButton->SetToolTip("Refresh meetings of selected account");
    pRefreshButton->Disable();

    datePickerAndButtonHorizontalSizer->Add(
        pDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    datePickerAndButtonHorizontalSizer->AddStretchSpacer();
    datePickerAndButtonHorizontalSizer->Add(
        pRefreshButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Employer choice ctrl */
    auto employerLabel = new wxStaticText(pThisPanel, wxID_ANY, "Employer");
    pEmployerChoiceCtrl = new wxChoice(pThisPanel, tksIDC_EMPLOYERCHOICECTRL);
    pEmployerChoiceCtrl->SetToolTip("Select employer to associate this meeting task with");

    pMainSizer->Add(employerLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    pMainSizer->Add(pEmployerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Horizontal Line */
    auto line0 = new wxStaticLine(pThisPanel, wxID_ANY);
    pMainSizer->Add(line0, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Account label and choice control */
    auto accountLabel = new wxStaticText(pThisPanel, wxID_ANY, "Account");

    pAccountsChoiceCtrl = new wxChoice(pThisPanel, tksIDC_ACCOUNT_CHOICE_CTRL);
    pAccountsChoiceCtrl->SetToolTip("Select an account to display meetings for");

    pAccountsChoiceCtrl->SetFocus();

    pMainSizer->Add(accountLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    pMainSizer->Add(pAccountsChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Feedback label */
    pFeedbackLabel = new wxStaticText(pThisPanel, tksIDC_FEEDBACKLABEL, "No account selected");
    pMainSizer->Add(
        pFeedbackLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal().Top());

    /* Main Scrolled Window */
    pScrolledWindow = new wxScrolledWindow(pThisPanel, wxID_ANY);
    pScrolledWindowSizer = new wxBoxSizer(wxVERTICAL);
    pScrolledWindow->SetSizer(pScrolledWindowSizer);
    pScrolledWindow->SetScrollRate(0, 20);
    pScrolledWindowSizer->FitInside(pScrolledWindow);

    pMainSizer->Add(pScrolledWindow, wxSizerFlags(1).Expand());
}

void OutlookMeetingsViewFrame::FillControls()
{
    wxDateTime dt = wxDateTime::Now();
    dt.SetDay(1);
    dt.SetMonth(wxDateTime::Jan);

    pDatePickerCtrl->SetRange(dt, wxDateTime::Now());

    pEmployerChoiceCtrl->Append("Select employer", new ClientData<std::int64_t>(-1));
    pEmployerChoiceCtrl->SetSelection(0);

    std::vector<Model::EmployerModel> employers;
    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = employerPersistence.Filter("", employers);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterEmployersMessage,
            tks::Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        for (auto& employer : employers) {
            pEmployerChoiceCtrl->Append(
                employer.Name, new ClientData<std::int64_t>(employer.EmployerId));

            if (employer.IsDefault) {
                mEmployerId = employer.EmployerId;
                pEmployerChoiceCtrl->SetStringSelection(employer.Name);
            }
        }
    }

    pAccountsChoiceCtrl->Append("Select account");
    pAccountsChoiceCtrl->SetSelection(0);
}

// clang-format off
void OutlookMeetingsViewFrame::ConfigureEventBindings()
{
    pDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &OutlookMeetingsViewFrame::OnDateSelection,
        this,
        tksIDC_DATEPICKERCTRL
    );

    pRefreshButton->Bind(
        wxEVT_BUTTON,
        &OutlookMeetingsViewFrame::OnRefresh,
        this,
        tksIDC_REFRESH_BUTTON
    );

    pEmployerChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &OutlookMeetingsViewFrame::OnEmployerChoice,
        this
    );

    pAccountsChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &OutlookMeetingsViewFrame::OnAccountChoice,
        this
    );

    Bind(
        wxEVT_CLOSE_WINDOW,
        &OutlookMeetingsViewFrame::OnClose,
        this,
        wxID_ANY
    );
}
// clang-format on

void OutlookMeetingsViewFrame::DataToControls()
{
    pFeedbackLabel->SetLabel(
        "Fetching Outlook (classic) meetings may invoke additional auto-closing dialogs");

    std::vector<std::string> accountNames;

    Services::Outlook::OutlookClassicService service(pLogger);
    Services::Outlook::OutlookResult result;
    {
        wxBusyCursor cursor;

        result = service.FetchAccountNames(accountNames);
    }

    if (!result.Success) {
        std::string message = "Failed to fetch Outlook accounts";
        pFeedbackLabel->SetLabel(message);

        wxMessageDialog dialog(this,
            message,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(result.Message);

        dialog.ShowModal();

        return;
    }

    for (const std::string& accountName : accountNames) {
        pAccountsChoiceCtrl->Append(accountName);
    }

    if (accountNames.size() == 1) {
        wxBusyCursor cursor;

        const int AccountChoiceIndexOne = 1;

        pAccountsChoiceCtrl->SetSelection(AccountChoiceIndexOne);
        wxCommandEvent event(wxEVT_CHOICE, pAccountsChoiceCtrl->GetId());
        event.SetEventObject(pAccountsChoiceCtrl);
        event.SetInt(AccountChoiceIndexOne);
        event.SetString(pAccountsChoiceCtrl->GetString(AccountChoiceIndexOne));

        wxPostEvent(pAccountsChoiceCtrl->GetEventHandler(), event);
    }
}

void OutlookMeetingsViewFrame::OnParentFrameMove()
{
    wxSize parentWindowSize = pParent->GetSize();
    wxPoint screenPos = pParent->GetScreenPosition();
    int screenX = screenPos.x + parentWindowSize.x;
    int screenY = screenPos.y;

    SPDLOG_LOGGER_TRACE(
        pLogger, "Parent position changed to new position ({0},{1})", screenX, screenY);

    wxPoint topRightScreen(screenX, screenY);
    SetPosition(topRightScreen);
}

void OutlookMeetingsViewFrame::OnParentFrameResize()
{
    wxSize parentWindowSize = pParent->GetSize();
    SPDLOG_LOGGER_TRACE(pLogger,
        "PARENT SIZE ({0},{1})",
        parentWindowSize.GetHeight(),
        parentWindowSize.GetWidth());

    wxSize dialogSize;
    dialogSize.SetHeight(parentWindowSize.GetHeight());
    dialogSize.SetWidth(-1);
    SetSize(dialogSize);
    SetMaxSize(dialogSize);

    if (!bIsMainFrameMaximized) {
        OnParentFrameMove();
    }
}

void OutlookMeetingsViewFrame::OnDateSelection(wxDateEvent& event)
{
    if (mSelectedAccount.empty()) {
        // if no account is selected, we return as there is nothing to do
        return;
    }

    const wxDateTime& selectedDate = event.GetDate();
    mSelectedDate = selectedDate.Format("%Y/%m/%d").ToStdString();

    wxBusyCursor cursor;

    mMeetingModels.clear();

    if (pActiveMeetingsPanel != nullptr) {
        RemoveActiveMeetingsPanel();
    }

    if (!pRefreshButton->IsEnabled()) {
        pRefreshButton->Enable();
    }

    FetchOutlookMeetingsAndUpdateFeedbackLabel();

    std::vector<Model::AttendedMeetingModel> attendedMeetingModels = FetchAttendedMeetings();

    AddMeetingsToPanel(attendedMeetingModels);

    SetDialogSizeFromParent();
}

void OutlookMeetingsViewFrame::OnRefresh(wxCommandEvent& event)
{
    wxBusyCursor cursor;

    if (mSelectedAccount.empty()) {
        ResetFeedbackLabelOnNoData();

        return;
    }

    mMeetingModels.clear();

    if (pActiveMeetingsPanel != nullptr) {
        RemoveActiveMeetingsPanel();
    }

    FetchOutlookMeetingsAndUpdateFeedbackLabel();

    std::vector<Model::AttendedMeetingModel> attendedMeetingModels = FetchAttendedMeetings();

    AddMeetingsToPanel(attendedMeetingModels);

    SetDialogSizeFromParent();
}

void OutlookMeetingsViewFrame::OnEmployerChoice(wxCommandEvent& event)
{
    int employerIndex = event.GetSelection();
    ClientData<std::int64_t>* employerIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pEmployerChoiceCtrl->GetClientObject(employerIndex));

    if (employerIdData->GetValue() < 1) {
        for (size_t i = 0; i < mControlChoicesData.size(); i++) {
            wxWindow* projectWnd = FindWindowById(mControlChoicesData[i].ProjectChoiceControlId);
            if (projectWnd) {
                wxChoice* projectChoiceCtrl = wxDynamicCast(projectWnd, wxChoice);
                if (projectChoiceCtrl) {
                    projectChoiceCtrl->Clear();
                    projectChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
                    projectChoiceCtrl->SetSelection(0);
                    projectChoiceCtrl->Disable();
                }
            }

            wxWindow* categoryWnd = FindWindowById(mControlChoicesData[i].CategoryChoiceControlId);
            if (categoryWnd) {
                wxChoice* categoryChoiceCtrl = wxDynamicCast(categoryWnd, wxChoice);
                if (categoryChoiceCtrl) {
                    categoryChoiceCtrl->Clear();
                    categoryChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
                    categoryChoiceCtrl->SetSelection(0);
                    categoryChoiceCtrl->Disable();
                }
            }
        }

        mEmployerId = -1;

        return;
    }

    mEmployerId = employerIdData->GetValue();

    for (size_t i = 0; i < mControlChoicesData.size(); i++) {
        wxWindow* projectWnd = FindWindowById(mControlChoicesData[i].ProjectChoiceControlId);
        if (projectWnd) {
            wxChoice* projectChoiceCtrl = wxDynamicCast(projectWnd, wxChoice);
            if (projectChoiceCtrl) {
                projectChoiceCtrl->Clear();
                projectChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
                projectChoiceCtrl->SetSelection(0);
            }
        }

        wxWindow* categoryWnd = FindWindowById(mControlChoicesData[i].CategoryChoiceControlId);
        if (categoryWnd) {
            wxChoice* categoryChoiceCtrl = wxDynamicCast(categoryWnd, wxChoice);
            if (categoryChoiceCtrl) {
                categoryChoiceCtrl->Clear();
                categoryChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
                categoryChoiceCtrl->SetSelection(0);
            }
        }
    }

    if (mControlChoicesData.size() <= 0) {
        return;
    }

    std::vector<Model::ProjectModel> projectModels;
    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = projectPersistence.FilterByEmployerId(mEmployerId, projectModels);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterProjectsMessage,
            tks::Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();

        return;
    }
    for (size_t i = 0; i < mControlChoicesData.size(); i++) {
        wxWindow* projectWnd = FindWindowById(mControlChoicesData[i].ProjectChoiceControlId);
        if (projectWnd) {
            wxChoice* projectChoiceCtrl = wxDynamicCast(projectWnd, wxChoice);
            if (!projectChoiceCtrl->IsEnabled()) {
                projectChoiceCtrl->Enable();
            }
            if (projectChoiceCtrl) {
                if (!projectModels.empty()) {
                    bool hasDefaultProject = false;
                    std::int64_t defaultProjectId = -1;

                    for (auto& project : projectModels) {
                        projectChoiceCtrl->Append(
                            project.DisplayName, new ClientData<std::int64_t>(project.ProjectId));

                        if (project.IsDefault) {
                            hasDefaultProject = true;
                            defaultProjectId = project.ProjectId;
                            projectChoiceCtrl->SetStringSelection(project.DisplayName);
                        }
                    }

                    if (hasDefaultProject) {
                        std::vector<Services::CategoryViewModel> categories;
                        Services::CategoryService categoryService(pLogger, mDatabaseFilePath);

                        auto sqliteResult =
                            categoryService.FilterByProjectId(defaultProjectId, categories);

                        if (!sqliteResult.Success) {
                            wxRichMessageDialog dialog(this,
                                Messages::FilterCategoriesByProjectMessage,
                                tks::Common::GetProgramName(),
                                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
                            dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
                            dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

                            dialog.ShowModal();

                            return;
                        }
                        wxWindow* categoryWnd =
                            FindWindowById(mControlChoicesData[i].CategoryChoiceControlId);
                        if (categoryWnd) {
                            wxChoice* categoryChoiceCtrl = wxDynamicCast(categoryWnd, wxChoice);
                            if (categoryChoiceCtrl) {
                                if (!categoryChoiceCtrl->IsEnabled()) {
                                    categoryChoiceCtrl->Enable();
                                }
                                if (!categories.empty()) {
                                    for (auto& category : categories) {
                                        categoryChoiceCtrl->Append(category.GetFormattedName(),
                                            new ClientData<std::int64_t>(category.CategoryId));
                                    }
                                } else {
                                    categoryChoiceCtrl->Disable();
                                }
                            }
                        }
                    }
                } else {
                    projectChoiceCtrl->Disable();
                }
            }
        }
    }
}

void OutlookMeetingsViewFrame::OnAccountChoice(wxCommandEvent& event)
{
    wxBusyCursor cursor;

    mMeetingModels.clear();

    if (pActiveMeetingsPanel != nullptr) {
        RemoveActiveMeetingsPanel();
    }

    int selection = event.GetSelection();
    if (selection == 0) {
        ResetFeedbackLabelOnNoData();
        mSelectedAccount.clear();

        return;
    } else {
        mSelectedAccount = pAccountsChoiceCtrl->GetString(selection).ToStdString();
        if (!pRefreshButton->IsEnabled()) {
            pRefreshButton->Enable();
        }
        if (!pDatePickerCtrl->IsEnabled()) {
            pDatePickerCtrl->Enable();
        }
    }

    FetchOutlookMeetingsAndUpdateFeedbackLabel();

    std::vector<Model::AttendedMeetingModel> attendedMeetingModels = FetchAttendedMeetings();

    AddMeetingsToPanel(attendedMeetingModels);

    SetDialogSizeFromParent();
}

void OutlookMeetingsViewFrame::OnClose(wxCloseEvent& event)
{
    wxCommandEvent* cmdEvent = new wxCommandEvent(tksEVT_OUTLOOKMEETINGSFRMCLOSED);
    wxQueueEvent(pParent, cmdEvent);
    event.Skip();
}

void OutlookMeetingsViewFrame::OnProjectChoice(wxCommandEvent& event)
{
    SPDLOG_LOGGER_TRACE(pLogger, "Project choice selection: \"{0}\"", event.GetSelection());
    int projectIndex = event.GetSelection();

    wxChoice* choiceCtrl = dynamic_cast<wxChoice*>(event.GetEventObject());
    if (choiceCtrl) {
        ClientData<std::int64_t>* projectIdData =
            reinterpret_cast<ClientData<std::int64_t>*>(choiceCtrl->GetClientObject(projectIndex));
        std::int64_t projectId = projectIdData->GetValue();
        SPDLOG_LOGGER_TRACE(pLogger, "Selected project ID from choice ctrl \"{0}\"", projectId);

        wxWindowID choiceControlId = choiceCtrl->GetId();
        wxWindowID categoryChoiceCtrlId = -1;

        for (size_t i = 0; i < mControlChoicesData.size(); i++) {
            if (mControlChoicesData[i].ProjectChoiceControlId == choiceControlId) {
                SPDLOG_LOGGER_TRACE(pLogger,
                    "Updating project ID \"{0}\" of control choice ID \"{1}\" data struct",
                    projectId,
                    choiceControlId);
                mControlChoicesData[i].ProjectId = projectId;
                categoryChoiceCtrlId = mControlChoicesData[i].CategoryChoiceControlId;
                break;
            }
        }

        if (categoryChoiceCtrlId > 0) {
            wxWindow* windowCtrl = FindWindowById(categoryChoiceCtrlId);
            if (windowCtrl) {
                wxChoice* categoryChoiceCtrl = wxDynamicCast(windowCtrl, wxChoice);
                if (categoryChoiceCtrl) {
                    categoryChoiceCtrl->Clear();
                    categoryChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
                    categoryChoiceCtrl->SetSelection(0);

                    std::vector<Services::CategoryViewModel> categories;
                    Services::CategoryService categoryService(pLogger, mDatabaseFilePath);

                    auto sqliteResult = categoryService.FilterByProjectId(projectId, categories);

                    if (!sqliteResult.Success) {
                        wxRichMessageDialog dialog(this,
                            Messages::FilterCategoriesByProjectMessage,
                            tks::Common::GetProgramName(),
                            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
                        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
                        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

                        dialog.ShowModal();

                        return;
                    }

                    if (!categories.empty()) {
                        for (auto& category : categories) {
                            categoryChoiceCtrl->Append(category.GetFormattedName(),
                                new ClientData<std::int64_t>(category.CategoryId));
                        }
                    } else {
                        categoryChoiceCtrl->Disable();
                    }
                }
            }
        }
    }
}

void OutlookMeetingsViewFrame::OnCategoryChoice(wxCommandEvent& event)
{
    SPDLOG_LOGGER_TRACE(pLogger, "Category choice selection: \"{0}\"", event.GetSelection());
    int projectIndex = event.GetSelection();

    wxChoice* choiceCtrl = dynamic_cast<wxChoice*>(event.GetEventObject());
    if (choiceCtrl) {
        ClientData<std::int64_t>* categoryIdData =
            reinterpret_cast<ClientData<std::int64_t>*>(choiceCtrl->GetClientObject(projectIndex));
        std::int64_t categoryId = categoryIdData->GetValue();
        SPDLOG_LOGGER_TRACE(pLogger, "Selected category ID from choice ctrl \"{0}\"", categoryId);

        wxWindowID choiceControlId = choiceCtrl->GetId();

        for (size_t i = 0; i < mControlChoicesData.size(); i++) {
            if (mControlChoicesData[i].CategoryChoiceControlId == choiceControlId) {
                SPDLOG_LOGGER_TRACE(pLogger,
                    "Updating category ID \"{0}\" of control choice ID \"{1}\" data struct",
                    categoryId,
                    choiceControlId);
                mControlChoicesData[i].CategoryId = categoryId;
                break;
            }
        }
    }
}

void OutlookMeetingsViewFrame::OnAttendedCheckBoxCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Checkbox with ID: \"{0}\" checked", event.GetId());
        wxWindow* wnd = dynamic_cast<wxWindow*>(event.GetEventObject());
        wxStringClientData* scd = dynamic_cast<wxStringClientData*>(wnd->GetClientObject());

        SPDLOG_LOGGER_TRACE(pLogger, "Window ID \"{0}\"", wnd->GetId());
        wxWindowID checkboxId = event.GetId();

        if (!scd) {
            return;
        }
        auto& s = scd->GetData();
        auto eventEntryId = s.ToStdString();
        SPDLOG_LOGGER_TRACE(pLogger,
            "Checkbox with ID: \"{0}\" and ENTRY_ID -> \n{1}",
            event.GetId(),
            eventEntryId);

        const auto& foundMeetingIterator = std::find_if(mMeetingModels.begin(),
            mMeetingModels.end(),
            [=](const Services::Outlook::OutlookMeetingModel& model) {
                return model.EntryId == eventEntryId;
            });

        if (foundMeetingIterator != mMeetingModels.end()) {
            auto& meetingModel = *foundMeetingIterator;
            SPDLOG_LOGGER_TRACE(
                pLogger, "Meeting found with detail: \n{0}", meetingModel.DebugPrint());

            std::int64_t projectId = -1;
            std::int64_t categoryId = -1;

            auto controlDataIterator = std::find_if(mControlChoicesData.begin(),
                mControlChoicesData.end(),
                [=](const ControlChoiceData& cch) { return cch.CheckBoxControlId == checkboxId; });

            if (controlDataIterator != mControlChoicesData.end()) {
                ControlChoiceData cch = *controlDataIterator;
                projectId = cch.ProjectId;
                categoryId = cch.CategoryId;
            }

            dlg::TaskDialog meetingTaskDialog(pParent, pCfg, pLogger, mDatabaseFilePath);
            meetingTaskDialog.SetAttendedMeetingData(
                meetingModel.TrimmedSubject(), meetingModel.Duration, meetingModel.Location);
            meetingTaskDialog.SetAttendedMeetingDataEx(meetingModel.EntryId,
                meetingModel.TrimmedSubject(),
                meetingModel.Start,
                meetingModel.End,
                meetingModel.Duration,
                meetingModel.Location);
            if (projectId != -1 && categoryId != -1) {
                meetingTaskDialog.UpdateChoicesFromAttendedMeeting(
                    mEmployerId, projectId, categoryId);
            }

            int ret = meetingTaskDialog.ShowModal();

            wxCheckBox* attendedCheckBoxCtrl = wxDynamicCast(wnd, wxCheckBox);
            if (attendedCheckBoxCtrl) {
                if (ret != wxID_OK) {
                    attendedCheckBoxCtrl->SetValue(false);
                } else {
                    attendedCheckBoxCtrl->Disable();
                }
            }
        }
    } else {
        SPDLOG_LOGGER_TRACE(pLogger, "Checkbox with ID \"{0}\" unchecked", event.GetId());
    }
}

void OutlookMeetingsViewFrame::FetchOutlookMeetingsAndUpdateFeedbackLabel()
{
    SPDLOG_LOGGER_TRACE(pLogger,
        "Outlook account name selected \"{0}\"",
        mSelectedAccount.empty() ? "(none)" : mSelectedAccount);

    Services::Outlook::OutlookClassicService service(pLogger);
    Services::Outlook::OutlookResult result =
        service.FetchCalendarMeetings(mSelectedAccount, mSelectedDate, mMeetingModels);

    if (!result.Success) {
        wxMessageDialog dialog(this,
            "Failed to get Outlook accounts",
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(result.Message);

        dialog.ShowModal();

        pFeedbackLabel->SetLabel(result.Message);

        return;
    } else if (result.Success && !result.Message.empty()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Retrieved \"{0}\" meetings", mMeetingModels.size());

        if (mMeetingModels.size() == 0) {
            ResetFeedbackLabelOnNoData("No meetings found");

            return;
        }
    }

    if (pFeedbackLabel != nullptr) {
        pMainSizer->Detach(pFeedbackLabel);
        pFeedbackLabel->Destroy();
        pFeedbackLabel = nullptr;
        pMainSizer->Layout();
        SPDLOG_LOGGER_TRACE(pLogger, "Removed feedback static text from main sizer");
    }
}

std::vector<Model::AttendedMeetingModel> OutlookMeetingsViewFrame::FetchAttendedMeetings()
{
    Persistence::AttendedMeetingsPersistence attendedMeetingsPersistence(
        pLogger, mDatabaseFilePath);

    std::vector<Model::AttendedMeetingModel> attendedMeetingModels;
    auto sqliteResult =
        attendedMeetingsPersistence.GetByTodaysDate(Utils::UnixTimestampTodayMidnight(),
            Utils::UnixTimestampTomorrowMidnight(),
            attendedMeetingModels);

    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterAttendedMeetingsByTodayDateMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    }

    return attendedMeetingModels;
}

void OutlookMeetingsViewFrame::AddMeetingsToPanel(
    const std::vector<Model::AttendedMeetingModel>& attendedMeetingModels)
{
    std::vector<Model::ProjectModel> projectModels;
    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    if (mMeetingModels.size() != 0) {
        auto sqliteResult = projectPersistence.FilterByEmployerId(mEmployerId, projectModels);
        if (!sqliteResult.Success) {
            wxRichMessageDialog dialog(this,
                Messages::FilterProjectsMessage,
                tks::Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
            dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

            dialog.ShowModal();
        }
    }

    /* Panel Sizer */
    auto panelSizer = new wxBoxSizer(wxVERTICAL);

    /* Panel */
    pActiveMeetingsPanel = new wxPanel(pScrolledWindow, wxID_ANY);
    pActiveMeetingsPanel->SetSizer(panelSizer);

    int attendedCheckBoxControlId = tksIDC_ATTENDEDCHECKBOX_BASE;
    int projectChoiceControlId = tksIDC_PROJECTSCHOICECTRL_BASE;
    int categoryChoiceControlId = tksIDC_CATEGORIESCHOICECTRL_BASE;

    for (const auto& meetingModel : mMeetingModels) {
        bool meetingAttended = false;

        auto attendedMeetingFoundIterator = std::find_if(attendedMeetingModels.begin(),
            attendedMeetingModels.end(),
            [&](const Model::AttendedMeetingModel& model) {
                return model.EntryId == meetingModel.EntryId;
            });

        if (attendedMeetingFoundIterator != attendedMeetingModels.end()) {
            meetingAttended = true;
        }

        AddMeetingControlsToPanel(panelSizer,
            &attendedCheckBoxControlId,
            &projectChoiceControlId,
            &categoryChoiceControlId,
            meetingModel,
            meetingAttended,
            projectModels);

        ++attendedCheckBoxControlId;
        ++projectChoiceControlId;
        ++categoryChoiceControlId;
    }

    pScrolledWindowSizer->Add(pActiveMeetingsPanel, wxSizerFlags().Expand());
    pScrolledWindowSizer->SetSizeHints(pActiveMeetingsPanel);
    pScrolledWindowSizer->Layout();

    pMainSizer->Layout();
}

void OutlookMeetingsViewFrame::SetDialogSizeFromParent()
{
    if (!bIsMainFrameMaximized) {
        wxSize parentWindowSize = pParent->GetSize();
        SPDLOG_LOGGER_TRACE(pLogger,
            "Parent size ({0},{1})",
            parentWindowSize.GetHeight(),
            parentWindowSize.GetWidth());

        wxSize dialogMaxSize;
        dialogMaxSize.SetHeight(parentWindowSize.GetHeight());
        dialogMaxSize.SetWidth(-1);
        SetSize(dialogMaxSize);
    }
}

void OutlookMeetingsViewFrame::RemoveActiveMeetingsPanel()
{
    pScrolledWindowSizer->Detach(pActiveMeetingsPanel);
    bool windowDelete = pActiveMeetingsPanel->Destroy();
    if (!windowDelete) {
        pLogger->warn("Failed to delete active meetings panel and its child controls");
    }
    pActiveMeetingsPanel = nullptr;

    pScrolledWindowSizer->Layout();
    pMainSizer->Layout();

    SPDLOG_LOGGER_TRACE(pLogger, "Removed active meetings panel from scrolled window");
}

void OutlookMeetingsViewFrame::ResetFeedbackLabelOnNoData(const std::string& message)
{
    if (pFeedbackLabel == nullptr) {
        pFeedbackLabel = new wxStaticText(
            pThisPanel, tksIDC_FEEDBACKLABEL, message.empty() ? "No account selected" : message);
        // insert the feedback label above the scrolled window
        const int FeedbackLabelSizerIndex = 4;
        pMainSizer->Insert(FeedbackLabelSizerIndex,
            pFeedbackLabel,
            wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal().Top());

        pMainSizer->Layout();
    } else {
        pFeedbackLabel->SetLabel(message.empty() ? "No account selected" : message);
    }

    if (pRefreshButton->IsEnabled()) {
        pRefreshButton->Disable();
    }

    if (pDatePickerCtrl->IsEnabled()) {
        pDatePickerCtrl->Disable();
    }
}

void OutlookMeetingsViewFrame::AddMeetingControlsToPanel(wxBoxSizer* panelSizer,
    int* attendedCheckBoxControlId,
    int* projectChoiceControlId,
    int* categoryChoiceControlId,
    const Services::Outlook::OutlookMeetingModel& meetingModel,
    bool meetingAttended,
    const std::vector<Model::ProjectModel>& projectModels)
{
    ControlChoiceData choiceData;
    choiceData.CheckBoxControlId = *attendedCheckBoxControlId;
    choiceData.ProjectChoiceControlId = *projectChoiceControlId;
    choiceData.CategoryChoiceControlId = *categoryChoiceControlId;

    // static box for meeting controls
    auto staticBox = new wxStaticBox(pActiveMeetingsPanel, wxID_ANY, "");
    auto staticBoxSizer = new wxStaticBoxSizer(staticBox, wxVERTICAL);
    panelSizer->Add(staticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    auto flexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    flexGridSizer->AddGrowableCol(1, 1);
    staticBoxSizer->Add(flexGridSizer, wxSizerFlags().Expand().Proportion(1));

    if (pEnv->GetBuildConfiguration() == BuildConfiguration::Debug) {
        auto meetingIdLabel = new wxStaticText(staticBox, wxID_ANY, "Entry ID");
        auto providedInfoBitmap = wxArtProvider::GetBitmapBundle(
            wxART_INFORMATION, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
        auto meetingIdLabelValue = new wxStaticBitmap(staticBox, wxID_ANY, providedInfoBitmap);
        meetingIdLabelValue->SetToolTip(meetingModel.EntryId);

        flexGridSizer->Add(
            meetingIdLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
        flexGridSizer->Add(meetingIdLabelValue, wxSizerFlags().Border(wxALL, FromDIP(4)).Left());
    }

    auto subjectLabel = new wxStaticText(staticBox, wxID_ANY, "Subject");
    auto subjectText = new wxTextCtrl(
        staticBox, wxID_ANY, meetingModel.Subject, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

    auto durationWithTimeLabel = new wxStaticText(staticBox, wxID_ANY, "Duration");
    auto formattedValue = fmt::format(
        "{0} ({1} -- {2})", meetingModel.Duration, meetingModel.Start, meetingModel.End);
    auto durationWithTimeLabelValue = new wxTextCtrl(
        staticBox, wxID_ANY, formattedValue, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

    auto locationLabel = new wxStaticText(staticBox, wxID_ANY, "Location");
    auto locationLabelValue = new wxTextCtrl(staticBox,
        wxID_ANY,
        meetingModel.Location,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_READONLY);

    flexGridSizer->Add(subjectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    flexGridSizer->Add(subjectText, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    flexGridSizer->Add(
        durationWithTimeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    flexGridSizer->Add(
        durationWithTimeLabelValue, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    flexGridSizer->Add(locationLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    flexGridSizer->Add(locationLabelValue, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Horizontal line */
    auto line = new wxStaticLine(staticBox, wxID_ANY);
    staticBoxSizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Project choice ctrl */
    auto projectLabel = new wxStaticText(staticBox, wxID_ANY, "Project");
    auto projectChoiceCtrl = new wxChoice(staticBox, *projectChoiceControlId);
    projectChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    projectChoiceCtrl->SetSelection(0);

    projectChoiceCtrl->Bind(
        wxEVT_CHOICE, &OutlookMeetingsViewFrame::OnProjectChoice, this, *projectChoiceControlId);

    /* Category choice ctrl */
    auto categoryLabel = new wxStaticText(staticBox, wxID_ANY, "Category");
    auto categoryChoiceCtrl = new wxChoice(staticBox, *categoryChoiceControlId);
    categoryChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    categoryChoiceCtrl->SetSelection(0);

    categoryChoiceCtrl->Bind(
        wxEVT_CHOICE, &OutlookMeetingsViewFrame::OnCategoryChoice, this, *categoryChoiceControlId);

    if (!projectModels.empty()) {
        bool hasDefaultProject = false;
        std::int64_t defaultProjectId = -1;

        for (auto& project : projectModels) {
            projectChoiceCtrl->Append(
                project.DisplayName, new ClientData<std::int64_t>(project.ProjectId));

            if (project.IsDefault) {
                hasDefaultProject = true;
                defaultProjectId = project.ProjectId;
                choiceData.ProjectId = defaultProjectId;
                projectChoiceCtrl->SetStringSelection(project.DisplayName);
            }
        }

        if (hasDefaultProject) {
            std::vector<Services::CategoryViewModel> categories;
            Services::CategoryService categoryService(pLogger, mDatabaseFilePath);

            auto sqliteResult = categoryService.FilterByProjectId(defaultProjectId, categories);
            std::string operationMessage = Messages::FilterCategoriesByProjectMessage;

            if (!sqliteResult.Success) {
                wxRichMessageDialog dialog(this,
                    operationMessage,
                    tks::Common::GetProgramName(),
                    wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
                dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
                dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

                dialog.ShowModal();

                return;
            }

            if (!categories.empty()) {
                for (auto& category : categories) {
                    categoryChoiceCtrl->Append(category.GetFormattedName(),
                        new ClientData<std::int64_t>(category.CategoryId));
                }
            } else {
                categoryChoiceCtrl->Disable();
            }
        }
    } else {
        projectChoiceCtrl->Disable();
    }

    staticBoxSizer->Add(projectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    staticBoxSizer->Add(projectChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    staticBoxSizer->Add(categoryLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    staticBoxSizer->Add(categoryChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Horizontal line */
    auto line2 = new wxStaticLine(staticBox, wxID_ANY);
    staticBoxSizer->Add(line2, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Attended checkbox */
    auto attendedCheckBox = new wxCheckBox(staticBox, *attendedCheckBoxControlId, "Attended");
    wxStringClientData* meetingEntryIdData = new wxStringClientData(meetingModel.EntryId);
    attendedCheckBox->SetClientObject(meetingEntryIdData);

    attendedCheckBox->Bind(wxEVT_CHECKBOX,
        &OutlookMeetingsViewFrame::OnAttendedCheckBoxCheck,
        this,
        *attendedCheckBoxControlId);

    staticBoxSizer->Add(attendedCheckBox, wxSizerFlags().Border(wxALL, FromDIP(4)).Right());

    if (meetingAttended) {
        attendedCheckBox->SetValue(true);
        attendedCheckBox->Disable();
    }

    mControlChoicesData.push_back(choiceData);
}
} // namespace tks::UI::frames
