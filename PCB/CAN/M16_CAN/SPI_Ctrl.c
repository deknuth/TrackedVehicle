/*============================================================
        SPI Control C file  V1.00
==============================================================
MCU: 						ATmega16L
Frequency: 			7.3728MHz
Test Objective: SPI control Function
Writer:					Fenghui Zhu
Data:						2008.12.5
=============================================================*/

//********************* 
//* 用户类型定义区    * 
//*********************



//********************* 
//* 头 文 件 配 置 区 * 
//*********************

#include "SPI_Ctrl.h"

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


//********************** 
//*   函 数 声 明 区   * 
//**********************
/*void SPI_MasterTransmit(char cData);		//SPI主机发送程序
void SPI_MasterInit(void);							//SPI初始化，并启动
void SPI_Disable(void);									//停止SPI
void SPI_Enable(void);*/									//使能SPI

/******************** 
*   全局函数声明区  * 
********************/ 


//**********************************************************// 
//   函数说明：SPI主机发送程序											        // 
//   输入：    发送数据，Char格式	                          // 
//   输出：    SPI总线回传数据               		           // 
//   调用函数：无                                           //
//**********************************************************//
unsigned char SPI_MasterTransmit(unsigned char cData)
{
	SPDR = cData;									// 启动数据传输 
	while(!(SPSR & (1<<SPIF)));		// 等待传输结束
	return SPDR;									// 返回数据寄存器； 
}

//**********************************************************// 
//   函数说明：SPI初始化，为MCP2515配置                        // 
//   输入：    无									                          // 
//   输出：    无                                 	        // 
//   调用函数：无                         //
//**********************************************************//
void SPI_MasterInit(void)	//SPI初始化，并启动
{
	SPI_DDR |= ((1<<CS)|(1<<MOSI)|(1<<SCK));
					//设置MOSI, SS 和SCK 为输出，其他为输入
	SPI_PORT &= ~((1<<MOSI)|(1<<SCK));	//SPI引脚设置，SCK,MOSI,CS都为输出，且先清零。
	SPI_PORT |= (1<<CS);		//MCP2515的CS为低有效
	SPCR = (1<<SPE)|(1<<MSTR);		//enable SPI, Master mode, MSB go first, SCK=F/4
//	SPSR=(1<<SPI2X);
}

//**********************************************************// 
//   函数说明：停止SPI程序                                 // 
//   输入：    无									                          // 
//   输出：    无                                         // 
//   调用函数：无                                           //
//**********************************************************//
void SPI_Disable(void)		//停止SPI
{
	SPCR &= ~(1<<SPE);			//清零SPE位
}

//**********************************************************// 
//   函数说明：使能SPI程序，必须先对SPI初始化               // 
//   输入：    无									                          // 
//   输出：    键值                                         // 
//   调用函数：无                                           //
//**********************************************************//
void SPI_Enable(void)			//使能SPI
{
	SPCR |= (1<<SPE);				//置SPE位
}
