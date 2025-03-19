cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_MODELS
    "${CMAKE_CURRENT_SOURCE_DIR}/attributegroupmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/attributetypemodel.cpp"
    PARENT_SCOPE
)
