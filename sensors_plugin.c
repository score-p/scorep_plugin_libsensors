/*
 * Copyright (c) 2016, Technische Universität Dresden, Germany
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 *    and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 *    and the following disclaimer in the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <scorep/SCOREP_MetricPlugins.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <sensors/sensors.h>


#define MAX_SENSOR_COUNTER 40

typedef struct lm_sensors_counter_t {
  const sensors_subfeature * subFeature;
  const sensors_chip_name * chip;
} sensor_counter;

static int number_sensors = 0;
static int added_sensor_counters = 0;
static sensor_counter sensor_counters[MAX_SENSOR_COUNTER];
static SCOREP_MetricTimeValuePair * results[MAX_SENSOR_COUNTER];
static int nr_results [MAX_SENSOR_COUNTER];
static int max_results [MAX_SENSOR_COUNTER];

SCOREP_MetricTimeValuePair* return_values;
int return_values_length;

static uint64_t (*wtime)(void) = NULL;
static volatile int counter_enabled;
static int period = 1000000;

static pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread;
int thread_finished;
int thread_active;

/* used to get the timer from Score-P */
void set_timer( uint64_t (*timer)(void)){
    wtime=timer;
}

/* thread function that reads sensors information on a regular base */
void * thread_report(void * ignored){
    int retVal=0;
    /* we have to report uint64_t, but read double */
    union{
        double dbl;
        uint64_t uint64;
    } value;
    thread_finished=0;
    /* as long as finalize isn't called from Score-P */
    while (counter_enabled){
        int i=0;
        if (wtime==NULL) continue;
      pthread_mutex_lock(&read_mutex);
        for (i=0; i< added_sensor_counters; i++ ){
            // buffer size not sufficient?
            // if so increase buffersize
            if (nr_results[i]==max_results[i]){
              SCOREP_MetricTimeValuePair * reallocated;
              reallocated = realloc(results[i],nr_results[i]*2*sizeof(SCOREP_MetricTimeValuePair));
              /* if reallocation failed */
              if (reallocated == NULL){
                fprintf(stderr, "Score-P Sensors Plugin: failed to allocate more memory for results.\n"
                    "Stopping Sensor measurement after %d readings",nr_results[i]);
                counter_enabled = 0;
              }
              else {
                /* allocation ok */
                results[i] = reallocated;
              }
              max_results[i]=nr_results[i]*2;
            }
            /* read values and store them in array */
            results[i][nr_results[i]].timestamp=wtime();
                if (sensors_get_value(sensor_counters[i].chip,
                                    sensor_counters[i].subFeature->number,
                                    &value.dbl)<0)
                  /* TODO find better way then atof() */
                  value.dbl=atof("NaN");
                results[i][nr_results[i]].value=value.uint64;
            nr_results[i]++;
        }
        pthread_mutex_unlock(&read_mutex);
        usleep(period);
    }
    thread_finished=1;
    return NULL;
}

int32_t init()
{
  char * from_env;
  int i;
  FILE * sensors_file=NULL;
  /* check whether sensors config file is given */
  from_env = getenv("SCOREP_METRIC_SENSORS_PLUGIN_FILE");
   if (from_env != NULL) {
     sensors_file=fopen(from_env,"r");
   }
  sensors_init(sensors_file);
  /* check whether an amplitude is set */
  from_env = getenv("SCOREP_METRIC_SENSORS_PLUGIN_PERIOD");
   if (from_env != NULL) {
     period=atoi(from_env);
     if (period==0)
       period=1000000;
   }
  /* initially we have 1000000 counter values */
  for (i = 0 ; i < MAX_SENSOR_COUNTER; i++){
    nr_results[i]=0;
    max_results[i]=1000000;
  }
  return_values = (SCOREP_MetricTimeValuePair*) malloc (1000000*sizeof(SCOREP_MetricTimeValuePair));
  if (return_values==NULL){
    fprintf(stderr, "Score-P Sensors Plugin: failed to allocate memory for results.\n");
    return -ENOMEM;
  }
  return_values_length = 1000000;
  counter_enabled=1;
  thread=0;
  thread_finished=1;
  thread_active=0;
  return 0;
}

