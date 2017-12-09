/*

 * res-sensor.c
 *
 *  Created on: 8. pro 2017.
 *      Author: Maja
 */
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "dev/button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "lib/sensors.h"
#include "mpu-9250-sensor.h"
#include "lib/sensors.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */

RESOURCE(res_sensor,
         "title=\"Accelerometer 3-axis:?axis=x|y|z..\";rt=\"accelerometer\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);


static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

	  size_t len = 0;
	  const char *axis = NULL;
	  int success = 1;


	if((len = REST.get_query_variable(request, "axis", &axis))) {


	    if(strncmp(axis, "x", len) == 0) {
	    	int x_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
	    	  if(x_axis < 0) {
	    		  snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "-");
	    	      x_axis = -x_axis;
	    	    }
	    	  //G
	    	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d.%02d", x_axis / 100, x_axis % 100);
	    } else if(strncmp(axis, "y", len) == 0) {
	    	int y_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
	    	//printf("usao u y");
	    	//snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", y_axis);
	    	 if(y_axis < 0) {
	    		 snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "-");
	    		 y_axis = -y_axis;
	    	}
	    		    	  //G
	    	 snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d.%02d", y_axis / 100, y_axis % 100);
	    } else if(strncmp(axis, "z", len) == 0) {
	    	int z_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);
	    		    	//printf("usao u y");
	    		    	//snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", y_axis);
	    		  if(z_axis < 0) {
	    		    snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "-");
	    		    	z_axis = -z_axis;
	    		    }
	    		    		    	  //G
	    		   snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d.%02d", z_axis / 100, z_axis % 100);
	    /*	printf("usao u z");
	    	int z_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);
	    	print_mpu_reading(x_axis);
	    	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", z_axis); */
	    }
	 }
	else {
		//error - empty string returned
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, " ");

	}
	int length= strlen((char*)buffer);
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
	REST.set_header_etag(response, (uint8_t *)&length, 1);
	REST.set_response_payload(response, buffer, length);
}


