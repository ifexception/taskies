cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_UTILS
    "${CMAKE_CURRENT_SOURCE_DIR}/datestore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/mswutils.cpp"
    PARENT_SCOPE
)
