/*

 * res-login.c
 *
 *  Created on: 8. pro 2017.
 *      Author: Maja
 */
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

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


static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	const char *pin = NULL;
	size_t length = 0;

	  /*if(length = REST.get_query_variable(request, "pin", &pin)) {

	    if(data_in_flash.is_pin_changed == 0 && length != 4) {

	    // start allarm , wrong PIN
	      printf("------- wrong PIN, alarm START ------\n");
	    }
	    else if(data_in_flash.is_pin_changed == 0 && length == 4) {
	    	if(strncmp(pin, data_in_flash.default_pin, length)){
	    		//send secret because the pin is correct
	    		// reset etimer so we have 5 mins of secret

	    		printf("------- GOOD PIN, secret > %d ------\n", data_in_flash.secret);

	    	}
	    	else{
	    		// start allarm , wrong PIN
	    		printf("------- wrong PIN, alarm START ------\n");
	    	}

	    }


	    //if(!put_once)
	    	memcpy(buffer, message, length);
	    //else if(put_once)
	    	//memcpy(buffer,input, (rv+1)*sizeof(char));
	  }*/
}

