cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_SERVICE_TASKS
    "${CMAKE_CURRENT_SOURCE_DIR}/taskviewmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tasksservice.cpp"
    PARENT_SCOPE
)
