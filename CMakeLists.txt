cmake_minimum_required(VERSION 3.16)
find_program(CCACHE_PROGRAM ccache)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -pedantic -fopenmp -g -O0 -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -pedantic -g -O3")


project(relationscutter)

execute_process(COMMAND bash "-c" "git submodule update --init --recursive --remote --no-fetch"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/lib/lemon/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
set(LEMON_SOURCE_ROOT_DIR lib/lemon)
set(LEMON_ENABLE_COIN NO)
set(LEMON_ENABLE_ILOG NO)
set(LEMON_ENABLE_GLPK NO)
set(LEMON_ENABLE_SOPLEX NO)
add_subdirectory(${LEMON_SOURCE_ROOT_DIR} deps/lemon)

set(LEMON_INCLUDE_DIRS
        ${LEMON_SOURCE_ROOT_DIR}
        ${CMAKE_BINARY_DIR}/deps/lemon
        )
set(LEMON_LIBRARIES lemon)

add_subdirectory(lib/cxxopts)
add_subdirectory(lib/spdlog)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp lib/* *.cc)
string(REGEX REPLACE "src/main.cpp" "" SOURCES "${SOURCES}")
set(TARGET_LIBS lemon cxxopts spdlog)

include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_BINARY_DIR}
        ${LEMON_INCLUDE_DIRS}
)
add_executable(${PROJECT_NAME} ${SOURCES} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${TARGET_LIBS})