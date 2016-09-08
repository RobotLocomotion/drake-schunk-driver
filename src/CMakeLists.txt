cmake_minimum_required(VERSION 2.8.5)
project(schunk-driver)

include (../externals/cmake/pods.cmake)
pods_find_pkg_config(lcm)

include (../externals/cmake/lcmtypes.cmake)
add_lcmtype(lcmt_schunk_command.lcm)
add_lcmtype(lcmt_schunk_status.lcm)

#     lcmtypes_build([C_AGGREGATE_HEADER header_fname]
#                    [C_LIBNAME lib_name]
#		     [CPP_AGGREGATE_HEADER header_fname]
#                    [JARNAME jar_name])

lcmtypes_build()

set(wsg_files)
list(APPEND wsg_files
  wsg_command_message.cc wsg_command_sender.cc
  wsg_return_message.cc wsg_return_receiver.cc
  position_force_control.cc
  position_control.cc)

add_executable(demo demo.cc ${wsg_files})
target_compile_options(demo PUBLIC -std=c++14)

add_executable(pfdemo position_force_demo.cc ${wsg_files})
target_compile_options(pfdemo PUBLIC -std=c++14)

add_executable(schunk_driver schunk_driver.cc ${wsg_files})
target_link_libraries(schunk_driver pthread)
target_compile_options(schunk_driver PUBLIC -std=c++14)
add_dependencies(schunk_driver schunk_driver_lcmtypes)
pods_use_pkg_config_packages(schunk_driver lcm)
