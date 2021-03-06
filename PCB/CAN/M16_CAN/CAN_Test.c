/*============================================================
        CAN BUS Test code V1.00
==============================================================
MCU: 						ATmega16L
CAN chip:       MCP2515+PCA82C250
Frequency: 			7.3728MHz
Function: 			测试节点自发自收功能，
                用LED来指示报文收发是否一致
Writer:					Fenghui Zhu
Data:						2008.12.4
=============================================================*/

//********************* 
//* 用户类型定义区    * 
//*********************

//********************* 
//* 头 文 件 配 置 区 * 
//*********************
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include "Habit_Frank.h"
#include "mcp2515.h"
#include "SPI_Ctrl.h"
#include "CAN_BUS.h" 
#include "LCD_CTRL.h"
#include "USART.h"
//******************** 
//*  系 统 宏 定 义  * 
//******************** 

//********************* 
//*   常 数 宏 定 义  *  
//*********************

//********************** 
//*   动 作 宏 定 义   * 
//**********************

//********************** 
//*   全局变量声明区   * 
//**********************
u32 ID_R ;
u08 data_R[8] ;
u08 length_R = 0;
u08 flags_R;
u08 i;

u16 j;
u16 receive_num;
u08 data_temp[80];
//********************** 
//*   函 数 声 明 区   * 
//**********************

//**********************************************************// 
//   函数说明：延时程序		                                 // 
//   输入：    延时单位数									                   // 
//   输出：    无                                          // 
//   调用函数：无                                            //
//***********************************************************//

void delay_x(unsigned int delay_t)								/*  2.5us/once  7.3Mhz*/
{														/*  10us/once  1.8Mhz */
    unsigned int ide;
    for (ide=0;ide<delay_t;ide++){ };
    return;
}


/*********************************************************** 
*   函数说明：键值获取程序                                  * 
*   输入：    无									                          * 
*   输出：    键值                                          * 
*   调用函数：无                                            * 
***********************************************************/ 
unsigned char GetKeyNum(void)
{
    unsigned char key_num0,key_num1;
    key_num1=((~PIND)&0X78);
    if(key_num1)
    {
        delay_x(0x2fff);						//30ms
        key_num0=((~PIND)&0X78);
    }
    while(((~PIND)&0X78)) { 	}	//等待键盘松开时看门狗定时器清零;
    if(key_num0!=key_num1) key_num0=0;
    return(key_num0);
}


//************************************************************* 
//*   函数说明：MCU端口初始化          			              		* 
//*   输入：    无                     						            * 
//*   输出：    无                     	                    	* 
//*   调用函数：                     						              * 
//*************************************************************
void port_init(void)					//IO端口初始化
{
    DDRA|=(0Xff);				//PA output LCD data
    PORTA&=~(0Xff);

    DDRB|=(0xff);
    PORTB&=~(0Xff);        //PB0--Ctrl1; PB1--Ctrl2; PB2--CS1; PB3--CS2; PB4--CS3(~SS);
    DDRB&=~(0X40);         //PB5--DIN; PB6--DOUT; PB7--CLK;

    DDRC|=0Xff;						//PC0--R/W_LCD; PC1--RS_LCD; PC2~PC5---JATG; PC6--BS; PC7--E_LCD;
    PORTC&=~(0Xff);
    DDRC &= ~(1 << PC6);
    PORTC |= (1 << PC6);

    DDRD&=~(0X78);
    PORTD|=(0X78);			//PD3,PD4,PD5,PD6 is touch_switch input,
    DDRD|=0X80;
    PORTD&=~(0X80);			//PD7 IS LED OUTPUT

    DDRD 	&= ~(1 << PD2);
    PORTD |= (1 << PD2);
    MCUCR = 0X00;						//低电平出发中断
    GICR = (1 << INT0);			//INT0中断使能



}

//************************************************************* 
//*   函数说明：外部中断INT0 服务程序
//*   输入：    无
//*   输出：    无
//*   调用函数：
//*************************************************************
SIGNAL (SIG_INTERRUPT0 )		//外部中断
{
    can_read_message( &ID_R, data_R, &length_R, &flags_R);

    for (i = 0 ; i < length_R ; i++)
    {
        data_temp[receive_num] = 	data_R[i] ;
        receive_num ++ ;
    }

    //	putstr_L(data_R , length_R);
    //	USART_Transmit( receive_num );
    //	DisplayListChar(0, 1, data_R);

}

//************************************************************* 
//*   函数说明：Main函数
//*   输入：    无
//*   输出：    无
//*   调用函数：
//*************************************************************
int main(void)
{
    cli();					//关全局中断
    u08	KeyNum = 0xff;
    u08 temp[8] = {0,0,0,0,0,0,0,0};
    u32 ID_S ;
    u08 data_S[8] = "/Hello! ";
    u08 temp_reg;
    u08 length_S ;
    u08 flags_S ;

    length_S 	= 8 ;
    ID_S 			= 0xa5a4a3a2;
    flags_S 	= 0 ;

    receive_num = 0;
    port_init();
    uart_init();
    SPI_MasterInit();		//SPI初始化，并启动
    mcp2515_init();		//MCP2515初始化
    //	LCDInit();		//LCD1602初始化

    sei();			//开全局中断

    //	DisplayListChar(0 , 0, data_S);

    //	can_send_message( ID_S, data_S, length_S, flags_S);

    mcp2515_read_register_p( TXB0SIDH , temp , 4 );
    temp[4] = mcp2515_read_register( TXB0CTRL );
    temp[5] = mcp2515_read_register( TXB0DLC );
    //	temp[6] = mcp2515_read_register( RXB1DLC );

    while(1)
    {
        KeyNum = GetKeyNum();        //获取按键值
        if ( KeyNum == 0x08 )
        {
            cli();					//关全局中断
            //				for(i = 0; i < 100 ; i++)
            {
                can_send_message( ID_S, data_S, length_S, flags_S);
                delay_x( 0xff );
            }
            temp_reg = mcp2515_read_register( CANINTF );
            USART_Transmit( temp_reg );
            temp_reg = mcp2515_read_register( CANSTAT );
            USART_Transmit( temp_reg );
            temp_reg = mcp2515_read_register( EFLG );
            USART_Transmit( temp_reg );
            temp_reg = 0xff;
            mcp2515_write_register( CANINTF , 0);
            mcp2515_write_register( CANSTAT , 0);
            mcp2515_write_register( EFLG , 0 );

            sei();			//开全局中断
        }
        else if ( KeyNum == 0x10 )
        {
            cli();					//关全局中断
            putstr_L(data_temp, (receive_num + 1));
            USART_Transmit( (u08) ( receive_num >> 3 ) );
            temp_reg = mcp2515_read_register( CANINTF );
            USART_Transmit( temp_reg );
            temp_reg = mcp2515_read_register( CANSTAT );
            USART_Transmit( temp_reg );
            temp_reg = mcp2515_read_register( EFLG );
            USART_Transmit( temp_reg );
            temp_reg = 0xff;

            mcp2515_write_register( CANINTF , 0);
            mcp2515_write_register( CANSTAT , 0);
            mcp2515_write_register( EFLG , 0 );

            receive_num = 0;
            for ( j = 0 ; j <80 ; j++)
            {
                data_temp[j] = 48 ;	//	"0"
            }
            sei();			//开全局中断
        }
        //		can_read_message( &ID_R, data_R, &length_R, &flags_R);

        //		DisplayListChar(0, 1, data_R);
    }


}
