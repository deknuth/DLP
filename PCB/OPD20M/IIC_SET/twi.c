#include "main.h"

volatile unsigned char trLen = 0;
volatile unsigned char tsLen = 0;
volatile unsigned char tsCount = 0;
volatile int tsTicks = 0;				// twi send time out flag
unsigned char trBuf[MAX];
unsigned char tsBuf[MAX];
volatile unsigned char 	trReady = 0;
volatile unsigned char TWIState;

void TWIInt(void)
{
    DDRD &= ~(1 << PC5) | (1 << PC4);
    TWCR = 0x00;
    TWBR = TWBR_SET;	// set speed
    TWSR = 0x00;
    TWAR = TR_ADD;		// convert to slave read mode and open broadcast
    TWIState = IDLE;
    TWCR = 1<<TWEN | 1<<TWEA | 1<<TWIE;
    _delay_us(30);
}

void TWIReset(void)
{
    trLen = 0;
    TWCR = TW_ACK;
}

ISR(TWI_vect)
{
    switch (TWSR & 0xf8)
    {
    /*********** master send part *************/
    case TW_START:			// 0x08 start flag send over
        TWDR = TS_ADD;		// TW send address and write bit SLA+W
        TWCR = TW_ACK;
        break;

    case TW_REP_START:		// 0x10 repeated START condition
        TWDR = TS_ADD;
        TWCR = TW_ACK;
        break;

    case TW_MT_SLA_ACK:			// 0x18 address send over
        TWDR = tsBuf[tsCount++];
        tsLen--;
        TWCR = TW_ACK;
        break;

    case TW_MT_SLA_NACK:		// 0x20 address send failed receive not ack
        if(tsTicks++ > 5)		// timeout test times
        {
            TWIState = ERROR;
            TWCR = TW_STO;
        }
        else
            TWCR = TW_STA;
        break;

    case TW_MT_DATA_ACK:		// 0x28 receive interrupt
        if(tsLen)				// not send over
        {
            TWDR = tsBuf[tsCount++];
            tsLen--;
            TWCR = TW_ACK;
        }
        else					// data send over->set stop
        {
            TWCR = TW_STO;
            TWIState = IDLE;
        }
        break;

    case TW_MT_DATA_NACK:	// 0x30
        if(tsLen)			// not send over
        {
            TWDR = tsBuf[tsCount++];
            tsLen--;
            TWCR = TW_ACK;
        }
        else
        {
            TWIState = IDLE;
            TWCR = TW_STO;
        }
        break;

        /*********** slave receive part ***********/
    case TW_SR_SLA_ACK:		// 0x60
        TWIState = BUSY;		// contorl TWI bus
        TWCR = TW_ACK;
        break;

    case TW_SR_ARB_LOST_SLA_ACK:	// 0x68
        TWCR = TW_ACK;
        break;

    case TW_SR_GCALL_ACK:	// 0x70
        TWIState = BUSY;		// contorl TWI bus
        TWCR = TW_ACK;
        break;

    case TW_SR_ARB_LOST_GCALL_ACK:		// 0x78
        TWCR = TW_ACK;
        break;

    case TW_SR_DATA_ACK:	// 0x80
        trBuf[trLen++] = TWDR;
        if(trLen == MAX)			// buf full
            TWCR = TW_NACK;
        else
            TWCR = TW_ACK;
        break;

    case TW_SR_DATA_NACK:	// 0x88
        TWCR = TW_SAO;
        TWIRecv();
        break;

    case TW_SR_GCALL_DATA_ACK:	// 0x90
        TWCR = TW_ACK;
        break;

    case TW_SR_GCALL_DATA_NACK:	// 0x98
        TWCR = TW_ACK;
        break;

    case TW_SR_STOP:			// 0xA0 slave receive over
        TWIState = IDLE;		// release TWI bus
        TWCR = TW_NACK;
        TWIRecv();
        break;

        /************** other *************/
    case TW_MT_ARB_LOST:	// 0x38 ÖÙ²ÃÊ§°Ü
        TWCR = TW_STA;		// ÖØ·¢
        break;

    default:
        TWCR = TW_ACK;
        break;
    }
}

unsigned char TWISend(unsigned char *buf,unsigned int len)
{
    unsigned int s = 0;
    tsTicks = 0;
    if(len > MAX)
    {
        while(1)
        {
            memcpy(tsBuf,&buf[s],MAX);
            s += MAX;
            tsLen = MAX;
            tsCount = 0;
            while(TWIState%2);	// wait TWI idle
            TWIState = BUSY;
            TWAR = 0x00;
            TWCR = TW_STA;		// start TWI
            while(TWIState%2);	// wait send over
    //        _delay_us(2);
            if(s+MAX > len)
                break;
        }
        if(s < len)
        {
            tsLen = len - s;
            memcpy(tsBuf,&buf[s],tsLen);
            tsCount = 0;
            while(TWIState%2);	// wait TWI idle
            TWIState = BUSY;
            TWAR = 0x00;
            TWCR = TW_STA;		// start TWI
            while(TWIState%2);	// wait send over
        }
    }
    else
    {
        memcpy(tsBuf,buf,len);
        tsLen = len;
        tsCount = 0;
        while(TWIState%2);	// wait TWI idle
        TWIState = BUSY;
        TWAR = 0x00;
        TWCR = TW_STA;		// start TWI
        while(TWIState%2);	// wait send over
    }
    TWAR = TR_ADD;			// convert to slave read mode and open broadcast
    TWCR = TW_ACK;
    return TWIState;
}
