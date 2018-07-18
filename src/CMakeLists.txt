
set(HEADERS
	crc.h
    defaults.h
    position_force_control.h
    wsg.h
    wsg_command_message.h
    wsg_command_sender.h
    wsg_return_message.h
    wsg_return_receiver.h
    )

set(SOURCES
    position_force_control.cc
    wsg_command_message.cc
    wsg_command_sender.cc
    wsg_return_message.cc
    wsg_return_receiver.cc
	)

include_directories("${HEADERS}")

add_library(schunk_driver_library
	"${SOURCES}")



# target_include_directories(schunk_driver_library
# 	PUBLIC "${HEADERS}")

add_executable(schunk_driver_node schunk_driver_ros.cc)
add_dependencies(schunk_driver_node ${catkin_EXPORTED_TARGETS})
target_link_libraries(schunk_driver_node
	schunk_driver_library
	${catkin_LIBRARIES})

# Mark executables and/or libraries for installation
install(TARGETS schunk_driver_library schunk_driver_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

