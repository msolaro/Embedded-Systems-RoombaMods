
/**
 * clock-intr.c: timer- and interrupt-based clock program
 * This is skeleton code. You need to make it complete.
 *
 * @author James Talbert, Zach Glanz, Zhao Zhang
 * @date 05/24/2018
 */

#include "limits.h"     //Used to initialize min to INT_MAX in part 2, not super needed
#include "math.h"       //Used to calculate cosine of an angle in part 2

#include "button.h"
#include "cyBot_uart.h" // Functions for communiticate between CyBot and Putty (via UART)
#include "open_interface.h"

#include <stdint.h>
#include "timer.h"
#include "lcd.h"
#include <stdbool.h>
#include "driverlib/interrupt.h"

#include "cyBot_uart.h" // Functions for communiticate between CyBot and Putty (via UART)
// PuTTy: Buad=115200, 8 data bits, No Flow Control, No Party,  COM1
#include "cyBot_Scan.h" // For scan sensors

//#include "servo.h"
#include "ping.h"
#include "ir.h"


volatile unsigned int period_width = 320000; //clock ticks for 20 ms

void timer1_init()
{
    //SYSCTL_RCGCGPIO_R = 0x32;         //Enable port E for buttons
    SYSCTL_RCGCGPIO_R = 0x3F;           //Can't tell what port lcd is on so I just put F

    GPIO_PORTB_DEN_R |= 0x20;
    GPIO_PORTB_PCTL_R |= 0x700000;
    GPIO_PORTB_AFSEL_R |= 0x20;
    GPIO_PORTB_DIR_R |= 0x20;

    //***set GPIO PB5, turn on clk, alt. function, output, enable***
    SYSCTL_RCGCTIMER_R |= 0x2;                      //Enable clock to GPIO port B
    TIMER1_CTL_R &= ~0x100;                         //disable timer to config
    //-------------------------------------------------------

    TIMER1_TBMR_R &= 0xFFFFF000;                    //Clearing bytes 2:0
    TIMER1_TBMR_R |= 0xA;                           //Setting TBMR configs      0x082A
    // Periodic, edge count, PWM enable, down count, match interupt, drives high on zero

    TIMER1_CFG_R &= 0xFFFFFFF8;                     //Clearing bits 2:0
    TIMER1_CFG_R |= 0x4;                            //Setting timer to 16 bit mode

    TIMER1_TBILR_R = period_width & 0xFFFF;         //lower 16 bits of the interval (set period)
    TIMER1_TBPR_R = period_width >> 16;             //set the upper 8 bits of the interval (time prescalar)

    //TIMER1_TBPMR_R = 0;                             // set match prescaler
    //TIMER1_TBMATCHR_R = 0xFF00;                     // set value for initial interrupt


    TIMER1_ICR_R |= 0x800;                          //clear interrupts
    TIMER1_IMR_R &= 0xFFFEF0E0;                     //Clearing all but RES bits
    TIMER1_IMR_R |= 0x800;                          //enable match interrupts


    TIMER1_TBMATCHR_R = 0x0;

    TIMER1_CTL_R |= 0x100;                           //enable timer
}

int turnDegrees(double degrees){
    unsigned int pulse_width = 0;                    //pulse width in cycles

    //180 degrees is 30,000 ticks: 0 == 283000, 180 = 313000     (using degrees * 150, cant think to see if it matters)
    pulse_width = (int)(degrees * 155) + 283000;

    TIMER1_TBMATCHR_R = pulse_width & 0x0000FFFF; //set pulse width
    TIMER1_TBPMR_R = pulse_width >> 16;           // set match prescaler

    return 1;
}
/*------------------------------------------------------------*/

void calib();
void part1();
void part2();

#define LAB_SECTION 2       //1 = part1, 2 = part2

int main(void){

    switch(LAB_SECTION){
    case 1: part1();
    break;
    case 2: part2();
    break;
    default: calib();
    break;
    }
}


/*------------------------------------------------------------*/



