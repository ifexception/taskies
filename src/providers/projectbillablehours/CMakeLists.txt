cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_PROVIDER_PROJECTBILLABLEHOURS
    "${CMAKE_CURRENT_SOURCE_DIR}/projectbillablehoursprovider.cpp"
    PARENT_SCOPE
)
