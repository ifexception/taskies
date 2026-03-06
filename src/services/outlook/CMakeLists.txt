cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_SERVICE_OUTLOOK
    "${CMAKE_CURRENT_SOURCE_DIR}/outlookclassicservice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/outlookmeetingmodel.cpp"
    PARENT_SCOPE
)
