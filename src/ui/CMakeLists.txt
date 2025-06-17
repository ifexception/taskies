cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("common")
add_subdirectory("dataview")
add_subdirectory("dlg")
add_subdirectory("wizards")

set(
    SRC_UI
    "${CMAKE_CURRENT_SOURCE_DIR}/persistencemanager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/translator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/mainframe.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/events.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/taskbaricon.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/notificationpopupwindow.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/statusbar.cpp"
    ${SRC_UI_COMMON}
    ${SRC_UI_DATAVIEW}
    ${SRC_DLG}
    ${SRC_WIZARDS}
    PARENT_SCOPE
)
