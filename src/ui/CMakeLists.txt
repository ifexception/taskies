cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("common")
add_subdirectory("dataview")
add_subdirectory("dlg")
add_subdirectory("wizards")

set(
    SRC_UI
    ${SRC_UI_COMMON}
    ${SRC_UI_DATAVIEW}
    ${SRC_DLG}
    ${SRC_WIZARDS}
    PARENT_SCOPE
)
