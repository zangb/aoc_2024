cmake_minimum_required(VERSION 3.29)
project("sixth" LANGUAGES CXX)
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(TBB REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)
target_compile_options(${PROJECT_NAME} PRIVATE -std=c++20)
