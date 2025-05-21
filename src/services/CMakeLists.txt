cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("categories")
add_subdirectory("setupwizard")
add_subdirectory("tasks")

set(
    SRC_SERVICES
    ${SRC_SERVICE_CATEGORY}
    ${SRC_SERVICE_SETUPWIZ}
    ${SRC_SERVICE_TASKS}
    PARENT_SCOPE
)
