cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_SERVICE_STATICATTRIBUTEGROUPS
    "${CMAKE_CURRENT_SOURCE_DIR}/staticattributegroupviewmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/staticattributegroupsservice.cpp"
    PARENT_SCOPE
)
