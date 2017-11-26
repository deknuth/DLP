#include "main.h"


void PortInit(void)
{
    DDRB = 0B00001110;
    PORTB= 0B00001110;
    PINB = 0x00;

    DDRD = 0B11101000;
    PORTD= 0B11100000;
    PIND = 0x00;

    DDRC = 0B00000001;
    PORTC= 0B00110000;
    PINC = 0x00;
}

void UartInit(void)
{
    UBRR0H = (F_CPU / BAUD / 16 - 1) / 256;
    UBRR0L = (F_CPU / BAUD / 16 - 1) % 256;
    UCSR0B = 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0;
    UCSR0C = 1<<UCSZ00 | 1<<UCSZ01;
}

void SendStr(unsigned char* data,unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        while(!(UCSR0A & (1 << UDRE0)));
        UDR0 = *(data++);
    }
}

unsigned char word = 0;
volatile unsigned char start = 0;
volatile unsigned char zLen = 0;
volatile unsigned char U0Ready = 0;
unsigned char U0Buf[64] = {0};
volatile unsigned char U0Count = 0;

ISR(USART_RX_vect)
{
	word =UDR0;
	if(word == 0xFE)
	{
		if(!U0Ready)
			start = 1;
	}
	if(start)
	{
		U0Buf[U0Count++] = word;
		if(U0Count == zLen)
		{
			U0Ready = 1;
			start = zLen = 0;
		}
		else if(U0Count == 2)
		{
			if(word < 19)		// 协议最大长度
				zLen = word;
			else
			{
				U0Count = 0;
				start = 0;
			}
		}
	}
}


unsigned char buf[MAX] = {0};
unsigned char rLen = 0;

void TWIRecv(void)
{
    trReady = 1;
	memcpy(buf,trBuf,trLen);
    rLen = trLen;
    TWIReset();
}

int main(void)
{
	unsigned char temp[2] = { 0x08 };
	cli();
    PortInit();
    UartInit();
    TWIInt();
    sei();
	SendStr("AB",2);
	_delay_ms(20);
	TWISend(temp,1);
	while(1)
    {		
		if(U0Ready)
		{
			U0Ready = U0Count = 0;
		}
		if(trReady)
		{
			SendStr(buf,rLen);
		}
	}

}