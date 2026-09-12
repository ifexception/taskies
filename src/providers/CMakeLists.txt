cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("projectbillablehours")
add_subdirectory("taskhours")

set(
    SRC_PROVIDERS
    ${SRC_PROVIDER_PROJECTBILLABLEHOURS}
    ${SRC_PROVIDER_TASKHOURS}
    PARENT_SCOPE
)
