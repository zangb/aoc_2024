cmake_minimum_required(VERSION 3.29)
project("third" LANGUAGES CXX)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
