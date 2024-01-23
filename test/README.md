# E2E-Tests
This directory contains data to briefly test the capabilities of the plugin.

## Preconditions
Build and install this plugin, including the helper binary `scorep-plugin-libsensors-print-sensors`.
More precisely, make sure that the plugins `libsensors_plugin.so`, and the binary `scorep-plugin-libsensors-print-sensors` are available. (In `$PATH` and `$LD_LIBRARY_PATH` respectively.)

Build the binary `sleep5` using `Make`.
This requires `scorep-gcc`.

Ensure that `otf2-print` is installed for testing the produced traces.

Ensure that `trace_no_plugin` and `trace_with_plugin` do **not** exist.
(`rm -rf trace_no_plugin trace_with_plugin`)

## Test Content
The sample program `sleep5` is built and traced with scorep.

The metric to record is selected from the binary `scorep-plugin-libsensors-print-sensors`.

## Run Tests
```bash
# optional setup
export PATH=$(readlink -f ../build):$PATH
export LD_LIBRARY_PATH=$(readlink -f ../build):$LD_LIBRARY_PATH

rm -rf trace_no_plugin trace_with_plugin

bash record_trace.sh
```

Will exit with code 0 on success, otherwise will report failure.

## Troubleshooting
When encountering the following message, the plugin could not be loaded/found.
Ensure it is available in `$LD_LIBRARY_PATH`.

```
[Score-P] src/services/metric/scorep_metric_plugins.c:322: Warning: Could not open metric plugin sensors_plugin. Error message was: libsensors_plugin.so: cannot open shared object file: No such file or directory
```
