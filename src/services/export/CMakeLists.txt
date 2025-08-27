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
    "${CMAKE_CURRENT_SOURCE_DIR}/exportoptions.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/csvexportprocessor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/csvexporterservice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/csvexportmappedoptions.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/columnexportmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/headervaluepair.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exportsservice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/row.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/excelexporterservice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/datagenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/excelexportprocessor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/excelinstancecheck.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exportresult.cpp"
    PARENT_SCOPE
)
