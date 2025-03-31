cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_COMMON
    "${CMAKE_CURRENT_SOURCE_DIR}/common.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/enums.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/constants.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/validator.cpp"

    PARENT_SCOPE
)
