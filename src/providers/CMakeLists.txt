cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("projectbillablehours")

set(
    SRC_PROVIDERS
    ${SRC_PROVIDER_PROJECTBILLABLEHOURS}
    PARENT_SCOPE
)
