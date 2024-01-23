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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sensors/sensors.h>

int main(int, char**)
{
  int chipNr=0;
  int featureNr=0;
  int subFeatureNr=0;
  const sensors_subfeature * tmp_subFeature;
  const sensors_feature * tmp_feature;
  const sensors_chip_name * tmp_chip;
  char buffer[256];
  int retVal;
  double value;
  char * sensors_file_path;
  FILE * sensors_file=NULL;
  memset(buffer,0,256);
  /* check whether sensors config file is given */
  sensors_file_path = getenv("SCOREP_METRIC_SENSORS_PLUGIN_FILE");
   if (sensors_file_path == NULL) {
     sensors_file=fopen(sensors_file_path,"r");
   }
  sensors_init(sensors_file);
  printf("Chip/Feature/Sub-feature;\"SCOREP_METRIC_SENSORS_PLUGIN\";current value\n");
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
            sprintf(buffer, "%s/%s/%s", tmp_chip->prefix, sensors_get_label(tmp_chip,tmp_feature),tmp_subFeature->name);

            retVal=sensors_get_value(tmp_chip,
                tmp_subFeature->number,
                &value);
            printf("%s/%s/%s/%s;\"%s\";%e\n",
                tmp_chip->prefix, tmp_chip->path,
                tmp_feature->name, tmp_subFeature->name,
                buffer, value);
          }
      }
    }
  sensors_cleanup();
  return 0;
}
