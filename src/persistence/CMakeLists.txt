cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_PERSISTENCE
    "${CMAKE_CURRENT_SOURCE_DIR}/employerspersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/clientspersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/projectspersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/categoriespersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/workdayspersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/taskspersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exportpersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/attributegroupspersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/attributetypespersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/attributespersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/taskattributevaluespersistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/staticattributevaluespersistence.cpp"
    PARENT_SCOPE
)