void part1(){
    lcd_init();
    uart_init();

    GPIO_init();
    PING_init();
    //servo_init();
    //calibrate();
    ADC_init();


    button_init();
    timer1_init();

    uint8_t button;
    double degrees = 90;        //Defaults to 90
    int direction = 1;          //Turn right initially

    float voltage = 0;
    int ir_distance = 0;

    char buffer[300];


    uart_sendStr("Degrees       IR Distance (cm)        Sonar Distance (cm)\n\n");

    while(1){
        turnDegrees(degrees);

        voltage = ADC_read2(0);
        ir_distance = IR_Convert(voltage);


        //IntRegister(INT_TIMER3B, TIMER3B_Handler);
        int distance = ping_read() & 0xFFFFFFFF;

        /*
        lcd_printf(
                "Degrees:   %s\n"         //"  degrees->"   OR   "<-degrees  "
                +"Ping Dist: %d\n"
                +"IR Dist:   %d\n",
                ((direction > 0) ? "  "+ degrees +"->" : "<-"+ degrees +"  "), distance, ir_distance);
         */

        snprintf(buffer, sizeof(buffer), "%-3.0lf           %-5d                   %-5d\n", degrees, distance, ir_distance);
        uart_sendStr(buffer);
        //lcd_printf("Degrees:   %.0lf\nPing Dist: %d\nIR Dist:   %d\n", degrees, distance, ir_distance);

        //lcd_printf("Degrees: %.2lf\nDirection: %s", degrees, ((direction > 0) ? "->" : "<-"));


        degrees = degrees + (direction * 2);

        if (degrees >= 180){
            degrees = 180;
            direction *= -1;
        }
        else if (degrees <= 0){
            degrees = 0;
            direction *= -1;
        }

        timer_waitMillis(180);
    }
}


