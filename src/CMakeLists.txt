cmake_minimum_required (VERSION 3.22)
project ("Taskies")

add_subdirectory("common")
message (STATUS "add subdirectory common")

add_subdirectory("core")
message (STATUS "add subdirectory core")

add_subdirectory("ipc")
message (STATUS "add subdirectory ipc")

add_subdirectory("ui")
message (STATUS "add subdirectory ui")

add_subdirectory("models")
message (STATUS "add subdirectory models")

add_subdirectory("persistence")
message (STATUS "add subdirectory persistence")

add_subdirectory("services")
message (STATUS "add subdirectory services")

add_subdirectory("utils")
message (STATUS "add subdirectory utils")

set (SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/application.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc"
    "${CMAKE_CURRENT_SOURCE_DIR}/application.manifest"
    ${SRC_COMMON}
    ${SRC_CORE}
    ${SRC_IPC}
    ${SRC_MODELS}
    ${SRC_PERSISTENCE}
    ${SRC_DIALOGS}
    ${SRC_EXPORTS}
    ${SRC_UI}
    ${SRC_SERVICES}
    ${SRC_UTILS}
    PARENT_SCOPE
)
