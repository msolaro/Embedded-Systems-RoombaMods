/** CYBOT 9
 * 
 * IR functionality
 *
 *
 * @author Max Solaro
 * @date 08/14/2016
 */

#include "uart.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_uart.h"
#include "open_interface.h"
#include "movement.h"



// Functions for communiticate between CyBot and Putty (via UART)
// PuTTy: Buad=115200, 8 data bits, No Flow Control, No Party,  COM1



//int main(void)
//{
//    uart_init();
//    lcd_init();
//    ADC_init();
//    float voltage=0;
//    int distance =0;
//
//
//    calibrate();
//    while(1)
//    {
//
//    voltage =ADC_read2(0);
//    distance = IR_Convert(voltage);
//
//
//    lcd_printf("distance: %d cm,   voltage: %f",distance,voltage/1000);
//    timer_waitMillis(500);
//    }
//
//}
   void ADC_init()
   {

       //enable ADC 0 module on port D
       SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;
       //enable clock for ADC
       SYSCTL_RCGCADC_R |= 0x00000001;
       //enable port D pin 0 to work as alternate functions
       GPIO_PORTD_AFSEL_R |= 0x01;
       //set pin to input - 0
       GPIO_PORTD_DEN_R &= 0b11111110;
       //disable analog isolation for the pin
       GPIO_PORTD_AMSEL_R |= 0x01;
       //initialize the port trigger source as processor (default)
       GPIO_PORTD_ADCCTL_R = 0x00;

       //disable SS0 sample sequencer to configure it
       ADC0_ACTSS_R &= 0x11111110;
       //initialize the ADC trigger source as processor (default)
       ADC0_EMUX_R = 0x00000000  ;
       //set 1st sample to use the AIN10 ADC pin
       ADC0_SSMUX0_R |= 0x000A;
       //enable raw interrupt status
       ADC0_SSCTL0_R |= (ADC_SSCTL0_IE0 | ADC_SSCTL0_END0);
       //enable oversampling to average
       ADC0_SAC_R |= 0x00000006;
       //re-enable ADC0 SS0
       ADC0_ACTSS_R |= 0x00000001;



   }
   int ADC_read()
   {
       ADC0_PSSI_R=ADC_PSSI_SS1;
       //initiates ss1 conversion
       while((ADC0_RIS_R & ADC_RIS_INR0) ==0)
       {//wait for adc conversion to be completed

       }

       int value =0;//return result
       value = ADC0_SSFIFO1_R;

       // clears interupt
     //  ADC0_ISC_R=ADC_ISC_IN1;


       return value;
   }

   unsigned ADC_read2(char channel){


        //disable ADC0SS0 sample sequencer to configure it
        ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN0;
        //set 1st sample to use the channel ADC pin
        ADC0_SSMUX0_R |= channel;
        //re-enable ADC0 SS0
        ADC0_ACTSS_R |= ADC_ACTSS_ASEN0;
        //initiate SS0 conversion
        ADC0_PSSI_R = ADC_PSSI_SS0;
        //wait for ADC conversion to be complete
        while ((ADC0_RIS_R & ADC_RIS_INR0) == 0)
        {
        }
        //clear interrupt
        ADC0_ISC_R = ADC_ISC_IN0;

    return ADC0_SSFIFO0_R;
}

   int IR_Convert(float voltage)
   {
       //TODO
      float distance=0;
      voltage = voltage/1000.00000;
      distance = 1/((voltage-0.1226)/28.372);
      if(distance <0)
          return distance*-1;
      return distance;



   }
//CYBOT 9
   void calibrate(void)
   {
    uart_init();
    ADC_init();
    int i=0;
    int distance = 70;
    char voltage;
    char buffer[300];

    for (i = 0; i < 3; i++)
    {
        oi_t *sensor_data = oi_alloc();
        oi_init(sensor_data);

        move_forward(sensor_data, 5);

        voltage = (char)ADC_read2(0);
        distance = distance - 5;
        oi_free(sensor_data);
        //  timer_waitMillis(1000);
//       uart_sendChar(voltage);
//        snprintf(buffer, sizeof(buffer), "%f\n",voltage);
//       uart_sendStr(buffer);
     //   lcd_printf("Voltage: %f\n",voltage);
        //-----------------------------------------


    }



}
