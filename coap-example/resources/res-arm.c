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
	int alarm_on = 0;

	if(buzzer_state())
		{
			alarm_on = 1;
		}

	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", alarm_on);
			int length= strlen((char*)buffer);
			REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
			REST.set_header_etag(response, (uint8_t *)&length, 1);
			REST.set_response_payload(response, buffer, length);

}

static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

}

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

}

static void res_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

}
