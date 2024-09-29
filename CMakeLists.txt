cmake_minimum_required(VERSION 3.20)

project(MyCrowProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent module to download dependencies
include(FetchContent)

# CrowCpp
FetchContent_Declare(
  CrowCpp
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG master 
)
FetchContent_MakeAvailable(CrowCpp)

# SqliteModernCpp
FetchContent_Declare(
  SqliteModernCpp
  GIT_REPOSITORY https://github.com/SqliteModernCpp/sqlite_modern_cpp.git
  GIT_TAG master  
)
FetchContent_MakeAvailable(SqliteModernCpp)

add_executable(${PROJECT_NAME} account.cpp)

# Link CrowCpp and SqliteModernCpp
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Crow::Crow
    sqlite_modern_cpp
)

# Include directories for SqliteModernCpp (if needed)
target_include_directories(${PROJECT_NAME} PRIVATE ${SqliteModernCpp_SOURCE_DIR})
