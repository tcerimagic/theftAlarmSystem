/*
 * res-threshold.c

 *
 *  Created on: 8. pro 2017.
 *      Author: Maja
 */

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_threshold,
         "title=\"Hello world: ?len=0..\";rt=\"Text\"",
         res_get_handler,
         NULL,
		 res_put_handler,
         NULL);

//get, put
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	 	 load_data();
		 const char *secret_receive = NULL;
		 const char *secret_default= data_in_flash.secret;
		 int length = 0; /*           |<-------->| */
		  /* The query string can be retrieved by rest_get_query() or parsed for its key-value pairs. */
		  if(length=REST.get_query_variable(request, "secret", &secret_receive)) {
			if(secret_default==atoi(secret_receive)){
				//secret is valid
				snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", data_in_flash.threshold);
			}
			else {
				turn_on_alarm();
				snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "-1");
			}

		 }
		  int length= strlen((char*)buffer);
		  REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
		  REST.set_header_etag(response, (uint8_t *)&length, 1);
		  REST.set_response_payload(response, buffer, length);
}

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
			 load_data();
			 const char *secret_receive = NULL;
			 const char *threshold_receive = NULL;
			 const char *secret_default= data_in_flash.secret;
			 int length = 0;
			 int success=0;

			  if(length=REST.get_query_variable(request, "secret", &secret_receive)) {
				if(secret_default==atoi(secret_receive)){
					//secret is valid
					printf("secret is good!\n");

					if(len = REST.get_query_variable(request, "threshold", &threshold_receive)){

						data_in_flash.threshold = atoi(threshold_receive);
						snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "0");
					}
					else{
						snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "-1");
					}

				}
			  }
				else {
					printf("secret not good!\n");
					turn_on_alarm();
					snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "-1");
				}

			  	save_data();

			  	int length= strlen((char*)buffer);

			  	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
			  	REST.set_header_etag(response, (uint8_t *)&length, 1);
			  	REST.set_response_payload(response, buffer, length);

}



