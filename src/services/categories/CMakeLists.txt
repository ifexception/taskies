cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_SERVICE_CATEGORY
    "${CMAKE_CURRENT_SOURCE_DIR}/categoryviewmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/categoryservice.cpp"
    PARENT_SCOPE
)
