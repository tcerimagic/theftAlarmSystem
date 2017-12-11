/*

 * res-change.c
 *
 *  Created on: 8. pro 2017.
 *      Author: Eniz
 */
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "flash-head.h"

static void res_put_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset);
static void res_delete_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_change,
		"title=\"Hello world: ?len=0..\";rt=\"Text\"",
		NULL,
		NULL,
		res_put_handler,
		res_delete_handler);

//put delete
static void res_put_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset) {
	size_t len = 0;
	const char *new_pin = NULL;
	const char *secret = NULL;
	char *message = NULL;
	uint8_t alarm_triggered = 0;
	int success = 1;

	load_data();
	if ((len = REST.get_query_variable(request, "secret", &secret))) {
		printf("sent secret > %.*s\n", len, secret);

		if (atoi(secret) == data_in_flash.secret) {
			printf("secret is good!\n");
		} else {
			printf("secret not good!\n");
			success = 0;
			alarm_triggered = 1;
			turn_on_alarm();
		}
	}
	else {
		printf("something wrong !\n");
		success = 0;
	}
	if (success && (len = REST.get_query_variable(request, "pin", &new_pin))) {
		printf("new_pin > %s\n", new_pin);

		//strncpy(data_in_flash.user_pin, new_pin, len);
		data_in_flash.user_pin = atoi(new_pin);
		data_in_flash.is_pin_default = 0;

	}
	else {
		printf("something wrong!\n");
		success = 0;
	}

	if(success == 0 && alarm_triggered == 0){
		strcat(message, "-1");
	}
	else if(success == 1 && alarm_triggered == 0){
		strcat(message, "0");
	}
	else if(alarm_triggered == 1){
		strcat(message, " ");
	}

	save_data();
	//memcpy(buffer, message, length);
	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", message);
	int length= strlen((char*)buffer);

	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
	REST.set_header_etag(response, (uint8_t *)&length, 1);
	REST.set_response_payload(response, buffer, length);

}

static void res_delete_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset) {

	//resets pin to 1234 if valid else alarm activated

	size_t len = 0;
	const char *new_pin = NULL;
	const char *secret = NULL;
	char *message = NULL;
	uint8_t alarm_triggered = 0;
	int success = 1;

	load_data();
	if ((len = REST.get_query_variable(request, "secret", &secret))) {
			printf("sent secret > %.*s\n", len, secret);

			if (atoi(secret) == data_in_flash.secret) {
				printf("secret is good!\n");
				data_in_flash.is_pin_default = 1;
			} else {
				printf("secret not good!\n");
				success = 0;
				turn_on_alarm();
			}
		}else {
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
