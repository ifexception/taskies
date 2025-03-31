cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("dlg")
add_subdirectory("wizards")

set(
    SRC_UI
    ${SRC_DLG}
    ${SRC_WIZARDS}
    PARENT_SCOPE
)
