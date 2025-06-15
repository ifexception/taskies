cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_PREFERENCES_DLG
    "${CMAKE_CURRENT_SOURCE_DIR}/preferencesdlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/preferencesgeneralpage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/preferencesdatabasepage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/preferencestaskspage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/preferencestasksviewpage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/preferencesexportpage.cpp"

    PARENT_SCOPE
)
