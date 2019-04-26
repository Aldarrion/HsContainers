cmake_minimum_required(VERSION 3.12)
project (HsContainers)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug")

include_directories("Containers/include" "Tests/include")

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
