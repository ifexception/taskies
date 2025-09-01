cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_EXPORT_DIALOGS
    "${CMAKE_CURRENT_SOURCE_DIR}/exporttocsvdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exporttoexceldlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/quickexporttoformatdlg.cpp"
    PARENT_SCOPE
)
