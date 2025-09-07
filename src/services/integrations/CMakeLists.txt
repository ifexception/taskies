cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_SERVICE_INTEGRATIONS
    "${CMAKE_CURRENT_SOURCE_DIR}/outlookintegratorservice.cpp"
    PARENT_SCOPE
)
