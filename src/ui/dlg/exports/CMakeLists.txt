cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_EXPORT_DIALOGS
    "${CMAKE_CURRENT_SOURCE_DIR}/exporttocsvdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/quickexporttocsvdlg.cpp"
    PARENT_SCOPE
)
