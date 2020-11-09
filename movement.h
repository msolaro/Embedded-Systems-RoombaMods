/*
 *
 * movement.h
 *
 * contains functions for preset movement paths
 * @author Max Solaro
 * @author Eric Stablein
 * @date 9/4/2018
 *
 *
 *
 */


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Timer.h"
#include "lcd.h"
#include <inc/tm4c123gh6pm.h>
#include "open_interface.h"


void move_forward(oi_t *sensor, int centimeter);
//moves the roomba forward x centimeters

void move_backward(oi_t *sensor, int centimeter);
//moves the roomba backwards x centimeters

void turn_counterclockwise(oi_t *sensor, int degrees);
//turns the roomba x degrees counterclockwise

void turn_clockwise(oi_t *sensor, int degrees);
//turns the roomba clockwise x degrees

void moveRectangle_counterclockwise(oi_t *sensor, int centimeter);
//moves the roomba in a rectangle with each side being x centimeters.




