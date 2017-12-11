/*

 * res-login.c
 *
 *  Created on: 8. pro 2017.
 *      Author: Maja
 */
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include  "../flash-head.h"
//#include "../flash-functions.c"

static void res_get_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_login,
		"title=\"Hello world: ?len=0..\";rt=\"Text\"",
		res_get_handler,
		NULL,
		NULL,
		NULL);

static void res_get_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset)
{
	const char *pin = NULL;
	size_t length = 0;

	if (length = REST.get_query_variable(request, "pin", &pin))
	{

		load_data();

		if (length != 4)
		{
			turn_on_alarm();
		}
		else if (length == 4 && data_in_flash.is_pin_default == 1)
		{
			load_data();
			if (atoi(pin) == 1234)
			{
				etimer_set(&fmin_etimer,CLOCK_SECOND * 300);
				snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", data_in_flash.secret);
						int length= strlen((char*)buffer);
						REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
						REST.set_header_etag(response, (uint8_t *)&length, 1);
						REST.set_response_payload(response, buffer, length);
			}
			else
			{
				turn_on_alarm();
			}
			save_data();
		}

		else if (length == 4 && data_in_flash.is_pin_default == 0)
		{
			load_data();
			if (atoi(pin) == data_in_flash.user_pin)
			{
				etimer_set(&fmin_etimer,CLOCK_SECOND * 300);
				snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", data_in_flash.secret);
				int length= strlen((char*)buffer);
				REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
				REST.set_header_etag(response, (uint8_t *)&length, 1);
				REST.set_response_payload(response, buffer, length);
			}
			else
			{
				turn_on_alarm();
			}
			save_data();
		}
		save_data();

	}
}

