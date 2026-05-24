cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_WIZARDS
    "${CMAKE_CURRENT_SOURCE_DIR}/setupwizard.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/portablesetupwizard.cpp"

    PARENT_SCOPE
)
