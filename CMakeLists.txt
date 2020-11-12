#---Setup the example project---#
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(CHARM)

#---Find the Garfield package---#
find_package(Garfield REQUIRED)

#---Define variables---#
add_executable(CHARM CHARM.C)
target_link_libraries(CHARM Garfield)

