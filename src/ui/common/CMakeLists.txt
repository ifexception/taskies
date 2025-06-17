cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_UI_COMMON
    "${CMAKE_CURRENT_SOURCE_DIR}/taskattributevalueclientdata.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/notificationclientdata.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/clientdata.cpp"
    PARENT_SCOPE
)
