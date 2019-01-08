/*
 * Settings.h
 *
 *  Created on: Oct 30, 2017
 *      Author: Andrea Graziani
 */

#ifndef FILE_SETTINGS_H_
#define FILE_SETTINGS_H_

typedef struct settings {

	unsigned char sliding_window_size;
	unsigned char loss_probability;
	unsigned char auto_timeout;
	unsigned long fixed_timeout;

} Settings;

void get_application_settings(Settings *Settings_obj);
void save_application_settings(Settings *Settings_obj);


#endif /* FILE_SETTINGS_H_ */
