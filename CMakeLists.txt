cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(SCPRoomGen)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glaze")

set(SCPROOMGEN_SRC
    ${CMAKE_CURRENT_LIST_DIR}/inc/generator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/generator.cpp

    ${CMAKE_CURRENT_LIST_DIR}/inc/testdata.h
    ${CMAKE_CURRENT_LIST_DIR}/src/testdata.cpp


    ${CMAKE_CURRENT_LIST_DIR}/src/test.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)

set(TEST_RESOURCES_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/testdata/")
configure_file("${CMAKE_CURRENT_LIST_DIR}/inc/config.h.in" "${CMAKE_CURRENT_LIST_DIR}/inc/config.h")


add_executable(${PROJECT_NAME} ${SCPROOMGEN_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)
target_link_libraries(${PROJECT_NAME} PUBLIC glaze::glaze)
target_link_libraries(${PROJECT_NAME} PUBLIC gtest_main glaze::glaze)

#include(GoogleTest)
#gtest_discover_tests(${PROJECT_NAME})