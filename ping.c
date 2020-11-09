/**
 * ping.c
 * 
 *
 * @author Max Solaro
 * @date 10/23/18
 */

#include "uart.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_uart.h"
#include "open_interface.h"
#include "movement.h"
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "ping.h"



volatile enum{LOW, HIGH, DONE} state;
volatile unsigned rising_time;// start time of the return pulse
volatile unsigned falling_time;// end time of the return pulse


void TIMER3B_Handler(void)
{
    TIMER3_ICR_R |= 0x400; // Clear capture interrupt flag...CBECINT = bit 10, 0b100_0000_0000

    int event_time = TIMER3_TBR_R; // read current event time

    //changes state depending on previous state
    if(state == LOW){
        rising_time = event_time;
        state = HIGH;
    }
    else if(state == HIGH){
        falling_time = event_time;
        state = DONE;
    }
    else{
        state = DONE;
    }
}

void GPIO_init(){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1; //enable clock to GPIO, R1 = port B
    SYSCTL_RCGCGPIO_R |= BIT1;  //turn on PORTB system clock
    GPIO_PORTB_DEN_R |= 0x08;

    GPIO_PORTB_AFSEL_R |= 0x08; // set bit 3 to enable PB3 alternate function
    GPIO_PORTB_PCTL_R |= 0x7000; // set PB3 to T3CCP1 function using PMC3 = 7

}
void PING_init(){
      SYSCTL_RCGCTIMER_R |= BIT3;  //turn on timer

      //Disable Timer3B while setting up
      TIMER3_CTL_R = TIMER_CTL_TBEN & 0xEFF;


      TIMER3_CTL_R |= TIMER_CTL_TBEVENT_BOTH; //Set positive edge (Shoe said to use BOTH)
      TIMER3_CFG_R |= TIMER_CFG_16_BIT; //16-bit mode
      TIMER3_TBMR_R |= TIMER_TBMR_TBMR_CAP; //Capture Mode
      TIMER3_TBMR_R |= TIMER_TBMR_TBCMR; // Edge-Time
      TIMER3_TBMR_R |= TIMER_TBMR_TBCDIR; //Count up

      //Set upper bounds
      TIMER3_TBILR_R = 0xFFFF;
      TIMER3_TBPR_R = 0xFF;

      TIMER3_ICR_R |= 0x400; // Clear capture interrupt flag...CBECINT = bit 10, 0b100_0000_0000
      TIMER3_IMR_R |= 0x400; // Enable event capture interrupts...CBEIM = bit 10, 0b100_0000_0000

      NVIC_PRI9_R |= 0x20; // set bits 7:5 to 0b001
      NVIC_EN1_R |= 0x10; // set bit 4...initialize local interrupts

      IntRegister(INT_TIMER3B, TIMER3B_Handler); //Bind Timer3B interrupt requests to interrupt handler
      IntMasterEnable(); //initialize global interrupts

      TIMER3_CTL_R |= TIMER_CTL_TBEN; //Enable Timer3B
      }

/* send out a pulse on PB3 */
void send_pulse(){
    //turn off alternate functions
    GPIO_PORTB_AFSEL_R &= ~0x08; // clear bit 3 to enable PB3 alternate function
    GPIO_PORTB_PCTL_R &= ~0x7000; // clear PB3 to T3CCP1 function using PMC3 = 7

    TIMER3_IMR_R &= ~0x400; // Disable event capture interrupts

    GPIO_PORTB_DIR_R |= 0x08; // set PB3 as output
    GPIO_PORTB_DATA_R |= 0x08; // set PB3 to high
    timer_waitMicros(5); // wait at least 5 microseconds based on data sheet
    GPIO_PORTB_DATA_R &= 0xF7; // set PB3 to low
    GPIO_PORTB_DIR_R &= 0xF7; // set PB3 as input

    TIMER3_IMR_R |= 0x400; // Enable event capture interrupts

    //look for intial state (LOW)
    state = LOW;

    //turn alternate functions back on
    GPIO_PORTB_AFSEL_R |= 0x08; // set bit 3 to enable PB3 alternate function
    GPIO_PORTB_PCTL_R |= 0x7000; // set PB3 to T3CCP1 function using PMC3 = 7

}

unsigned ping_read(){
    send_pulse();// send the starting pulse to PING


    width = falling_time - rising_time; //difference in clock cycles
    width_time = (width * (62.5/1000000)); //clock speed into ms

    //overflow counter
    if(rising_time > falling_time){
        overflow += 1;
        width += 0xFFFFFF;
    }

    //equation for converting clock cycles to cm
    cm = width * .002125 * .5;

    return cm;
}

