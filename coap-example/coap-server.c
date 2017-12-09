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

#include "buzzer.h"
#include "ti-lib.h"
#include "lpm.h"

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

/*------ Structures --------------------------*/

struct AuthData{
	char *default_pin = "1234";
	char *user_pin;
	int is_pin_changed; // if 0 -> not changed
};

/*------ Global variables --------------------*/

static struct etimer 5min_etimer;
static struct AuthData data_in_flash;



/*------ FUNctions ---------------------------*/
static void
save_data()
{
  /* Dump current running config to flash */

  int rv;
  cc26xx_web_demo_sensor_reading_t *reading = NULL;

  rv = ext_flash_open();

  if(!rv) {
    printf("Could not open flash to save data\n");
    ext_flash_close();
    return;
  }

  rv = ext_flash_erase(CONFIG_FLASH_OFFSET, sizeof(struct auth_data));

  if(!rv) {
    printf("Error erasing flash\n");
  } else {

    rv = ext_flash_write(CONFIG_FLASH_OFFSET, sizeof(auth_data),
                         (uint8_t *)&new_data);
    if(!rv) {
      printf("Error saving data\n");
    }
  }

  ext_flash_close();
}


static void
load_data()
{
  /* Read from flash into saved_data */
  int rv = ext_flash_open();

  if(!rv) {
    printf("Could not open flash to load data\n");
    ext_flash_close();
    return;
  }

  rv = ext_flash_read(CONFIG_FLASH_OFFSET, sizeof(tmp_cfg),
                      (uint8_t *)&saved_data);

  ext_flash_close();

  if(!rv) {
    printf("Error loading data\n");
    return;
  }

}

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
AUTOSTART_PROCESSES(&resolv_process,&er_example_server);

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
  etimer_set(&5min_etimer, CLOCK_SECOND * 300);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == sensors_event && data == &button_sensor) {
      PRINTF("*******BUTTON*******\n");

      /* Call the event_handler for this application-specific event. */
      res_event.trigger();

    }
    else if(etimer_expired(&5min_etimer)){



    }

 } /* while (1) */

  PROCESS_END();
}
