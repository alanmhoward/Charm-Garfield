#---Setup the example project---#
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(CHARM)

#---Find the Garfield package---#
find_package(Garfield REQUIRED)

#---Define variables---#
add_executable(CHARM CHARM.C)
target_link_libraries(CHARM Garfield)

#Copy local files to the build directory---#
foreach(_file ar-80_co2-20_1000mbar.gas he3_cf4.gas 3He_CF4_8bar.gas)
	configure_file(${_file} ${_file} ${_file} COPYONLY)
endforeach()
