cmake_minimum_required(VERSION 3.29)
project("fourth" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
