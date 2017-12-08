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
//I2C 状态定义
//MT 主方式传输 MR 主方式接受
#define START    0x08
#define RE_START   0x10
#define MT_SLA_ACK   0x18
#define MT_SLA_NOACK 0x20
#define MT_DATA_ACK   0x28
#define MT_DATA_NOACK 0x30
#define MR_SLA_ACK   0x40
#define MR_SLA_NOACK 0x48
#define MR_DATA_ACK   0x50
#define MR_DATA_NOACK 0x58

#define RD_DEVICE_ADDR 0xA1   //前4位器件固定,后三位看连线,最后1位是读写指令位
#define WD_DEVICE_ADDR 0xA0

//常用TWI操作(主模式写和读)
#define Start()    (TWCR=(1< 
#define Stop()    (TWCR=(1< 
#define Wait()    {while(!(TWCR&(1< 
#define TestAck()   (TWSR&0xf8)          //观察返回状态
#define SetAck    (TWCR|=(1< 
#define SetNoAck   (TWCR&=~(1< 
#define Twi()    (TWCR=(1< 
#define Write8Bit(x) {TWDR=(x);TWCR=(1<


#define TW_ACK		((1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA))
#define TW_NACK		((1<<TWINT) | (1<<TWEN) | (1<<TWIE))
#define TW_STO		((1<<TWINT) | (1<<TWEN) | (1<<TWSTO) | (1<<TWIE) | (1<<TWEA))
#define TW_STA		((1<<TWINT) | (1<<TWEN) | (1<<TWSTA) | (1<<TWIE) | (1<<TWEA))
#define TW_SAO		((1<<TWINT) | (1<<TWEN) | (1<<TWSTA) | (1<<TWSTO) | (1<<TWIE) | (1<<TWEA))




unsigned char I2C_Write(unsigned char Wdata,unsigned char RegAddress);
unsigned char I2C_Read(unsigned RegAddress);
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

unsigned char I2C_Read(unsigned RegAddress)
{
    unsigned char temp;
    Start();//I2C启动
    Wait();
    if (TestAck()!=START)
       return 1;       //ACK   
   
    Write8Bit(WD_DEVICE_ADDR); //写I2C从器件地址和写方式
    Wait();
    if (TestAck()!=MT_SLA_ACK)
       return 1;        //ACK
   
    Write8Bit(RegAddress);   //写器件相应寄存器地址
    Wait();
    if (TestAck()!=MT_DATA_ACK)
       return 1;
   
    Start();           //I2C重新启动
    Wait();
    if (TestAck()!=RE_START)
       return 1;
   
    Write8Bit(RD_DEVICE_ADDR); //写I2C从器件地址和读方式
    Wait();
    if(TestAck()!=MR_SLA_ACK)
       return 1;       //ACK
   
    Twi();        //启动主I2C读方式
    Wait();
    if(TestAck()!=MR_DATA_NOACK)
    return 1;      //ACK
   
    temp=TWDR;//读取I2C接收数据
       Stop();//I2C停止
    return temp;
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