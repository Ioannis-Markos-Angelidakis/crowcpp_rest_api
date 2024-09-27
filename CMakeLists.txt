cmake_minimum_required(VERSION 3.20)

# Project details
project(MyCrowProject LANGUAGES CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent module to download dependencies
include(FetchContent)

# CrowCpp
FetchContent_Declare(
  CrowCpp
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG master  # You can specify a version or commit
)
FetchContent_MakeAvailable(CrowCpp)

# SqliteModernCpp
FetchContent_Declare(
  SqliteModernCpp
  GIT_REPOSITORY https://github.com/SqliteModernCpp/sqlite_modern_cpp.git
  GIT_TAG master  # You can specify a version or commit
)
FetchContent_MakeAvailable(SqliteModernCpp)

# Add your executable
add_executable(${PROJECT_NAME} main.cpp)

# Link CrowCpp and SqliteModernCpp
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Crow::Crow
    sqlite_modern_cpp
)

# Include directories for SqliteModernCpp (if needed)
target_include_directories(${PROJECT_NAME} PRIVATE ${SqliteModernCpp_SOURCE_DIR})
