project(sensors_plugin)

cmake_minimum_required(VERSION 2.8)

#Input Options
option(SENSORS_STATIC "Link libsensors statically." ON)
option(SENSORS_DIR "Path to libsensors directory" "")
option(SENSORS_INC "Path to libsensors headers" "")
option(SENSORS_LIB "Path to libsensors" "")

#additional c flags
set(CMAKE_C_FLAGS "-D_GNU_SOURCE -g -std=gnu99 -pthread")

#debugging c flags
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wstrict-prototypes -Wall -Wundef -Wno-long-long -Wsign-compare -Wcomment -pedantic -finline-functions -fno-strict-aliasing")
set(CMAKE_C_FLAGS_RELEASE "-Os")

include(common/FindScorep.cmake)
if(SCOREP_FOUND)
    include_directories(${SCOREP_INCLUDE_DIRS})    
    link_directories(${SCOREP_LIBRARY_DIRS})
    SET(CMAKE_INSTALL_PREFIX ${SCOREP_ROOT_PATH} CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
else()
    message(SEND_ERROR "Scorep was not found but is required!")
endif()

# find libsensors
find_path(SENSORS_INC_DIR sensors/sensors.h HINTS ${SENSORS_INC} ${SENSORS_DIR}/include DOC "Path to sensors/sensors.h")
if( SENSORS_INC_DIR )
	if(SENSORS_STATIC)
		message("Using static libsensors")
		message("Linking might fail if you use the static version of libsensors and your library is not compiled with -fPIC. In this case use the cmake flag -DSENSORS_STATIC=OFF")
		set(LIBSENSORS_NAME libsensors.a)
	else()
		set(LIBSENSORS_NAME libsensors.so)
	endif()
	find_path(SENSORS_LIB_DIR ${LIBSENSORS_NAME} HINTS ${SENSORS_LIB} ${SENSORS_DIR}/include ENV LD_LIBRARY_PATH DOC "Path to ${LIBSENSORS_NAME}")
	if ( SENSORS_LIB_DIR )
		
	else ()
    	message(SEND_ERROR "${LIBSENSORS_NAME} not found, but required, use -DSENSORS_DIR or -DSENSORS_LIB!")
	endif ()
else()
    message(SEND_ERROR "sensors/sensors.h not found, but required, use -DSENSORS_DIR or -DSENSORS_INC!")
endif()

link_directories(${CMAKE_SOURCE_DIR})

add_library(sensors_plugin SHARED sensors_plugin.c)

target_link_libraries(sensors_plugin pthread ${LIBSENSORS_NAME})

install(TARGETS sensors_plugin LIBRARY DESTINATION lib)
