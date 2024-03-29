cmake_minimum_required(VERSION 3.5)
project(sensors_plugin)

option(SENSORS_STATIC "Link libsensors statically." ON)
set(SENSORS_DIR "" CACHE STRING "Path to libsensors directory")
set(SENSORS_INC "" CACHE STRING "Path to libsensors headers")
set(SENSORS_LIB "" CACHE STRING "Path to libsensors")

option(BUILD_PRINT_SENSORS_BINARY "build helper program to list all available sensors" ON)

set(PLUGIN_SOURCE sensors_plugin.c)

include(common/FindScorep.cmake)
if(SCOREP_FOUND)
    include_directories(${SCOREP_INCLUDE_DIRS})
    link_directories(${SCOREP_LIBRARY_DIRS})
else()
    message(SEND_ERROR "Scorep was not found but is required!")
endif()

# find libsensors
find_path(SENSORS_INC_DIR sensors/sensors.h HINTS ${SENSORS_INC} ${SENSORS_DIR}/include DOC "Path to sensors/sensors.h")
if(SENSORS_INC_DIR)
    if(SENSORS_STATIC)
        message("Using static libsensors")
        message("Linking might fail if you use the static version of libsensors and your library is not compiled with -fPIC. In this case use the cmake flag -DSENSORS_STATIC=OFF")
        set(LIBSENSORS_NAME libsensors.a)
    else()
        set(LIBSENSORS_NAME libsensors.so)
    endif()
    find_path(
        SENSORS_LIB_DIR ${LIBSENSORS_NAME}
        HINTS ${SENSORS_LIB} ${SENSORS_DIR}/include ${SENSORS_DIR}/lib
        ENV LD_LIBRARY_PATH
        DOC "Path to ${LIBSENSORS_NAME}")
    if (NOT SENSORS_LIB_DIR)
        message(SEND_ERROR "${LIBSENSORS_NAME} not found, but required, use -DSENSORS_DIR or -DSENSORS_LIB!")
    endif ()
else()
    message(SEND_ERROR "sensors/sensors.h not found, but required, use -DSENSORS_DIR or -DSENSORS_INC!")
endif()

#additional c flags
set(CMAKE_C_FLAGS "-D_GNU_SOURCE -g -std=gnu99 -pthread")
#debugging c flags
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wstrict-prototypes -Wall -Wundef -Wno-long-long -Wsign-compare -Wcomment -pedantic -finline-functions -fno-strict-aliasing")
#release c flags
set(CMAKE_C_FLAGS_RELEASE "-Os")

link_directories(${CMAKE_SOURCE_DIR})
add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCE})
target_link_libraries(${PROJECT_NAME} pthread ${LIBSENSORS_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE
    "${SENSORS_INC_DIR}")
target_link_directories(${PROJECT_NAME} PUBLIC
    "${SENSORS_LIB_DIR}")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

if (BUILD_PRINT_SENSORS_BINARY)
    add_executable(scorep-plugin-libsensors-print-sensors print_sensors.c)
    target_link_libraries(scorep-plugin-libsensors-print-sensors PUBLIC
        ${LIBSENSORS_NAME}
        m) # -lm equivalent
    target_include_directories(scorep-plugin-libsensors-print-sensors PRIVATE
        "${SENSORS_INC_DIR}")
    target_link_directories(scorep-plugin-libsensors-print-sensors PUBLIC
        "${SENSORS_LIB_DIR}")
    install(
        TARGETS scorep-plugin-libsensors-print-sensors
        DESTINATION bin)
endif()

