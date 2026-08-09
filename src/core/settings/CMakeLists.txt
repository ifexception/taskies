cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_CORE_SETTINGS
    "${CMAKE_CURRENT_SOURCE_DIR}/tasksviewcolumnsetting.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/presetcolumnsetting.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/presetsetting.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/settings.cpp"
    PARENT_SCOPE
)
