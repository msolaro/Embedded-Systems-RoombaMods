/*
*
*   uart.c
*
*
*
*
*
*   @author
*   @date
*/

#include "uart.h"
#include "button.h"
void uart_init(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;

    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;

    GPIO_PORTB_AFSEL_R |= (BIT0 | BIT1);
    //enables alternate functions on port b pins 0 and1

    GPIO_PORTB_PCTL_R |= 0x00000011;
    //enable Rx and Tx on port B on pins 0 and 1

    GPIO_PORTB_DEN_R |= (BIT0 | BIT1);
    //set pin 0 and 1 to digital

    GPIO_PORTB_DIR_R &= ~BIT0;
    // sets pin 0 to Rx or input

    GPIO_PORTB_DIR_R |= BIT1;
    //sets pin 1 to Tx or output

    uint16_t iBRD = 8; //double check these numbers //update for part 4
    uint16_t fBRD = 44;
    UART1_CTL_R &= ~(UART_CTL_UARTEN);
    //turns off uart 1

    UART1_IBRD_R = iBRD;
    UART1_FBRD_R = fBRD;
    //setting the buad rates

    UART1_LCRH_R = UART_LCRH_WLEN_8;
    //sets fram,  data bts, 1 stop bit, no parity, no FIFO

    UART1_CC_R = UART_CC_CS_SYSCLK;
    //uses system clock as source

    UART1_CTL_R = (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN);
    // re-enable RX Tx and uart1
}

void uart_sendChar(char data){

    while(UART1_FR_R & 0x20)
    {
        //checks if fifo is ready to recieve

    }
    UART1_DR_R= data;
}

int uart_receive(void){

    char data = 0;

    while(UART1_FR_R & UART_FR_RXFE)
    {
        uint8_t button = button_getButton();
        if(button!=0)
        {
            return '\0';
        }
    }

    data = (char)(UART1_DR_R & 0xFF);
    return data;
}

void uart_sendStr(const char *data){
	//TODO for reference see lcd_puts from lcd.c file

}
