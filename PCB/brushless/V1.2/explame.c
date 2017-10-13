#include "main.h"

unsigned char tmpbuf[16] = {0}; //数组用于存放接收到的数据
unsigned int StateLoTable[8] = {0};

#define LED_BLINK	PORTC^=0x01
#define LED_ON	PORTC|=0x01
#define LED_OFF	PORTC&=~0x01

enum MOS{
    APH = (1<<7),
    BPH = (1<<6),
    CPH = (1<<5),
    APL = (1<<1),
    BPL = (1<<2),
    CPL = (1<<3),
    ABCH = 0xE0,
};

#define LMOS_OFF	{ TCCR1A&=0xF; TCCR2A&=0x3F; }	// 关闭下臂所有mos pwm输入
#define HMOS_OFF	PORTD|=ABCH     		// 关闭上臂所有mos
#define AMOS_OFF        { LMOS_OFF; HMOS_OFF; }
#define ALMOS_ON	TCCR1A|=1<<COM1A1
#define BLMOS_ON	TCCR1A|=1<<COM1B1
#define CLMOS_ON	TCCR2A|=1<<COM2A1
#define AHCL	{ AMOS_OFF; asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); PORTD&=~APH; CLMOS_ON; }
#define AHBL	{ AMOS_OFF; asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); PORTD&=~APH; BLMOS_ON; }
#define CHBL	{ AMOS_OFF; asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); PORTD&=~CPH; BLMOS_ON; }
#define CHAL	{ AMOS_OFF; asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); PORTD&=~CPH; ALMOS_ON; }
#define BHAL	{ AMOS_OFF; asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); PORTD&=~BPH; ALMOS_ON; }
#define BHCL	{ AMOS_OFF; asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); PORTD&=~BPH; CLMOS_ON; }
#define DIS_PCINT	PCICR&=~1<<PCIE1
#define ENB_PCINT	PCICR|=1<<PCIE1

#define T0_ON   { TCCR0B=1<<CS02|1<<CS00; TIMSK0|=1<<TOIE0; TCNT0=0; }  // 1024 divide   32.64ms
#define T0_OFF  { TCCR0B=0; TIMSK0&=~1<<TOIE0; }

unsigned char dir = 0;

static void commut(unsigned char phase);        //  commutation

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

void PCInit(void)
{
    PCMSK1 |= 0x0E;
}

void T1Init(void)
{
    TCCR1A = 1<<COM1A1 | 1<<COM1B1 | 1<<WGM10;		// 8
    TCCR1B = 1<<CS10;			//
    OCR1A = OCR1B = 0x40;
}

void T2Init(void)
{
    TCCR2A = 1<<COM2A1 | 1<<COM2B1 | 1<<WGM20;
    TCCR2B = 1<<CS20;
    OCR2A = OCR2B = 0x40;
}

void SetSleep(void)
{
    MCUCR |= 1<<SM1;
    MCUCR |= 1<<SE;
    asm("sleep");
}

volatile unsigned char len = 0;
unsigned char times  = 0;
volatile unsigned int VelInte = 0;  // velocity integral
volatile unsigned int LastValue = 0;
volatile unsigned char stall = 1;

ISR(TIMER0_OVF_vect)
{
    if(times++ > 5)
    {
        if(LastValue == VelInte)
            stall = 1;
        else
        {
            LastValue = VelInte;
            stall = 0;
        }
        times = 0;
    }
    if(VelInte > 60000)
        VelInte = 0;
}

ISR(PCINT1_vect)
{
    VelInte++;
    commut((PINC&0x0E)>>1);
}

int StartFun(unsigned char SetSpeed)
{
	int count = 0;
	unsigned char value = 0;
    unsigned char temp = SetSpeed;
	times = 0;
    T0_OFF;
	value = ((PINC&0x0E)>>1);
	commut(value);
    OCR1A = OCR1B = OCR2A = temp;
    while(1)
    {
     	if(value != ((PINC&0x0E)>>1))
		{
			commut(value);
           	ENB_PCINT;				// Enable PC interrupt
			T0_ON;
			OCR1A = OCR1B = OCR2A = SetSpeed;
			stall = 0;
			return 1;
		}
		value = ((PINC&0x0E)>>1);
		temp -= 1;
        if(temp < 1)
        {
            OCR1A = OCR1B = OCR2A = 0xFF;
			return 0;
        }
        else
            OCR1A = OCR1B = OCR2A = temp;
    }
}

void commut(unsigned char phase)        //  commutation
{
    if(dir)
    {
        switch(phase)
        {
        case 5:
            CHBL;
            break;
        case 4:
            AHBL;
            break;
        case 6:
            AHCL;
            break;
        case 2:
            BHCL;
            break;
        case 3:
            BHAL;
            break;
        case 1:
            CHAL;
            break;
        default:
            break;
        }
    }
    else
    {
        switch(phase)
        {
        case 5:
            BHCL;
            break;
        case 4:
            BHAL;
            break;
        case 6:
            CHAL;
            break;
        case 2:
            CHBL;
            break;
        case 3:
            AHBL;
            break;
        case 1:
            AHCL;
            break;
        default:
            break;
        }
    }
}
volatile unsigned char speed = 155;
unsigned char lastSpeed = 0;
void CBLogicProcess(unsigned char *info,unsigned char iLen)		// core board infomation process
{	
    if(iLen==6 && info[1]==0x06)
    {
        if(info[0]==0xFE && info[5]==0xff)
			speed = info[4];
		if(speed != lastSpeed)
		{
			if(!stall)
				OCR1A = OCR1B = OCR2A = speed;
			lastSpeed = speed;
		}
    }

}

int main(void)
{
	unsigned char buf[MAX] = {0};
    PortInit();
    PCInit();
    sei();
    UartInit();
    T1Init();
    T2Init();
 //   TWIInt();
	_delay_ms(2);
	OCR1A = OCR1B = OCR2A = 0xFF;
	while(1)
    {
        if(stall)
		{
			if(speed > 0)
			{
				while(!StartFun(speed));
			}
		}
		
	/*	
        if(trReady)		// twi receive over
        {
            memcpy(buf,trBuf,trLen);
            len = trLen;
            TWIReset();
            CBLogicProcess(buf,len);
        }
		*/
		
	//	_delay_ms(1200);
	//	dir ^= 0x01;
	//	_delay_ms(1200);
    }

}


