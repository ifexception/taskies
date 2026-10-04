cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_PERSISTENCEBASE
    "${CMAKE_CURRENT_SOURCE_DIR}/persistencebase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/db_deleter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/stmt_deleter.cpp"
    PARENT_SCOPE
)
