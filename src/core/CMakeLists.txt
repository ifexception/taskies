cmake_minimum_required (VERSION 3.22)
project ("Taskies")

set(
    SRC_CORE
    "${CMAKE_CURRENT_SOURCE_DIR}/configuration.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/database_migration.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/database_backup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/database_optimizer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/environment.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/zip_database_backup.cpp"
    PARENT_SCOPE
)
