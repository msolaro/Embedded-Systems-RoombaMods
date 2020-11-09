/*
 * ir.h
 *
 *  Created on: Oct 23, 2018
 *      Author: msolaro
 */

#ifndef IR_H_
#define IR_H_

void ADC_init(void);
int ADC_read(void);
unsigned ADC_read2(char channel);
int IR_Convert(float voltage);
void calibrate(void);

float voltages[10];
double distancesCM[10];




#endif /* IR_H_ */