SCOREP_Metric_Plugin_MetricProperties * get_event_info(char * event_name)
{
  int chipNr=0;
  int featureNr=0;
  int subFeatureNr=0;
  const sensors_subfeature * tmp_subFeature;
  const sensors_feature * tmp_feature;
  const sensors_chip_name * tmp_chip;
  char buffer[256];
  char * unit=NULL;
  int found;

  SCOREP_Metric_Plugin_MetricProperties * return_values;

  if (number_sensors==MAX_SENSOR_COUNTER){
    fprintf(stderr,"Ignoring event '%s', only %i events supported\n"
        "Adopt MAX_SENSOR_COUNTER and recompile the sensors plugin\n"
        "to avoid this limitation.\n"
        ,event_name, MAX_SENSOR_COUNTER);
    return NULL;
  }

  tmp_chip = sensors_get_detected_chips(NULL, &chipNr);
  // while theres a new chip
  for(;tmp_chip!=NULL;tmp_chip = sensors_get_detected_chips(NULL, &chipNr)) {
    featureNr=0;
    tmp_feature = sensors_get_features(tmp_chip, &featureNr);
      // while theres a new feature
      for (;tmp_feature!=NULL;tmp_feature = sensors_get_features(tmp_chip, &featureNr)){
        subFeatureNr=0;
        tmp_subFeature=sensors_get_all_subfeatures(tmp_chip,tmp_feature,&subFeatureNr);
          // while theres a new subfeature
          for (;tmp_subFeature!=NULL;tmp_subFeature=sensors_get_all_subfeatures(tmp_chip,tmp_feature,&subFeatureNr))
          {
            sprintf(buffer, "%s/%s/%s", tmp_chip->prefix,sensors_get_label(tmp_chip,tmp_feature),tmp_subFeature->name);
            //fprintf(stderr,"%s\n",buffer);
            if (strcmp(buffer, event_name)==0){
              switch (tmp_feature->type){
                case SENSORS_FEATURE_IN:
                  unit = strdup("Volt");
                  break;
                case SENSORS_FEATURE_FAN:
                  unit = strdup("RPM");
                  break;
                case SENSORS_FEATURE_TEMP:
                  unit = strdup("Celsius");
                  break;
                case SENSORS_FEATURE_POWER:
                  unit = strdup("Watt");
                  break;
                case SENSORS_FEATURE_ENERGY:
                  unit = strdup("Joule");
                  break;
                case SENSORS_FEATURE_CURR:
                  unit = strdup("Ampere");
                  break;
                case SENSORS_FEATURE_HUMIDITY:
                  unit = strdup("Percent");
                  break;
                default:
                  unit = NULL;
                  break;
              }
              found=1;
            }
          }
      }
    }
    if (!found){
      fprintf(stderr,"The Sensor\"%s\" does not exist on your system\n",event_name);
      return NULL;
    }

    /* Wildcards are not supported yet, so we need two entries */
    return_values = malloc(2 * sizeof(SCOREP_Metric_Plugin_MetricProperties));
    if (return_values==NULL){
      fprintf(stderr, "Score-P Sensors Plugin: failed to allocate memory for internal struct.\n");
      return NULL;
    }

    /* if the description is null it should be considered the end */
    return_values[0].name        = strdup(event_name);
    return_values[0].unit        = unit;
    return_values[0].mode        = SCOREP_METRIC_MODE_ABSOLUTE_POINT;
    return_values[0].value_type  = SCOREP_METRIC_VALUE_DOUBLE;
    return_values[0].base        = SCOREP_METRIC_BASE_DECIMAL;
    return_values[0].exponent    = 0;
    /* Last element empty */
    return_values[1].name = NULL;
    number_sensors++;
    return return_values;
}

void fini()
{
  counter_enabled=0;
  if (1 == thread_active) {
    int ret = pthread_join(thread, NULL);
    if (0 != ret) {
      fprintf(stderr, "failed to clean up measurement thread (%d): %s\n",
              ret, strerror(errno));
    }
    thread_active=0;
  }
  pthread_mutex_destroy(&read_mutex);
  sensors_cleanup();
}

