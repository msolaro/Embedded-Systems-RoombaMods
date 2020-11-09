/*
 * servo.h
 *
 *  Created on: Oct 23, 2018
 *      Author: msolaro
 */

#ifndef SERVO_H_
#define SERVO_H_
#define LAB_SECTION 2       //1 = part1, 2 = part2

void move_servoButton(int direction,double degrees);
//moves the servo in a direction a certain amount of degrees
void move_servo(int direction, double degrees);

void servo_init(void);
//initializes servo

void TIMER1B_Handler(void);
//handler

//int turnDegrees(double degrees);

void sweep(int direction);
//volatile unsigned int period_width;// = 320000;
//void part1(void);
//void part2(void);
//void timer1_init(void);
#endif /* SERVO_H_ */