void part2(){
    lcd_init();
    uart_init();

    GPIO_init();
    PING_init();
    ADC_init();

    timer1_init();




    double degrees = 0;         //Start at 0, turn right
    double degreesToTurn = 1;

    float voltage = 0;
    int ir_distance = 0;
    int ping_distance = 0;

    int last_ir_distance = 0;   //In case we want to average the first
    int last_ping_distance = 0; // and last values of an object for higher precision


    uart_sendStr("Degrees       IR Distance (cm)        Sonar Distance (cm)\n\n");
    turnDegrees(degrees);       //Turn to 0 initially so there are no missed measurements
    timer_waitMillis(1000);      //Wait so servo reaches position




    int objectCount = 0;        //Count of objects encountered

    //#WARNING VERY close to memory limit, if this was an int array it would crash
    unsigned short distanceArray[3][90];   //3D array to hold both IR and Ping values, as well as their determined angle from 0 (As the servo increments by 2 degrees, 180/2 = 90)
    int foundObjectFlag = 0;

    char buffer[300];


    //Commence scan
    while(degrees <= 186){      //Start at 0, stop at 180
        turnDegrees(degrees);

        int avg;
        for(avg = 0; avg < 10; avg++){
            voltage = ADC_read2(0);
            ir_distance += IR_Convert(voltage);                          //Get IR distance
        }
        ir_distance /= (avg+1);                                           //Taking an average



        ping_distance = ping_read() & 0xFFFFFFFF;                   //Get Ping distance

        //(Using IR only because I don't trust Ping)
        //Test environment stops at 80cm out
        if(!foundObjectFlag && ir_distance < 80){                   //If an object is detected...
            distanceArray[0][objectCount * 2] = ir_distance;
            distanceArray[1][objectCount * 2] = ping_distance;
            distanceArray[2][objectCount * 2] = degrees;

            foundObjectFlag = 1;
        }
        else if(foundObjectFlag && ir_distance > 80){               //The object has been passed
            distanceArray[0][objectCount * 2 + 1] = last_ir_distance;     //(Current distances are now invalid)
            distanceArray[1][objectCount * 2 + 1] = last_ping_distance;   //(Use previous ones)   [Not super necessary, currently unused]
            distanceArray[2][objectCount * 2 + 1] = degrees - degreesToTurn;              //Last degree measure
            objectCount++;

            foundObjectFlag = 0;
        }



        //Send information to PuTTY----------------
        snprintf(buffer, sizeof(buffer), "%-3.0lf           %-5d                   %-5d\n", degrees, ir_distance, ping_distance);
        uart_sendStr(buffer);
        //-----------------------------------------

        last_ir_distance = ir_distance;
        last_ping_distance = ping_distance;

        degrees += degreesToTurn;                       //Degrees increments by degreesToTurn

        //Honestly 20 is pretty stable but JIC
        timer_waitMillis(100);               //Seems stable enough idk
        //timer_waitMillis(180);
    }


    //After scan is complete...

    double min = LONG_MAX;   //Current smallest object
    double min_dist = 0;
    double min_degree = 0;  //Degree measure of found min (Could technically use index to look back in array)
    double min_cover = 0;
    int min_index = 0;      //Index of smallest object

    //Temp variables for calculations
    double currentDist;
    double currentDist2;
    double currentAngle;
    double width;

    double testAngle;

    int numErrors = 0;      //Used to subtract from the index at the end, removes fake objects (Like glitches)

    int i;
    for(i = 0; i < objectCount; i++){             //Loop through the saved values

        currentAngle = distanceArray[2][i*2 + 1] - distanceArray[2][i*2];   //Get relative angle

        if(currentAngle == 0){                      //Object too small, possible error spike
            numErrors++;
            continue;
        }

        currentDist = distanceArray[0][i*2];        //First saved distance
        currentDist2 = distanceArray[0][i*2 + 1];   //Second saved distance


        //Law of Cosines: a^2 = b^2 + c^2 - 2bc*cos(A)
        //width = pow(currentDist, 2) + pow(currentDist2, 2) - (2 * currentDist*currentDist2*cos(currentAngle));
        //width = (pow(currentDist, 2)*2) - (2 * pow(currentDist, 2) *cos(currentAngle));
        // / 1.7068
        //double angleeeee = currentAngle * (3.14159265358979323846264338/180.0); //radians
        double angleeeee = currentAngle * .0174532925;  // <- == ^
        //width = currentDist * cos(0) - (currentDist * cos(angleeeee));
        currentDist = (currentDist + currentDist2) / 2;     //Find average of two distances
        width = currentDist * currentAngle * .0174532925; //(.017 == pi/180)

        //width = (pow(currentDist, 2)*2) - (4*currentDist*cos(currentAngle));        //Using first distance
        //width = sqrt(width);

        if(width < min){                                    //If old small now big...
            min = width;                                    //Make old small equal new small
            //min_dist = (double)(currentDist + currentDist2) / 2;    //Avg. distance
            min_dist = currentDist;
            min_degree = distanceArray[2][i*2 + 1] - currentAngle/2;                    //Right in the middle
            min_index = i - numErrors + 1;                      //Find correct index in case of glitch
            //min_cover = angleeeee;
            min_cover = currentAngle;

            testAngle = currentAngle;
        }
    }

    //min degree
   // min_degree= 180-min_degree;

    //snprintf(buffer, sizeof(buffer), "Index: %d\nDistance: %.3lf cm\nWidth: %.3lf cm\nDegree: %.0lf", min_index, min_dist, min, min_degree);
    snprintf(buffer, sizeof(buffer), "Index: %d\nDistance: %.1lf cm\nWidth: %.3lf cm\nAngle: %.3lf", min_index, min_dist, min, min_cover);// instead of min_cover use min_degree
    lcd_printf(buffer);

    turnDegrees(min_degree);                        //Turn back to the smallest object

    while(1){
        //End
    }
}

volatile double ADC;
volatile double distance;
volatile int timer = 0;;

void calib(){
    lcd_init();
    uart_init();

    GPIO_init();
    PING_init();
    ADC_init();

    timer1_init();




    double degrees = 0;         //Start at 0, turn right

    float voltage = 0;
    int ir_distance = 0;
    int ping_distance = 0;

    int last_ir_distance = 0;   //In case we want to average the first
    int last_ping_distance = 0; // and last values of an object for higher precision

    char buffer[300];

    turnDegrees(90);
    timer_waitMillis(1000);

    while(1){
        voltage = ADC_read2(0);
        ir_distance = IR_Convert(voltage);

        ping_distance = ping_read() & 0xFFFFFFFF;                   //Get Ping distance

        snprintf(buffer, sizeof(buffer), "IR:   %d\nPing: %d", ir_distance, ping_distance);
        lcd_printf(buffer);

        timer_waitMillis(500);
    }
}


