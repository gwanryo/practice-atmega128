/*
 * ultrasound.c
 *
 * Created: 2019-01-22 ���� 11:17:55
 * Author: ryo
 */

#include <mega128.h>
#include <stdint.h>
#include <stdio.h>
#include <delay.h>

#define _ALTERNATE_PUTCHAR_
void putchar(char c){while (!(UCSR0A & (1<<UDRE0)));UDR0=c;}

volatile unsigned int pulse_count = 0;
volatile unsigned int toggle = 0;

// UART0 initialize
// desired baud rate: 9600
// actual baud rate: 9600 (0.0%)
// char size: 8 bit
// parity: Disabled
void uart0_init(void)
{
    UCSR0B = 0x00; //disable while setting baud rate
    UCSR0A = 0x00;
    UCSR0C = 0x06;      // 0x0E;
    UBRR0H = 0x00;      // set baud rate hi
    UBRR0L = 0x67;      // set baud rate lo
    UCSR0B = 0x18;
}

void sonar_init(void)
{
    EIMSK = 0x01;       // INT0 External Interrupt Enable
    EICRA = 0x03;       // INT0 Rising Edge Interrupt Request
    SREG = 0x80;        // Global Interrupt Enable
}

interrupt [TIM0_OVF] void count_pulse(void)
{
    pulse_count++;      // Increase at every 58us
}

interrupt [EXT_INT0] void read_pulse(void)
{
    if(toggle == 0){
        //At Rising Edge
        pulse_count = 0;
        TIMSK = 0x01;   // Time Counter 0 Enable (8-bit Timer/Counter)
                        // 1 / 16 * 10^6 = 0.0000000625s
        TCCR0 = 0x02;   // 0.0000000625s * 8 = 0.5us
        TCNT0 = 140;    // 0.000058 / 0.0000005 = 116
                        // 8bit: 255, (255 - 116 + 1) = 140          
        EICRA = 0x02;   // INT0 Falling Edge Interrupt Request 
        toggle = 1;
    }else{
        //At Falling Edge
        printf("Object is %d cm far from sensor.\r\n", pulse_count);
         
        EICRA = 0x03;   // INT0 Rising Edge Interrupt Request
        toggle = 0;
    }
}


void main(void)
{
    DDRG = 0x03;
    PORTG = 0x01;
    
    DDRD = 0x02;        // Use PortD1 as output
    PORTD = 0x00;
    
    uart0_init();
    sonar_init();
    
    while (1)
    {
        PORTG ^= 0x03;
        
        PORTD = 0x02;
        delay_us(10);
        PORTD = 0x00;
        
        delay_ms(200);
    }
}
