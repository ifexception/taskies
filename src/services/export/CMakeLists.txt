cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_SERVICE_EXPORT
    "${CMAKE_CURRENT_SOURCE_DIR}/projectionbuilder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/availablecolumns.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/columnprojection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/projection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/columnjoinprojection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/sqliteexportquerybuilder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/csvexportoptions.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/csvexportprocessor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/csvexporter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/csvexportmappedoptions.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/columnexportmodel.cpp"
    PARENT_SCOPE
)
