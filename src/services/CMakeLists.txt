cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("attributes")
add_subdirectory("categories")
add_subdirectory("export")
add_subdirectory("setupwizard")
add_subdirectory("taskduration")
add_subdirectory("tasks")

set(
    SRC_SERVICES
    ${SRC_SERVICE_CATEGORY}
    ${SRC_SERVICE_SETUPWIZARD}
    ${SRC_SERVICE_EXPORT}
    ${SRC_SERVICE_TASKS}
    ${SRC_SERVICE_STATICATTRIBUTEGROUPS}
    ${SRC_SERVICE_TASKDURATION}
    PARENT_SCOPE
)
