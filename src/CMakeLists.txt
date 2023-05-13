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
)

set (SRC_DATA
    "data/employerdata.cpp"
    "data/clientdata.cpp"
    "data/projectdata.cpp"
    "data/categorydata.cpp"
)

set (SRC_DIALOGS
    "ui/dlg/employerdlg.cpp"
    "ui/dlg/errordlg.cpp"
    "ui/dlg/editlistdlg.cpp"
    "ui/dlg/clientdlg.cpp"
    "ui/dlg/projectdlg.cpp"
    "ui/dlg/categoriesdlg.cpp"
)

set (SRC
    "main.cpp"
    "application.cpp"
    "utils/utils.cpp"
    "core/environment.cpp"
    "core/configuration.cpp"
    "core/database_migration.cpp"
    "ui/persistencemanager.cpp"
    "ui/translator.cpp"
    "ui/mainframe.cpp"
    "ui/events.cpp"
    "resources.rc"
    "common/common.cpp"
    "common/enums.cpp"
    "common/constants.cpp"
    ${SRC_MODELS}
    ${SRC_DATA}
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
    wxUSE_TIMEPICKCTRL=1
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
