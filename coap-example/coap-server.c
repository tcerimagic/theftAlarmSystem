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
 *      Erbium (Er) REST Engine example.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "net/rpl/rpl.h"

#include "dev/button-sensor.h"
#include "batmon-sensor.h"
#include "lib/sensors.h"
#include "mpu-9250-sensor.h"
#include "ti-lib.h"

#include "dev/leds.h"
#include "board-peripherals.h"


#include "buzzer.h"
#include "ti-lib.h"
#include "lpm.h"
#include "buzzer.h"
#include "flash-head.h"
#include "flash-functions.c"

#define DEBUG 1
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


#define LED_TOGGLE_INTERVAL 		(CLOCK_SECOND * 0.5)

/*------ Global variables --------------------*/

static struct etimer fmin_etimer;
static int secret_code = 0;
static struct etimer alarm_timer;


enum {
	device_disarm_event,
	device_armed_event,
	get_lock
};




/*------ FUNctions ---------------------------*/


/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern resource_t
  res_hello,
  res_event,
  res_push,
  res_leds,
  res_batmon,
  res_toggle,
  res_login,
  res_change,
  res_sensor,
  res_arm,
  res_threshold,
  res_alarm,
  res_keepalive;

PROCESS(er_example_server, "Erbium Example Server");
PROCESS(alarm_on, "ALARM is ON");
AUTOSTART_PROCESSES(&resolv_process,&er_example_server, &alarm_on);

PROCESS_THREAD(er_example_server, ev, data)
{
	SENSORS_ACTIVATE(batmon_sensor);
	//SENSORS_ACTIVATE(mpu_9250_sensor);
	//init_mpu_reading(NULL);
	mpu_9250_sensor.configure(SENSORS_ACTIVE,MPU_9250_SENSOR_TYPE_ALL);
	buzzer_init();
#if DAG_ROOT_ENABLE
  struct uip_ds6_addr *root_if;
  uip_ipaddr_t ipaddr;
#endif

  PROCESS_BEGIN();

#if DAG_ROOT_ENABLE
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);

  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
#endif

  resolv_set_hostname("exercise6_sink");

  PROCESS_PAUSE();

  PRINTF("Starting Erbium Example Server\n");

  PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
  PRINTF("LL header: %u\n", UIP_LLH_LEN);
  PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
  PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

  /* Initialize the REST engine. */
  rest_init_engine();

  /*
   * Bind the resources to their Uri-Path.
   * WARNING: Activating twice only means alternate path, not two instances!
   * All static variables are the same for each URI path.
   */
  	 rest_activate_resource(&res_hello, "test/hello");
     rest_activate_resource(&res_leds, "actuators/leds");
     rest_activate_resource(&res_toggle, "actuators/toggle");
     rest_activate_resource(&res_push, "test/push");
     rest_activate_resource(&res_event, "sensors/button");
     rest_activate_resource(&res_batmon, "sensors/battery");
     rest_activate_resource(&res_login, "auth/login");
     rest_activate_resource(&res_change, "auth/change");
     rest_activate_resource(&res_arm, "theft/arm");
     rest_activate_resource(&res_threshold, "theft/threshold");
     rest_activate_resource(&res_alarm, "theft/alarm");
     rest_activate_resource(&res_sensor, "theft/sensor");
     rest_activate_resource(&res_keepalive, "keepalive");




  /* Define application-specific events here. */
  /*--- 5 minute timer initialisation ----*/

/*  if(secret_code == 0){
	  load_data();
	 set_init_data();
	 save_data();
  }*/

  etimer_set(&fmin_etimer, CLOCK_SECOND * 60);

  if(data_in_flash.secret == -1)
    {
	  load_data();
	  set_init_data();
	  save_data();
    }

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == sensors_event && data == &button_sensor) {
      PRINTF("*******BUTTON*******\n");

      /* Call the event_handler for this application-specific event. */
      res_event.trigger();

    }
    else if(etimer_expired(&fmin_etimer)){

    	load_data();
    	 if(secret_code == 0)
    	    {
    	    	set_init_data();
    	    }

    	secret_code = data_in_flash.secret;

    	data_in_flash.secret = ++secret_code;

        printf("default pin %s\n", data_in_flash.default_pin);

    	save_data();

    	etimer_reset(&fmin_etimer);



    }

 }

  PROCESS_END();
}

PROCESS_THREAD(alarm_on, ev, data){
	PROCESS_EXITHANDLER();
	PROCESS_BEGIN();

	//timer_set(&alarm_timer, LED_TOGGLE_INTERVAL);

	while(1) {

		if(ev == device_armed_event){

			int armed_device = 0;

			while(armed_device != 1){
				int x_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
				int y_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
				int z_axis= mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);

				load_data();
				if(x_axis > data_in_flash.threshold){
					turn_on_alarm();
					armed_device = 1;
				}
				else if(y_axis > data_in_flash.threshold){
					turn_on_alarm();
					armed_device = 1;
				}
				else if(z_axis > data_in_flash.threshold){
					turn_on_alarm();
					armed_device = 1;
				}
				save_data();

			}

			if(armed_device == 1){
				etimer_set(&alarm_timer, LED_TOGGLE_INTERVAL);

			}

		}
		else if(ev == device_disarm_event){
				turn_off_alarm();
		}

		if(etimer_expired(&alarm_timer)){
			leds_toggle(LEDS_RED);
			if(data_in_flash.is_alarm_on){
				etimer_set(&alarm_timer, LED_TOGGLE_INTERVAL);
			}
		}

		PROCESS_END();
}

