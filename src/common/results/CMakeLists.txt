cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_COMMON_RESULTS
    "${CMAKE_CURRENT_SOURCE_DIR}/sqliteresult.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exportresult.cpp"
    PARENT_SCOPE
)
