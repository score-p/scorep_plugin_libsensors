/*
print_sensors,
a program to display available sensors for a VampirTrace Plugin Counter
Copyright (C) 2010 TU Dresden, ZIH

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sensors/sensors.h>

int main(char **arv, int argc)
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
