cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_PROVIDER_TASKHOURS
    "${CMAKE_CURRENT_SOURCE_DIR}/taskhoursprovider.cpp"
    PARENT_SCOPE
)
