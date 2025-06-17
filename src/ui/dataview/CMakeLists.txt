cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_UI_DATAVIEW
    "${CMAKE_CURRENT_SOURCE_DIR}/tasktreemodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tasktreemodelnode.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tasklistmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/columnlistmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/columnlistitemmodel.cpp"
    PARENT_SCOPE
)
