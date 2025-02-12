cmake_minimum_required (VERSION 3.22)
project ("Taskies")

message(STATUS "LOCATING PACKAGES")

find_package(wxWidgets CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

message (STATUS "wxWidgets_FOUND: ${wxWidgets_FOUND}")
message (STATUS "unofficial-sqlite3_FOUND found: ${unofficial-sqlite3_FOUND}")
message (STATUS "ZLIB found: ${ZLIB_FOUND}")
message (STATUS "spdlog found: ${spdlog_FOUND}")
message (STATUS "date found: ${date_FOUND}")
message (STATUS "toml11 found: ${toml11_FOUND}")
message (STATUS "nlohmann_json found: ${nlohmann_json_FOUND}")
message (STATUS "fmt found: ${fmt_FOUND}")

set (SRC_MODELS
    "models/employermodel.cpp"
    "models/clientmodel.cpp"
    "models/projectmodel.cpp"
    "models/categorymodel.cpp"
    "models/workdaymodel.cpp"
    "models/taskmodel.cpp"
)

set(SRC_PERSISTENCE
    "persistence/employerpersistence.cpp"
    "persistence/clientpersistence.cpp"
    "persistence/projectpersistence.cpp"
    "persistence/categorypersistence.cpp"
    "persistence/workdaypersistence.cpp"
    "persistence/taskpersistence.cpp"
    "persistence/exportpersistence.cpp"
)

set(SRC_REPOS
    "repository/taskrepository.cpp"
    "repository/taskrepositorymodel.cpp"
    "repository/categoryrepositorymodel.cpp"
    "repository/categoryrepository.cpp"
    "repository/setupwizardrepository.cpp"
)

set (SRC_DIALOGS
    "ui/dlg/employerdlg.cpp"
    "ui/dlg/errordlg.cpp"
    "ui/dlg/editlistdlg.cpp"
    "ui/dlg/clientdlg.cpp"
    "ui/dlg/projectdlg.cpp"
    "ui/dlg/categoriesdlg.cpp"
    "ui/dlg/categorydlg.cpp"
    "ui/dlg/aboutdlg.cpp"
    "ui/dlg/preferencesdlg.cpp"
    "ui/dlg/preferencesgeneralpage.cpp"
    "ui/dlg/preferencesdatabasepage.cpp"
    "ui/dlg/preferencestaskspage.cpp"
    "ui/dlg/preferencestasksviewpage.cpp"
    "ui/dlg/taskdlglegacy.cpp"
    "ui/dlg/taskdlg.cpp"
    "ui/dlg/daytaskviewdlg.cpp"
    "ui/wizards/setupwizard.cpp"
    "ui/dlg/exporttocsvdlg.cpp"
    "ui/dlg/preferencesexportpage.cpp"
    "ui/dlg/quickexporttocsvdlg.cpp"
)

set (SRC
    "main.cpp"
    "application.cpp"
    "utils/utils.cpp"
    "utils/datestore.cpp"
    "core/environment.cpp"
    "core/configuration.cpp"
    "core/database_migration.cpp"
    "ui/persistencemanager.cpp"
    "ui/translator.cpp"
    "ui/mainframe.cpp"
    "ui/events.cpp"
    "ui/clientdata.cpp"
    "ui/notificationclientdata.cpp"
    "ui/taskbaricon.cpp"
    "ui/notificationpopupwindow.cpp"
    "ui/dataview/tasktreemodel.cpp"
    "ui/dataview/tasktreemodelnode.cpp"
    "ui/dataview/tasklistmodel.cpp"
    "ui/dataview/columnlistmodel.cpp"
    "ui/dataview/columnlistitemmodel.cpp"
    "ui/statusbar.cpp"
    "resources.rc"
    "application.manifest"
    "common/common.cpp"
    "common/enums.cpp"
    "common/constants.cpp"
    "common/validator.cpp"
    "ipc/applicationoptionsconnection.cpp"
    "ipc/server.cpp"
    "services/export/projectionbuilder.cpp"
    "services/export/availablecolumns.cpp"
    "services/export/columnprojection.cpp"
    "services/export/projection.cpp"
    "services/export/columnjoinprojection.cpp"
    "services/export/sqliteexportquerybuilder.cpp"
    "services/export/csvexportoptions.cpp"
    "services/export/csvexportprocessor.cpp"
    "services/export/csvexporter.cpp"
    "services/export/csvexportmappedoptions.cpp"
    "services/export/columnexportmodel.cpp"
    "services/taskduration/taskdurationservice.cpp"
    ${SRC_MODELS}
    ${SRC_PERSISTENCE}
    ${SRC_REPOS}
    ${SRC_DIALOGS}
)

add_executable (
    ${PROJECT_NAME}
    WIN32
    ${SRC}
)

target_compile_options (
    ${PROJECT_NAME}
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /permissive- /TP /EHsc>
)

target_compile_features (
    ${PROJECT_NAME}
    PRIVATE
    cxx_std_17
)

target_compile_definitions (
    ${PROJECT_NAME}
    PUBLIC
    _CRT_SECURE_NO_WARNINGS
    _UNICODE
    UNICODE
    WXUSINGDLL
    wxUSE_GUI=1
    wxUSE_DATEPICKCTRL=1
    wxNO_UNSAFE_WXSTRING_CONV=1
    __WXMSW__
    $<$<CONFIG:Debug>:TKS_DEBUG>
    $<$<CONFIG:Debug>:WXDEBUG>
)

target_link_libraries (${PROJECT_NAME} PRIVATE
    wx::core wx::base
    unofficial::sqlite3::sqlite3
    ZLIB::ZLIB
    spdlog::spdlog
    date::date date::date-tz
    toml11::toml11
    nlohmann_json::nlohmann_json
    fmt::fmt
)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/lang/en-US.json
        "${CMAKE_BINARY_DIR}"
    COMMAND_EXPAND_LISTS
)

#add_custom_command(
#    TARGET ${PROJECT_NAME}
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#        ${CMAKE_SOURCE_DIR}/lang/en-ZA.json
#        "${CMAKE_BINARY_DIR}"
#    COMMAND_EXPAND_LISTS
#)
