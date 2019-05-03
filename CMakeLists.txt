cmake_minimum_required(VERSION 3.12)

set(This HsContainers)

project (${This})

set(CMAKE_CONFIGURATION_TYPES "Release;Debug")

set(CMAKE_CXX_STANDARD 14)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(googletest)

include_directories("Containers/include" "Tests/include")
include_directories(${gtest_SOURCE_DIR}/include)

set (TESTS_SOURCES
    Tests/src/TestMain.cpp
)

set (CONTAINER_HEADERS
Containers/include/HashSet.h
Containers/include/LinearProbingHashSet.h
)

source_group(Tests FILES ${TESTS_SOURCES})
source_group(Containers FILES ${CONTAINER_HEADERS})

add_executable (Tests ${TESTS_SOURCES} ${CONTAINER_HEADERS})

add_dependencies(Tests gtest_main)

target_link_libraries(
    Tests PUBLIC
    gtest_main
)

include(GoogleTest)

enable_testing()
gtest_discover_tests(Tests)
