cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_SERVICE_TASKDURATION
    "${CMAKE_CURRENT_SOURCE_DIR}/taskdurationviewmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/taskdurationservice.cpp"
    PARENT_SCOPE
)
