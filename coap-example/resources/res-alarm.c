/*
 * res-alarm.c
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
#include "../flash-head.h"



static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_alarm,
         "title=\"Alarm-On-Off:?secret=PIN..\";rt=\"ALARM_ON-OFF\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

//get
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	 load_data();
	 const char *secret_receive = NULL;
	  //get secret from flash
	 const char *secret_default= data_in_flash.secret;
	 int alarm_var=1; //default alarm is on
	  /* Some data that has the length up to REST_MAX_CHUNK_SIZE. For more, see the chunk resource. */
	 int length = 0; /*           |<-------->| */
	  /* The query string can be retrieved by rest_get_query() or parsed for its key-value pairs. */
	  if(length=REST.get_query_variable(request, "secret", &secret_receive)) {
		  printf("len = %d",length);
		  printf("secret %s", secret_receive);

		if(secret_default==atoi(secret_receive)){
			alarm_var= data_in_flash.is_alarm_on;
		}
		else {
			turn_on_alarm();
			save_data();

		}
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", alarm_var);
		int length= strlen((char*)buffer);
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
		REST.set_header_etag(response, (uint8_t *)&length, 1);
		REST.set_response_payload(response, buffer, length);
	}

}



