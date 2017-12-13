/*

 * keepalive.c
 *
 *  Created on: 8. pro 2017.
 *      Author: Maja
 */
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "../flash-head.h"

#define ETIMER_INTERVAL 		(CLOCK_SECOND * 60)
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static struct etimer et;
/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_keepalive,
         "title=\"Keep_alive node:PUT..\";rt=\"Keeping node alive\"",
         NULL,
         NULL,
		 res_put_handler,
         NULL);

static void
res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	int success;
	const char *url = NULL;
	int len =0;
	len=REST.get_url(request,&url);
	//printf("%d", len);
	//printf("%s", url);
	load_data();
		if(strncmp(url, "keepalive", len) == 0){
			etimer_set(&et, ETIMER_INTERVAL);
			success=0;
		}
		else{
			turn_on_alarm();
			success=-1;
		}

	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", success);
	int length= strlen((char*)buffer);
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
	REST.set_header_etag(response, (uint8_t *)&length, 1);
	REST.set_response_payload(response, buffer, length);
}



