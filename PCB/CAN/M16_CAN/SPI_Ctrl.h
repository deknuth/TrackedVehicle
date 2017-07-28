/*============================================================
        SPI Control h file  V1.00
==============================================================
MCU: 						ATmega16L
Frequency: 			7.3728MHz
Test Objective: SPI control Function
Writer:					Fenghui Zhu
Data:						2008.12.5
=============================================================*/
#ifndef _SPI_Set_h_
#define _SPI_Set_h_
//********************* 
//* 用户类型定义区    * 
//*********************

//********************* 
//* 头 文 件 配 置 区 * 
//*********************
#include <avr/io.h>
#include <avr/iom16.h>
#include "Habit_Frank.h"
//******************** 
//*  系 统 宏 定 义  * 
//******************** 
#define  SPI_PORT   PORTB			//SPI端口
#define  SPI_DDR   DDRB 

#define  SS     4     			//  SPI存储时钟                                                                                       
#define  MOSI   5           //	SPI 主机发送，从机接受端口			 
#define  MISO		6						//  SPI 从机发送，主机接受端口
#define  SCK	  7						//	SPI移位时钟   
#define  CS     4     			//  SPI对外CS信号

#define  SPI_MOSI   5           //	SPI 主机发送，从机接受端口			 
#define  SPI_MISO		6						//  SPI 从机发送，主机接受端口
#define  SPI_SCK	  7						//	SPI移位时钟   
#define  SPI_CS     4     			//  SPI对外CS信号

//********************* 
//*   常 数 宏 定 义  * 
//*********************

//********************** 
//*   动 作 宏 定 义   * 
//**********************

//********************** 
//*   全局变量声明区   * 
//**********************

//********************** 
//*   函 数 声 明 区   * 
//**********************

/******************** 
*   全局函数声明区  * 
********************/ 
unsigned char SPI_MasterTransmit(unsigned char cData);			//SPI主机发送程序
void SPI_MasterInit(void);								//SPI初始化，并启动
void SPI_Disable(void);									//停止SPI
void SPI_Enable(void);										//使能SPI
void SPI_MasterTransmit_P(char cData); 	//SPI主机发送程序,有超时保护	


#endif
