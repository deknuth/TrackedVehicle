#include "main.h"
#include "uart.h"
#include "mcp2515.h"

#define LED_BLINK	PORTD^=0x02;
#define FEED_DOG	PORTC^=0x08;

void init(void);
void delay(unsigned int ms);

#define	UART	0

void PortInit(void)
{
	DDRD = 0B10000010;		// PD7->CAN_RST		low active
	PORTD= 0B10000000;
	PIND = 0x00;

	DDRC = 0B00001000;		// PD3->M8_DOG		
	PORTC= 0B10000000;
	PINC = 0x00;
}

void InterInit(void)
{
	EICRA |= 1<<ISC01;			// falling edge
	EIMSK |= 1<<INT0;
}

void blink(unsigned char times)
{
	unsigned char i;
	for(i=0; i<(times<<1); i++)
	{
		LED_BLINK;
		delay(250);
		FEED_DOG;

	}
}

ISR(INT0_vect)
{
	blink(2);
//	mcp2515_bit_modify( CANINTF, 0x00 , 1<<RX0IF);
}


void CAN2_Test(unsigned char num) 
{
  	unsigned char i;
  	struct can_msg msg_send;
  
  	msg_send.data[0]=0x08;
  	msg_send.data[1]=0x07;
  	msg_send.data[2]=0x06;
  	msg_send.data[3]=0x05;
  	msg_send.data[4]=0x04;
  	msg_send.data[5]=0x03;
  	msg_send.data[6]=0x02;
  	msg_send.data[7]=0x01;    

  
  	for(i=0;i<num;i++) 
  	{  
    	//CAN发送 
    	msg_send.id  = ID_TX;  //数据ID号
    	msg_send.len = 8;
    	msg_send.RTR = 0;
    	msg_send.prty = 0;
    	(void)MSCAN2SendMsg(msg_send);
		MCP2515_ByteRead(CNF1);
    	_delay_ms(100);
		PORTD^=0x02;
    	FEED_DOG;
  	}  
}


int main(void)
{
	PortInit();
	InterInit();
//	UartInit();
//	blink(3);
//	SendStr("abdd",4);
	FEED_DOG;
    Init2515(CAN_125Kbps);
	unsigned char temp, rtr = 0;
    unsigned char daten[8] = { 0x7B, 0x0F, 0x8E, 0x15, 0xC2, 0xAA, 0xAA, 0xFF };
	while(1)
	{
		CAN2_Test(4);
		FEED_DOG;
	}
	
#if 0
    while(1)
    {
	
	/*
        can_send_message( 0x048DABCD, daten, 8, CAN_EID );
        can_send_message( 0x00000123, daten, 5, CAN_RTR );
        daten[0] = daten[0] + 3;
        _delay_ms(200);
		blink(1);
		mcp2515_write_register( CANINTF , 0);
        mcp2515_write_register( CANSTAT , 0);
        mcp2515_write_register( EFLG , 0 );
		*/
		mcp2515_write_register(TXB0CTRL,0x03);   //设置为发送最高优先级  
		mcp2515_write_register(TXB0SIDH,0xff);   // SID10--SID3  
		mcp2515_write_register(TXB0SIDL,0x00);   //SID2--SID0  
		mcp2515_write_register(TXB0DLC, 8);     // 发送数据长度为8 字节  
		mcp2515_write_register(TXB0D0,0x01);   // 发送的数据88  
		mcp2515_write_register(TXB0D1,0x02);   // 发送的数据88  
		mcp2515_write_register(TXB0D2,0x03);   // 发送的数据88  
		mcp2515_write_register(TXB0D3,0x04);   // 发送的数据88  
		mcp2515_write_register(TXB0D4,0x05);   // 发送的数据88  
		mcp2515_write_register(TXB0D5,0x06);   // 发送的数据88  
		mcp2515_write_register(TXB0D6,0x07);   // 发送的数据88  
		mcp2515_write_register(TXB0D7,0x08);   // 发送的数据88
		
		temp = mcp2515_read_register(TXB0CTRL);
		
		
		while((mcp2515_read_register(TXB0CTRL) & 0x08) == 0x08);  //等待发送完毕  
		 _delay_ms(200);
		 uart_put_hex(temp);
	//	blink(1);
		FEED_DOG;
    }
	
	temp = mcp2515_read_register( CNF1 );
    uart_put_hex(temp);
	temp = mcp2515_read_register( CNF2 );
	uart_put_hex(temp);
    temp = mcp2515_read_register( CANINTE );
	uart_put_hex(temp);
	
    while(1)
    {
        temp = mcp2515_read_register( CANINTF );
        // 循环读can
        if ( temp & ((1<<RX0IF)|(1<<RX1IF)) )
        {
            unsigned char i;
            unsigned int iTemp = 0;

            SendStr("eine neue Nachricht wurde empfangen : \n");
            temp = mcp2515_read_register( RXB0SIDL );       
            if ( temp & (1<<IDE) )
            {
			
                SendStr("Extended Identifier : ");
				
                uart_put_hex(mcp2515_read_register( RXB0SIDL) & 0x03 );
                uart_put_hex(mcp2515_read_register( RXB0EID8) );
				uart_put_hex(mcp2515_read_register( RXB0EID0) );
                uart_putc('\n');
                uart_puts_P("Standard Identifier : ");
			
                iTemp = ( (unsigned int) mcp2515_read_register( RXB0SIDH ))<<3;
                iTemp |= (temp>>5) & 0x07;

                uart_put_hex((unsigned char) (iTemp>>8));
                uart_put_hex((unsigned char) iTemp & 0x00ff);
                uart_putc('\n');

                if ( mcp2515_read_register( RXB0DLC ) & (1<<RTR))
				{

					uart_puts_P("Remote Transmit Request\n");
				}
                
            }
            else
            {
                uart_puts_P("Standard Identifier : ");
                iTemp = ( (unsigned int) mcp2515_read_register( RXB0SIDH ))<<3;
                iTemp |= (temp>>5) & 0x07;

                uart_put_hex((unsigned char) (iTemp>>8));
                uart_put_hex((unsigned char) iTemp & 0x00ff);
				uart_putc('\n');

                if ( temp & (1<<SRR))
                {
                    uart_puts_P("Remote Transmit Request\n");
                    rtr = 1;
                }
                else
                {
                    ;
                }
            }
            /* Datenl鋘ge usw. einlesen */
            temp = mcp2515_read_register( RXB0DLC ) & 0x0F;
            if (rtr == 0)
            {
                /* Datenbytes ausgeben */
                for (i = 0;i < temp; i++ )
                {
                    unsigned char daten = mcp2515_read_register( RXB0D0 + i );
					
                    uart_puts_P("DByte ");
                    uart_putc(i + '0');
                    uart_puts_P(" : ");
                    uart_put_hex( daten );
                    uart_putc('\n');
					
                }
            }
            uart_putc('\n');
            /* Interrupt Flag l鰏chen */
            mcp2515_bit_modify( CANINTF, 0x01, 0x00 );
        }
    }
	#endif
	while(1);
    return 0;
}

void delay(unsigned int ms)
{
    unsigned int zaehler;
    while (ms)
    {
        zaehler = F_CPU / 5000;
        while (zaehler)
        {
            asm volatile ("nop");
            zaehler--;
        }
        ms--;
    }
}
