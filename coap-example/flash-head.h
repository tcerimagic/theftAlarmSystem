/*
 * flash-head.h
 *
 *  Created on: 09.12.2017
 *      Author: EnizM
 */

#ifndef THEFTALARMSYSTEM_COAP_EXAMPLE_FLASH_HEAD_H_
#define THEFTALARMSYSTEM_COAP_EXAMPLE_FLASH_HEAD_H_

/*------ FUNctions ---------------------------*/

void save_data(void);
void load_data(void);
void set_init_data(void);
void turn_on_alarm(void);
void turn_off_alarm(void);


/*------ Structures --------------------------*/

struct AuthData{
	int default_pin;
	int user_pin;
	int secret;
	int is_armed;
	int is_pin_default;
	int is_alarm_on;
	int x_axis;
	int y_axis;
	int z_axis;
	int threshold;
};

static struct etimer fmin_etimer;

struct AuthData data_in_flash;

process_event_t arm_device_event;

process_event_t disarm_device_event;

int arm_device;

#endif /* THEFTALARMSYSTEM_COAP_EXAMPLE_FLASH_HEAD_H_ */
