# Score-P libsensors Plugin Counter
This is a **async** plugin for [Score-P](https://www.vi-hps.org/projects/score-p/) to capture sensors available from hwmon.
It utilizes the library [`libsensors`](https://github.com/lm-sensors/lm-sensors) for that.

## Usage
The plugin is configure via environment variables.
The following are available:

- `SCOREP_METRIC_PLUGINS` (required):
  Comma-separated list of enable plugins. MUST contain `sensors_plugin`.
- `SCOREP_METRIC_SENSORS_PLUGIN` (required):
  Comma-separated list of metrics to record. See below how to find available metrics.
- `SCOREP_METRIC_SENSORS_PLUGIN_PERIOD` (optional, default=1000000)
  Set the amplitude to read the sensors (in usecs).
- `SCOREP_METRIC_SENSORS_PLUGIN_FILE` (optional, default=none)
  Use the given alternate `libsensors` config-file.
  Only intended for experienced users.

### Finding Available Metrics
The helper binary `scorep-plugin-libsensors-print-sensors` prints all available sensors.
Copy the string From the second column to the environment variable `SCOREP_METRIC_SENSORS_PLUGIN`.

Example output:

```
$ ./result/bin/scorep-plugin-libsensors-print-sensors
Chip/Feature/Sub-feature;"SCOREP_METRIC_SENSORS_PLUGIN";current value
thinkpad//sys/class/hwmon/hwmon4/fan1/fan1_input;"thinkpad/fan1/fan1_input";0.000000e+00
thinkpad//sys/class/hwmon/hwmon4/fan2/fan2_input;"thinkpad/fan2/fan2_input";0.000000e+00
thinkpad//sys/class/hwmon/hwmon4/temp1/temp1_input;"thinkpad/CPU/temp1_input";5.600000e+01
[...]
```

Here, one might use `export SCOREP_METRIC_SENSORS_PLUGIN="thinkpad/fan1/fan1_input,thinkpad/CPU/temp1_input"`

To scan for available sensors, start the tool `sensors-detect` as root and
follow the instructions on the screen. Use it at your own risk ;)

> Sensor detection might not be necessary, depending on your system.

## Compilation and Installation
### Prerequisites
To compile this plugin, you need:

- C compiler
- `libpthread`
- [Score-P](https://www.vi-hps.org/projects/score-p/)
- [`libsensors`](https://github.com/lm-sensors/lm-sensors) (including development files)

### Building
Ensure submodules are present. (execute `git submodule update--init --recursive`)
Then follow standard cmake procedure.

```bash
mkdir build && cd build
cmake ..
make -j
make install
```

Append `-DCMAKE_INSTALL_PREFIX=$PREFIX` to set a different install prefix.
Make sure that the installed `libsensors_plugin.so` is found by Score-P,
e.g. by setting your `LD_LIBRARY_PATH` appropriately.

The following further build options can be set with `-D<option>=<value>` on cmake invocation:

- `SENSORS_DIR` (no default): directory to installed libsensors
- `SENSORS_STATIC` (default `ON`): whether to link against static `libsensors.a` (value `ON`) or dynamic `libsensors.so` (value `OFF`)
- `BUILD_PRINT_SENSORS_BINARY` (default `ON`): whether to build the helper binary to print available sensors

> Further advanced options can be found in the source code.

## Full Example Invocation
```bash
# Score-P configuration
export SCOREP_ENABLE_PROFILING=0
export SCOREP_ENABLE_TRACING=1
export SCOREP_TOTAL_MEMORY=4095M

# plugin configuration
export SCOREP_METRIC_PLUGINS=sensors_plugin
export SCOREP_METRIC_SENSORS_PLUGIN="thinkpad/fan1/fan1_input,thinkpad/CPU/temp1_input"

# sleep5 is an Score-P instrumented binary from the test/ subdirectory
SCOREP_EXPERIMENT_DIRECTORY=trace ./sleep5

# dump results
otf2-print trace/traces.otf
```

## Testing
A simple script to check if the plugin is installed correctly resides in the subdirectory [`test/`](test/)

## Troubleshooting
1. Check whether the plugin library can be loaded from the `LD_LIBRARY_PATH`.
2. It may be, that `libsensors` needs root access to read the counters. In this case, you need to
   trace your application as root. 
   Alternatively, attempt to allow non-root users to read your sensors by setting `sudo sysctl kernel.perf_event_paranoid=-1`.
3. The maximum number of counters is defined in the source file. Adapt `MAX_SENSOR_COUNTER` to en-
   or decrease it and recompile the plugin.
4. Open an issue or send an E-Mail to the author.

## Authors
- Robert Schoene (robert.schoene at tu-dresden dot de)
