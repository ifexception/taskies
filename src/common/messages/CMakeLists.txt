cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_COMMON_MESSAGES
    "${CMAKE_CURRENT_SOURCE_DIR}/persistencemessages.cpp"
    PARENT_SCOPE
)
