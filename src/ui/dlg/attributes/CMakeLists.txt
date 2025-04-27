cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_ATTR_DLG
    "${CMAKE_CURRENT_SOURCE_DIR}/attributegroupdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/attributedlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/staticattributevaluesdlg.cpp"
    PARENT_SCOPE
)
