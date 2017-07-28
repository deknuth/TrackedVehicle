#include "main.h"

unsigned char tmpbuf[16] = {0}; //数组用于存放接收到的数据
unsigned int StateLoTable[8] = {0};

#define LED_BLINK	PORTC^=1<<3

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

#define T0_ON   { TCCR0B=1<<CS02; TIMSK0|=1<<TOIE0; TCNT0=0; }  // 256 divide   8.16ms
#define T0_OFF  { TCCR0B=0; TIMSK0&=~1<<TOIE0; }
#define I0_ON   EIMSK=1<<INT0;
#define I0_OFF   EIMSK&=~(1<<INT0);

volatile unsigned int VelInte = 0;  // velocity integral
volatile unsigned int LastValue = 0;
volatile unsigned char stall = 0;
volatile unsigned int speed = 0;
unsigned char dir = 0;
typedef struct pd{
    unsigned char SetSpeed;	// 设定目标 Desired Value
    double Proportion;	// 比例常数 Proportional Const
    double Integral;	// 积分常数 Integral Const
    double Derivative;	// 微分常数 Derivative Const
    int LastError;		// Error[-1]
    int PrevError;		// Error[-2]
}PID;

PID *sptr;
static void commut(unsigned char phase);        //  commutation

void PortInit(void)
{
    DDRB = 0B00001110;
    PORTB= 0B00001110;
    PINB = 0x00;

    DDRD = 0B11101000;
    PORTD= 0B11100000;
    PIND = 0x00;

    DDRC = 0B00001000;
    PORTC= 0B00000000;
    PINC = 0x00;
}

double Kp = 0.7;
double Ki = 0.4;		// 1.9
double Kd = 0.004;

void IncPIDInit(PID *sptr)
{
    sptr->LastError = 0; 		// Error[-1]
    sptr->PrevError = 0; 		// Error[-2]
    sptr->Proportion = 0.1; 	// Kp Proportional Const
    sptr->Integral = 0.01;		// KiIntegral Const
    sptr->Derivative = 0.01;	// Kd Derivative Const
    sptr->SetSpeed = 80;		// setting
}

int IncPIDCalc(int NextSpeed,PID *sptr)
{
    int iError;
    int value;
    iError = sptr->SetSpeed - NextSpeed;	// 增量计算
    value = Kp * iError - Ki * sptr->LastError + Kd * sptr->PrevError;
    sptr->PrevError = sptr->LastError;
    sptr->LastError = iError;
    return(value);
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

void I0Init(void)
{
    EICRA = 1<<ISC00 | 1<<ISC01;		// 中断0上升沿中断
}

void PCInit(void)
{
    PCICR |= 1<<PCIE1;			// PCINT[14:8] 中断使能
    PCMSK1 |= 0x07;				// 引脚变化中断屏蔽位
}

void T1Init(void)
{
    TCCR1A = 1<<COM1A1 | 1<<COM1B1 | 1<<WGM10;		// 8位相位修正
    TCCR1B = 1<<CS10;			// 0分频
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
volatile unsigned char flag = 0;
volatile int sp = 0;
unsigned int value = 0;
unsigned char sBuf[3] = {0};
unsigned char times = 0;
ISR(TIMER0_OVF_vect)
{
//    LED_BLINK;
//    sBuf[1] = speed&0xFF;
//    sBuf[0] = (speed>>8);
//   SendStr(sBuf,2);
    sp += IncPIDCalc(speed,sptr);
    if(sp > 255)
        sp = 255;
    if(sp < 0)
        sp = 0;
    OCR1A = OCR1B = OCR2A = (255-sp);
    speed = 0;
    if(times++ > 8)
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

ISR(INT0_vect)
{
    speed++;
}

ISR(PCINT1_vect)
{
    VelInte++;
    commut(PINC&0x07);
}

int StartFun(unsigned char SetSpeed)
{
    unsigned char i = 0;
    unsigned char temp = 0xE0;
	stall = times = 0;
    DIS_PCINT;
    T0_OFF;
    I0_OFF;
    OCR1A = OCR1B = OCR2A = temp;
    ENB_PCINT;				// Enable PC interrupt
    while(1)
    {
        for(i=0; i<8; i++)
        {
            if(VelInte > 4)     // 电机启动成功
                goto over;
            commut(i);
            _delay_ms(10);
        }
        temp -= 8;
        if(temp < 24)
        {
            OCR1A = OCR1B = OCR2A = 0xFF;
            return 0;
        }
        else
            OCR1A = OCR1B = OCR2A = temp;
    }
over:
    /*
    if(speed > SetSpeed)
    {
        for(i=temp; i>SetSpeed; i--)
        {
            OCR1A = OCR1B = OCR2A = i;
            _delay_ms(3);
        }
    }
    else
    {
        for(i=temp; i<SetSpeed; i++)
        {
            OCR1A = OCR1B = OCR2A = i;
            _delay_ms(1);
        }
    }*/
    speed = 0;
    T0_ON;
    I0_ON;
    return 1;
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

int main(void)
{
    sptr = (PID *)malloc(sizeof(PID));
    IncPIDInit(sptr);
    PortInit();
    PCInit();
    LED_BLINK;
    _delay_ms(500);
    LED_BLINK;
    sei();
    UartInit();
    T1Init();
    T2Init();
    I0Init();
    while(!StartFun(0x80))
        _delay_ms(500);
    while(1)
    {
        if(stall)
        {
            while(!StartFun(0x80))
                _delay_ms(500);
        }
    }
    free(sptr);
}


