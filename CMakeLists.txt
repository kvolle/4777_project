cmake_minimum_required(VERSION 2.8)
# Project Name
PROJECT(simulation)

#########################################################
# Include Files
#########################################################
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
file(GLOB exec_source "src/*.cpp" "include/*.h*")
add_executable(main src/main.cpp src/Agent.cpp src/Target.cpp src/Simulation.cpp)
#add_executable(main ${exec_source})
########################################################
# Linking & stuff
#########################################################
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE C COMPILER mpicc)
set(CMAKE CXX COMPILER mpicxx)
set(CMAKE_CXX_FLAGS "-std=c++0x")
set(CMAKE_MODULE_PATH "PATH_TO/CMakeModules;${CMAKE_MODULE_PATH}")

target_link_libraries(main ${MPI_LIBRARIES})

#if(MPI_COMPILE_FLAGS)
#  set_target_properties(hello PROPERTIES
#    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
#endif()

#if(MPI_LINK_FLAGS)
#  set_target_properties(hello PROPERTIES
#    LINK_FLAGS "${MPI_LINK_FLAGS}")
#endif()
