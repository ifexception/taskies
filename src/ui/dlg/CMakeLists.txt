cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("attributes")

set (
    SRC_DLG
    ${SRC_ATTR_DLG}
    PARENT_SCOPE
)
