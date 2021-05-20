project (net)
cmake_minimum_required(VERSION 3.5)

MESSAGE(STATUS "This is SOURCE dir(proc): " ${PROJECT_SOURCE_DIR})
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES})
add_library(sub::net ALIAS ${PROJECT_NAME})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib) 
target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/app)