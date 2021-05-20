project(nginx)
cmake_minimum_required (VERSION 3.5)

MESSAGE(STATUS "This is SOURCE dir " ${PROJECT_SOURCE_DIR})
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
#set(SOURCES ${SOURCES})
add_executable(${PROJECT_NAME} ${SOURCES})

file(GLOB libs "${CMAKE_SOURCE_DIR}/lib/*.so")
MESSAGE(STATUS "This is SOURCE dir(app) " ${CMAKE_SOURCE_DIR}/lib)
#target_link_libraries(${PROJECT_NAME} sub::signal)
target_link_libraries(${PROJECT_NAME} ${libs})

#target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/signal)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/proc)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/net)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})