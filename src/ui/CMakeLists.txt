cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("dlg")

set(
    SRC_UI
    ${SRC_DLG}
    PARENT_SCOPE
)
