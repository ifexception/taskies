cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_PERSISTENCE
    "${CMAKE_CURRENT_SOURCE_DIR}/attributegroupspersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/attributetypespersistence.cpp"
    PARENT_SCOPE
)
