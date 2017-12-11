/*
 * flash-head.c
 *
 *  Created on: 09.12.2017
 *      Author: EnizM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "net/rpl/rpl.h"
#include "flash-head.h"

#define CONFIG_FLASH_OFFSET 0
#define LED_TOGGLE_INTERVAL 		(CLOCK_SECOND * 0.5)

static struct etimer alarm_timer;

void set_init_data(){
	data_in_flash.is_armed = 0;
	data_in_flash.is_pin_default = 1; //pin is default = 1
	data_in_flash.default_pin = 1234;
}


void save_data(void)
{
  #if BOARD_SENSORTAG || BOARD_LAUNCHPAD
  /* Dump current running config to flash */

  int rv;

  rv = ext_flash_open();

  if(!rv) {
    printf("Could not open flash to save data\n");
    ext_flash_close();
    return;
  }

  rv = ext_flash_erase(CONFIG_FLASH_OFFSET, sizeof(struct AuthData));

  if(!rv) {
    printf("Error erasing flash\n");
  } else {

	printf("New secret = %d set\n", data_in_flash.secret);

    rv = ext_flash_write(CONFIG_FLASH_OFFSET, sizeof(data_in_flash),
                         (uint8_t *)&data_in_flash);
    if(!rv) {
      printf("Error saving data\n");
    }
  }

  ext_flash_close();
  #endif
}


void load_data(void)
{
  #if BOARD_SENSORTAG || BOARD_LAUNCHPAD
  /* Read from flash into saved_data */
  int rv = ext_flash_open();

  if(!rv) {
    printf("Could not open flash to load data\n");
    ext_flash_close();
    return;
  }

  rv = ext_flash_read(CONFIG_FLASH_OFFSET, sizeof(data_in_flash),
                      (uint8_t *)&data_in_flash);


  printf("Old secret = %d\n", data_in_flash.secret);

  ext_flash_close();

  if(!rv) {
    printf("Error loading data\n");
    return;
  }
  #endif

}

void turn_on_alarm()
{
	load_data();
	buzzer_start(1000);
	data_in_flash.is_alarm_on =1;
	etimer_set(&alarm_timer, LED_TOGGLE_INTERVAL);
	save_data();
}

void turn_off_alarm()
{
	load_data();
	buzzer_stop();
	data_in_flash.is_alarm_on = 0;
	save_data();
}
