cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_MODELS
    "${CMAKE_CURRENT_SOURCE_DIR}/employermodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/clientmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/projectmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/categorymodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/workdaymodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/taskmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/attributegroupmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/attributetypemodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/attributemodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/taskattributevaluemodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/staticattributevaluemodel.cpp"
    PARENT_SCOPE
)
