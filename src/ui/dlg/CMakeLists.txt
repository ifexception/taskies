cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("attributes")
add_subdirectory("preferences")
add_subdirectory("exports")

set (
    SRC_DIALOGS
    "${CMAKE_CURRENT_SOURCE_DIR}/aboutdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/employerdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/clientdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/projectdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/categoriesdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/categorydlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/taskdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/taskdlglegacy.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/taskmanageattributesdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/editlistdlg.cpp"
)

set (
    SRC_DLG
    ${SRC_ATTR_DLG}
    ${SRC_PREFERENCES_DLG}
    ${SRC_EXPORT_DIALOGS}
    ${SRC_DIALOGS}
    PARENT_SCOPE
)
