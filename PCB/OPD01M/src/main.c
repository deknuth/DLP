#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <math.h>
#include <util/delay.h>
#include <util/twi.h>
#include "main.h"
#define BAUD 115200
#define A_LED_ON	PORTC|=(1<<1)
#define A_LED_OFF	PORTC&=~(1<<1)

#define B_LED_ON	PORTD|=(1<<4)
#define B_LED_OFF	PORTD&=~(1<<4)

void PortInit(void) 
{
	DDRB =  0B00000110;
	PORTB = 0B00000000;
	PINB =	0x00;

	DDRC =  0B00000110;		// PC1(GSM_PW),PC2(GSM_RST)
	PORTC = 0B00000000;
	PINC =	0x00;
	
	DDRD =  0B00010000;		// PD4(KEY1)
	PORTD = 0B00000000;
	PIND =	0x00;
}
void UartInit(void)
{
    UBRR0H = (F_CPU / BAUD / 16 - 1) / 256;
    UBRR0L = (F_CPU / BAUD / 16 - 1) % 256;
    UCSR0B = 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0;
    UCSR0C = 1<<UCSZ00 | 1<<UCSZ01;
}
void U0Send(unsigned char* data,unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        while(!(UCSR0A & (1 << UDRE0)));
        UDR0 = *(data++);
    }
}
void BLink(unsigned char times)
{
	int i = 0;
	for(i=0;i<times;i++)
	{
		A_LED_ON;
		_delay_ms(400);
		A_LED_OFF;
		_delay_ms(400);
	}
}

char AT_INS[6][64] = {
	"AT+DSCADDR=0,\"TCP\",\"120.78.144.255\",8888\r",
	"AT&W\r",
	"AT+CFUN=1,1\r",
	"AT+GSN\r",
	"ATO\r",
	"+++"
};

enum at{ 
	CON_SER, 
	SAVE, 
	RESTART, 
	GET_MAC,
	DATA_MODE,
	CMD_MODE
};


void InitGSM(void)
{
	U0Send(AT_INS[CON_SER],strlen(AT_INS[CON_SER]));
	_delay_ms(400);
	U0Send(AT_INS[SAVE],strlen(AT_INS[SAVE]));
	_delay_ms(400);
	U0Send(AT_INS[RESTART],strlen(AT_INS[RESTART]));
	_delay_ms(400);
}


 
int main(void)
{
	cli();
	PortInit();
	UartInit();
	_delay_ms(1);
	_delay_ms(6000);
	PORTC|=0x02;
	_delay_ms(2000);
	PORTC&=~0x02;
	for(;;)
	{
		;
	}
}
