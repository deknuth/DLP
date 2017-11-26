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
#define BAUD 9600

#define MOT_N	{ PORTC|=0x10; PORTC&=~0x20; }		// 左转
#define MOT_P	{ PORTC&=~0x10; PORTC|=0x20; }		// 右转
#define MOT_STOP	{ PORTC&=~0x10; PORTC&=~0x20; }
#define LED_BLINK	PORTD^=0x10
#define LED_OFF	PORTD|=0x10
#define LED_ON	PORTD&=~0x10

void PortInit(void) 
{
	DDRB =  0B00000110;
	PORTB = 0B00000000;
	PINB =	0x00;

	DDRC =  0B00110110;		// PC1(GSM_PW),PC2(GSM_RST)
	PORTC = 0B00000000;
	PINC =	0x00;
	
	DDRD =  0B00010000;		// PD4(LED)
	PORTD = 0B00010000;
	PIND =	0x00;
}

void IntInit(void)
{
    EICRA = 0x0F;
	EIMSK = 0x03;
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
		LED_ON;
		_delay_ms(300);
		LED_OFF;
		_delay_ms(300);
	}
	LED_OFF;
}

volatile unsigned char flag = 0;
volatile unsigned char key = 0;
ISR(INT0_vect)		// 上锁
{
	key = 1;
}

ISR(INT1_vect)
{
	if(flag++ > 200)
		flag = 200;
}

unsigned char word = 0;
volatile unsigned char start = 0;
volatile unsigned char zLen = 0;
volatile unsigned char U0Ready = 0;
unsigned char U0Buf[64] = {0};
volatile unsigned char U0Count = 0;

ISR(USART_RX_vect)
{
	LED_BLINK;
	word =UDR0;
	if(word == 0xFE)
	{
		if(!U0Ready)
			start = 1;
	}
	if(start)
	{
		U0Buf[U0Count++] = word;
		if(U0Count == 4)
		{
			U0Ready = 1;
			start = zLen = 0;
		}
	}
}

int main(void)
{
	cli();
	PortInit();
	UartInit();
	IntInit();
	sei();
	BLink(2);
	_delay_ms(200);
	PORTC|=0x02;
	_delay_ms(2000);
	PORTC&=~0x02;
	
	for(;;)
	{
		if(U0Ready)
		{
			U0Ready=U0Count=0;
			if(PIND&0x04)
			{
				key = 0;
				MOT_P;
			}
		}
		if(flag)
		{
			_delay_ms(1000);
			MOT_STOP;
			BLink(3);
			flag = 0;
		}
	}
}
