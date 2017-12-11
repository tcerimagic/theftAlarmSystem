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
	char *default_pin;
	char *user_pin;
	int secret;
	int is_armed;
	int is_pin_default;
	int is_alarm_on;
};

struct AuthData data_in_flash;

#endif /* THEFTALARMSYSTEM_COAP_EXAMPLE_FLASH_HEAD_H_ */
