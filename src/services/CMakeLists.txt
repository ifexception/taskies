cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("category")
add_subdirectory("setupwizard")

set(
    SRC_SERVICES
    ${SRC_SERVICE_CATEGORY}
    ${SRC_SERVICE_SETUPWIZ}
    PARENT_SCOPE
)
