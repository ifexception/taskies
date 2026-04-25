cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_COMMON_MESSAGES
    "${CMAKE_CURRENT_SOURCE_DIR}/configmessages.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/operationmessages.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/persistencemessages.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/sqlitemessages.cpp"
    PARENT_SCOPE
)
