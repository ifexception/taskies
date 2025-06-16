cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_IPC
    "${CMAKE_CURRENT_SOURCE_DIR}/applicationoptionsconnection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/server.cpp"
    PARENT_SCOPE
)
