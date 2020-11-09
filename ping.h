/*
 * ping.h
 *
 *  Created on: Oct 23, 2018
 *      Author: msolaro
 */



#ifndef PING_H_
#define PING_H_

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>


int overflow;
unsigned long width;
unsigned long cm;
unsigned long width_time;


void PING_init(void);

void send_pulse(void);

unsigned ping_read(void);

void TIMER3B_Handler(void);

void GPIO_init(void);

#endif /* PING_H_ */