int32_t add_counter(char * event_name)
{
  int chipNr=0;
  int featureNr=0;
  int subFeatureNr=0;
  const sensors_subfeature * tmp_subFeature;
  const sensors_feature * tmp_feature;
  const sensors_chip_name * tmp_chip;
  char buffer[256];
  int found=0;
  if (added_sensor_counters>=MAX_SENSOR_COUNTER){
    fprintf(stderr,"Ignoring event '%s', only %i events supported\n"
        "Adopt MAX_SENSOR_COUNTER and recompile the sensors plugin\n"
        "to avoid this limitation.\n"
        ,event_name, MAX_SENSOR_COUNTER);
    return -1;
  }
  memset(buffer,0,256);
  thread_finished=0;
  tmp_chip = sensors_get_detected_chips(NULL, &chipNr);
  // while theres a new chip
  for(;tmp_chip!=NULL;tmp_chip = sensors_get_detected_chips(NULL, &chipNr)) {
    featureNr=0;
    tmp_feature = sensors_get_features(tmp_chip, &featureNr);
      // while theres a new feature
      for (;tmp_feature!=NULL;tmp_feature = sensors_get_features(tmp_chip, &featureNr)){
        subFeatureNr=0;
        tmp_subFeature=sensors_get_all_subfeatures(tmp_chip,tmp_feature,&subFeatureNr);
          // while theres a new subfeature
          for (;tmp_subFeature!=NULL;tmp_subFeature=sensors_get_all_subfeatures(tmp_chip,tmp_feature,&subFeatureNr))
          {
            sprintf(buffer, "%s/%s/%s", tmp_chip->prefix,sensors_get_label(tmp_chip,tmp_feature),tmp_subFeature->name);
            //fprintf(stderr,"%s\n",buffer);
            if (strcmp(buffer, event_name)==0){
              sensor_counters[added_sensor_counters].chip=tmp_chip;
              sensor_counters[added_sensor_counters].subFeature=tmp_subFeature;
              added_sensor_counters++;
              found=1;
            }
          }
      }
    }

  if (!found)
    return -1;

  results[added_sensor_counters-1]=malloc(max_results[added_sensor_counters-1]*sizeof(SCOREP_MetricTimeValuePair));
  if (results[added_sensor_counters-1]==NULL){
    fprintf(stderr, "Score-P Sensors Plugin: failed to allocate memory for results.\n");
    return -ENOMEM;
  }
  // are all events added?
  if (added_sensor_counters == number_sensors) {
    if (pthread_create(&thread, NULL, &thread_report, NULL)){
      fprintf(stderr, "Score-P Sensors Plugin: Unable to start measurement thread.\n");
      return -ECHILD;
    }
    thread_active=1;
  }

  return added_sensor_counters-1;
}

uint64_t get_all_values( int32_t                      id,
                         SCOREP_MetricTimeValuePair** time_value_list ){
  int saved_nr_results;
  pthread_mutex_lock(&read_mutex);
  saved_nr_results = nr_results[id];
  SCOREP_MetricTimeValuePair * return_values=malloc(saved_nr_results*sizeof(SCOREP_MetricTimeValuePair));
  if (return_values==NULL){
    fprintf(stderr, "Score-P Sensors Plugin: failed to allocate memory for passing results to Score-P.\n");
    return 0;
  }
  memcpy( return_values, results[id], saved_nr_results*sizeof(SCOREP_MetricTimeValuePair) );
  time_value_list[0]=return_values;
  nr_results[id] = 0;
  pthread_mutex_unlock(&read_mutex);
  return saved_nr_results;
}

/**
 * This function get called to give some informations about the plugin to scorep
 */
SCOREP_METRIC_PLUGIN_ENTRY( sensors_plugin )
{
    /* Initialize info data (with zero) */
    SCOREP_Metric_Plugin_Info info;
    memset( &info, 0, sizeof( SCOREP_Metric_Plugin_Info ) );

    /* Set up the structure */
    info.plugin_version               = SCOREP_METRIC_PLUGIN_VERSION;
    info.run_per                      = SCOREP_METRIC_PER_HOST;
    info.sync                         = SCOREP_METRIC_ASYNC;
    info.delta_t                      = UINT64_MAX;
    info.initialize                   = init;
    info.finalize                     = fini;
    info.get_event_info               = get_event_info;
    info.add_counter                  = add_counter;
    info.get_all_values               = get_all_values;
    info.set_clock_function           = set_timer;

    return info;
}
