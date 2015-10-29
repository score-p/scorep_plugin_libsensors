#Score-P libsensors Plugin Counter

##Compilation and Installation

###Prerequisites

To compile this plugin, you need:

* C compiler

* `libpthread`

* Score-P

* `libsensors` (including development files)

###Building

1. Create build directory

        mkdir build
        cd build

2. Invoke CMake

        cmake ..

2. Invoke make

        make

3. Copy it to a location listed in `LD_LIBRARY_PATH` or add current path to `LD_LIBRARY_PATH` with

        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`

##Usage

To make sensors available to your system, you should start the tool `sensors-detect` as root and
follow the instructions on the screen. Use it at your own risk ;).

`SCOREP_METRIC_SENSORS_PLUGIN` specifies the software events that shall be recorded when tracing an
application. The available events highly depend on your system. Compile and run `print_sensors.c`,
which events are supported. Use the `Plugin ID`s provided to add the events to your plugin counter
metrics.

* Compile

        gcc print_sensors.c -lsensors -o print_sensors

* Run (example from my system)

        user$ ./print_sensors
        Chip/Feature/Sub-feature            "VT_PLUGIN_CNTR_METRIC" =   current value
        coretemp//sys/class/hwmon/hwmon0/device/temp1/temp1_input           "Core 0/temp1_input"    =   3.700000e+01
        coretemp//sys/class/hwmon/hwmon0/device/temp1/temp1_max         "Core 0/temp1_max"  =   8.200000e+01
        coretemp//sys/class/hwmon/hwmon0/device/temp1/temp1_crit            "Core 0/temp1_crit" =   1.000000e+02
        coretemp//sys/class/hwmon/hwmon0/device/temp1/temp1_crit_alarm          "Core 0/temp1_crit_alarm"   =   0.000000e+00
        coretemp//sys/class/hwmon/hwmon1/device/temp1/temp1_input           "Core 1/temp1_input"    =   3.800000e+01
        coretemp//sys/class/hwmon/hwmon1/device/temp1/temp1_max         "Core 1/temp1_max"  =   8.200000e+01
        coretemp//sys/class/hwmon/hwmon1/device/temp1/temp1_crit            "Core 1/temp1_crit" =   1.000000e+02
        coretemp//sys/class/hwmon/hwmon1/device/temp1/temp1_crit_alarm          "Core 1/temp1_crit_alarm"   =   0.000000e+00
        coretemp//sys/class/hwmon/hwmon2/device/temp1/temp1_input           "Core 2/temp1_input"    =   3.500000e+01
        coretemp//sys/class/hwmon/hwmon2/device/temp1/temp1_max         "Core 2/temp1_max"  =   8.200000e+01
        coretemp//sys/class/hwmon/hwmon2/device/temp1/temp1_crit            "Core 2/temp1_crit" =   1.000000e+02
        coretemp//sys/class/hwmon/hwmon2/device/temp1/temp1_crit_alarm          "Core 2/temp1_crit_alarm"   =   0.000000e+00
        coretemp//sys/class/hwmon/hwmon3/device/temp1/temp1_input           "Core 3/temp1_input"    =   4.200000e+01
        coretemp//sys/class/hwmon/hwmon3/device/temp1/temp1_max         "Core 3/temp1_max"  =   8.200000e+01
        coretemp//sys/class/hwmon/hwmon3/device/temp1/temp1_crit            "Core 3/temp1_crit" =   1.000000e+02
        coretemp//sys/class/hwmon/hwmon3/device/temp1/temp1_crit_alarm          "Core 3/temp1_crit_alarm"   =   0.000000e+00

    (`Core */temp1_input` is the temperature of CPU core `*`)

To add a kernel event counter to your trace, you have to specify the environment variables
`SCOREP_METRIC_PLUGINS` and `SCOREP_METRIC_SENSORS_PLUGIN`.

Load the PC plugin library:

    SCOREP_METRIC_PLUGINS="sensors_plugin"

E.g.

    export SCOREP_METRIC_SENSORS_PLUGIN="Core 0/temp1_input:Core 1/temp1_input"

Experienced `libsensors` users may pass another sensors config-file by setting the environment
variable `SCOREP_METRIC_SENSORS_PLUGIN_FILE` to the config-files location.

###Environment variables

* `SCOREP_METRIC_SENSORS_PLUGIN_FILE` (default=none)

    Use this alternate `libsensors` config-file.

* `SCOREP_METRIC_SENSORS_PLUGIN_PERIOD` (default=1000000)

    Set the amplitude to read the sensors (in usecs).

###If anything fails

1. Check whether the plugin library can be loaded from the `LD_LIBRARY_PATH`.

2. It may be, that `libsensors` needs root access to read the counters. In this case, you need to
    trace your application as root.

3. The maximal number of counters is defined in the source file. Adapt `MAX_SENSOR_COUNTER` to en-
    or decrease it and recompile the plugin.

4. Write a mail to the author.

##Authors

* Robert Schoene (robert.schoene at tu-dresden dot de)
