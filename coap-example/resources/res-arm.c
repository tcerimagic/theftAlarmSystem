/*
 * res-arm.c
 *
 *  Created on: 8. pro 2017.
 *      Author: Maja
 */

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "buzzer.h"
#include "ti-lib.h"
#include "lpm.h"
#include "dev/leds.h"
#include "flash-head.h"

#include "board-peripherals.h"
#include "lib/sensors.h"
#include "mpu-9250-sensor.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_arm,
         "title=\"Hello world: ?len=0..\";rt=\"Text\"",
         res_get_handler,
		 res_post_handler,
		 res_put_handler,
		 res_delete_handler);


//post, put, delete
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	load_data();

	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", data_in_flash.is_armed);
			int length= strlen((char*)buffer);
			REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
			REST.set_header_etag(response, (uint8_t *)&length, 1);
			REST.set_response_payload(response, buffer, length);


}

static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
		size_t len = 0;
		const char *secret = NULL;
		char *message = NULL;
		int success = 1;

		load_data();

		if(data_in_flash.is_armed == 1)
		{
			success = 0;
			turn_on_alarm();
		}
		else if ((len = REST.get_query_variable(request, "secret", &secret))) {

			printf("sent secret > %.*s\n", len, secret);

			if (atoi(secret) == data_in_flash.secret) {
				printf("secret is good!\n");

				data_in_flash.x_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
				data_in_flash.y_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
				data_in_flash.z_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);
				data_in_flash.is_armed = 1;
				process_post(&alarm_on,device_armed_event,NULL);

			}

			else {
				printf("secret not good!\n");
				success = 0;
				turn_on_alarm();
			}
		}
		else {
			printf("something wrong !\n");
			success = 0;
		}


	if (success == 0) {
			strcat(message, "-1");
		} else {
			strcat(message, "0");
		}

		save_data();

			snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", message);
			int length= strlen((char*)buffer);

			REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
			REST.set_header_etag(response, (uint8_t *)&length, 1);
			REST.set_response_payload(response, buffer, length);



}

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	size_t len = 0;
			const char *secret = NULL;
			char *message = NULL;
			int success = 1;

			load_data();

			if ((len = REST.get_query_variable(request, "secret", &secret))) {

				printf("sent secret > %.*s\n", len, secret);

				if (atoi(secret) == data_in_flash.secret) {
					printf("secret is good!\n");

					data_in_flash.x_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
					data_in_flash.y_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
					data_in_flash.z_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);
					data_in_flash.is_armed = 1;
					process_post(&alarm_on,device_armed_event,NULL);
				}

				else {
					printf("secret not good!\n");
					success = 0;
					turn_on_alarm();
				}
			}
			else {
				printf("something wrong !\n");
				success = 0;
			}


		if (success == 0) {
				strcat(message, "-1");
			} else {
				strcat(message, "0");
			}

			save_data();

				snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", message);
				int length= strlen((char*)buffer);

				REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
				REST.set_header_etag(response, (uint8_t *)&length, 1);
				REST.set_response_payload(response, buffer, length);


}

static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

			size_t len = 0;
			const char *secret = NULL;
			char *message = NULL;
			int success = 1;

			load_data();

			 if ((len = REST.get_query_variable(request, "secret", &secret))) {

				printf("sent secret > %.*s\n", len, secret);

				if (atoi(secret) == data_in_flash.secret) {
					printf("secret is good!\n");
					if(data_in_flash.is_armed == 1)
						{
							turn_off_alarm();
							process_post(&alarm_on,device_disarm_event,NULL);
						}
				}

				else {
					printf("secret not good!\n");
					success = 0;
					turn_on_alarm();
				}
			}
			else {
				printf("something wrong !\n");
				success = 0;
			}


		if (success == 0) {
				strcat(message, "-1");
			} else {
				strcat(message, "0");
			}

			save_data();

				snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", message);
				int length= strlen((char*)buffer);

				REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
				REST.set_header_etag(response, (uint8_t *)&length, 1);
				REST.set_response_payload(response, buffer, length);

}
