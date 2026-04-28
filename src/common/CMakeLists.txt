cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("results")
add_subdirectory("messages")

set(
    SRC_COMMON
    "${CMAKE_CURRENT_SOURCE_DIR}/common.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/wxcommon.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/enums.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/constants.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/validator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/logmessages.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/queryhelper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/enumclientdata.cpp"
    ${SRC_COMMON_RESULTS}
    ${SRC_COMMON_MESSAGES}
    PARENT_SCOPE
)
