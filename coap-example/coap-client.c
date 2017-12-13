/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Erbium (Er) CoAP client example.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "er-coap-engine.h"
#include "dev/button-sensor.h"

/* Example URIs that can be queried. */

/*--------------------------Changed to 4 for test/hello-----------------------*/
//#define NUMBER_OF_URLS 3

#define NUMBER_OF_URLS 6
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_urls[NUMBER_OF_URLS] =
//{ ".well-known/core", "actuators/toggle", "sensors/battery" };
{ ".well-known/core", "actuators/toggle", "sensors/battery", "test/hello", "theft/alarm", "auth/login" };

static int uri_switch = 0;

const uint8_t *final_response;
static int response_length;

#define OBS_RESOURCE_URI "sensors/button"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

#define UDP_CONNECTION_ADDR       exercise6_sink.local

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

#define TOGGLE_INTERVAL 10

PROCESS(er_example_client, "Erbium Example Client");
AUTOSTART_PROCESSES(&er_example_client);

uip_ipaddr_t server_ipaddr;
static struct etimer et;
static coap_observee_t *obs;

static resolv_status_t set_connection_address(uip_ipaddr_t *ipaddr)
{
	resolv_status_t status = RESOLV_STATUS_ERROR;

	if(uiplib_ipaddrconv(QUOTEME(UDP_CONNECTION_ADDR), ipaddr) == 0) {
		uip_ipaddr_t *resolved_addr = NULL;
		status = resolv_lookup(QUOTEME(UDP_CONNECTION_ADDR),&resolved_addr);
		if(status == RESOLV_STATUS_UNCACHED || status == RESOLV_STATUS_EXPIRED) {
			//PRINTF("Attempting to look up %s\n",QUOTEME(UDP_CONNECTION_ADDR));
			resolv_query(QUOTEME(UDP_CONNECTION_ADDR));
			status = RESOLV_STATUS_RESOLVING;
		} else if(status == RESOLV_STATUS_CACHED && resolved_addr != NULL) {
			//PRINTF("Lookup of \"%s\" succeded!\n",QUOTEME(UDP_CONNECTION_ADDR));
		} else if(status == RESOLV_STATUS_RESOLVING) {
			//PRINTF("Still looking up \"%s\"...\n",QUOTEME(UDP_CONNECTION_ADDR));
		} else {
			//PRINTF("Lookup of \"%s\" failed. status = %d\n",QUOTEME(UDP_CONNECTION_ADDR),status);
		}
		if(resolved_addr)
			uip_ipaddr_copy(ipaddr, resolved_addr);
	} else {
		status = RESOLV_STATUS_CACHED;
	}

	return status;
}

/*----------------------------------------------------------------------------*/
/*
 * Handle the response to the observe request and the following notifications
 */
static void
notification_callback(coap_observee_t *obs, void *notification,
                      coap_notification_flag_t flag)
{
  int len = 0;
  const uint8_t *payload = NULL;

  printf("Notification handler\n");
  printf("Observee URI: %s\n", obs->url);
  if(notification) {
    len = coap_get_payload(notification, &payload);
  }
  switch(flag) {
  case NOTIFICATION_OK:
    printf("NOTIFICATION OK: %*s\n", len, (char *)payload);
    break;
  case OBSERVE_OK: /* server accepeted observation request */
    printf("OBSERVE_OK: %*s\n", len, (char *)payload);
    break;
  case OBSERVE_NOT_SUPPORTED:
    printf("OBSERVE_NOT_SUPPORTED: %*s\n", len, (char *)payload);
    obs = NULL;
    break;
  case ERROR_RESPONSE_CODE:
    printf("ERROR_RESPONSE_CODE: %*s\n", len, (char *)payload);
    obs = NULL;
    break;
  case NO_REPLY_FROM_SERVER:
    printf("NO_REPLY_FROM_SERVER: "
           "removing observe registration with token %x%x\n",
           obs->token[0], obs->token[1]);
    obs = NULL;
    break;
  }
}
/*----------------------------------------------------------------------------*/
/*
 * Toggle the observation of the remote resource
 */
void
toggle_observation(void)
{
  if(obs) {
    printf("Stopping observation\n");
    coap_obs_remove_observee(obs);
    obs = NULL;
  } else {
    printf("Starting observation\n");
    obs = coap_obs_request_registration(&server_ipaddr, REMOTE_PORT,
                                        OBS_RESOURCE_URI, notification_callback, NULL);
  }
}

/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void
client_chunk_handler(void *response)
{
  const uint8_t *chunk;

  int len = coap_get_payload(response, &chunk);

  snprintf((char *)final_response, REST_MAX_CHUNK_SIZE, "%s%s", "seceret=",(char *)chunk);

  printf("%.*s", len, (char *)chunk);
}
PROCESS_THREAD(er_example_client, ev, data)
{
  PROCESS_BEGIN();

  static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */

  static resolv_status_t status = RESOLV_STATUS_UNCACHED;
	PRINTF("Starting mDNS name resolution...\n");
	while(status != RESOLV_STATUS_CACHED) {
		status = set_connection_address(&server_ipaddr);

		if(status == RESOLV_STATUS_RESOLVING) {
			PROCESS_WAIT_EVENT_UNTIL(ev == resolv_event_found);
		} else if(status != RESOLV_STATUS_CACHED) {
			//PRINTF("Can't get connection address.\n");
			PROCESS_PAUSE();
		}
	}
	PRINTF(QUOTEME(UDP_CONNECTION_ADDR));
  PRINTF(": ");
	PRINT6ADDR(&server_ipaddr);
	printf("\n");

  /* receives all CoAP messages */
  coap_init_engine();

  etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);

  SENSORS_ACTIVATE(button_sensor);
  printf("Press the left button to request %s\n", service_urls[uri_switch]);

  while(1) {
    PROCESS_YIELD();

   /* if(etimer_expired(&et)) {
      printf("--Toggle timer--\n");

      coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
      coap_set_header_uri_path(request, service_urls[1]);

      const char msg[] = "Toggle!";

      coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);

      PRINT6ADDR(&server_ipaddr);
      PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

      COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                            client_chunk_handler);

      printf("\n--Done--\n");

      etimer_reset(&et);

    } */
  if(ev == sensors_event && data == &button_left_sensor) {

      /* send a request to notify the end of the process */

    	coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_urls[5]);
      coap_set_header_uri_query(request, "pin=1234");
      COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                            client_chunk_handler);



      coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
            coap_set_header_uri_path(request, service_urls[4]);
            coap_set_header_uri_query(request, (char *)final_response);
            COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                                  client_chunk_handler);


     // uri_switch = (uri_switch + 1) % NUMBER_OF_URLS;
    }
    else if(ev == sensors_event && data == &button_right_sensor) {

    	coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
    	      coap_set_header_uri_path(request, service_urls[5]);
    	      coap_set_header_uri_query(request, "pin=1234");
    	      COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
    	                            client_chunk_handler);



    	      coap_init_message(request, COAP_TYPE_CON, COAP_DELETE, 0);
    	            coap_set_header_uri_path(request, service_urls[4]);
    	            coap_set_header_uri_query(request, (char *)final_response);
    	            COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
    	                                  client_chunk_handler);


		}
  }

  PROCESS_END();
}
