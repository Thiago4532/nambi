cmake_minimum_required(VERSION 3.0.0)
project(soinga)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/spawn.cpp
    src/main.cpp
)

find_package(fmt)

add_executable(soinga ${SOURCES})
target_include_directories(soinga
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
# target_link_libraries(soinga
# )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)

