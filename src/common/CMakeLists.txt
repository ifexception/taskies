cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_COMMON
    "${CMAKE_CURRENT_SOURCE_DIR}/common.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/enums.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/constants.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/validator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/logmessages.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/queryhelper.cpp"
    PARENT_SCOPE
)
