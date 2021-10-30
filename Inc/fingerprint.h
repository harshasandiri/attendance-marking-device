/*
 * fingerprint.h
 *
 *  Created on: Sep 2, 2017
 *      Author: Harsha
 */

#ifndef FINGERPRINT_H_
#define FINGERPRINT_H_

#include "main.h"
#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "usb_host.h"

#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_sdram.h"


/* Private function prototypes -----------------------------------------------*/
void VERIFY_PWD(void);
void scan_finger(void);
void Img_generation_buffer1(void);
void Img_generation_buffer2(void);
void search_finger_database(void);
void Reg_Model(void);
unsigned char store_template(unsigned char ID);
unsigned char del_template(unsigned char ID);
void Delete_all(void);
void finger_search_gui(void);
void finger_register_gui(unsigned char ID);
void reg_from_computer(void);
void delete_sensor_data_from_pc(void);

#endif /* FINGERPRINT_H_ */
