/*============================================================
        CAN BUS Function headfile V1.00
==============================================================
MCU: 						ATmega16L
CAN chip:       MCP2515+PCA82C250
Frequency:
Function: 			定义CAN BUS功能函数
Writer:					Fenghui Zhu
Data:						2009-3-31 
Reference:			"can.h" of Fabian Greif
=============================================================*/
#ifndef _CAN_BUS_H_
#define _CAN_BUS_H_ 
//********************* 
//* 头 文 件 配 置 区 * 
//*********************
#include "Habit_Frank.h"
#include "mcp2515.h"
#include "SPI_Ctrl.h"

//********************* 
//*   常 数 宏 定 义  * 
//*********************
#define	CAN_RTR		0x80		//远程数据请求 标志
#define	CAN_EID		0x40		//报文为扩展格式 标志

//********************** 
//*   函 数 声 明 区   * 
//**********************

//************************************************************* 
//   函数说明：CAN BUS发送消息程序 
//						 通过对标志位判断，来判断报文格式和报文用途
//						 完成报文ID，数据的发送功能            			            
//   输入：    u32 id   ， 节点标示位信息
//						 u08 *data ， 待发数据
//						 u08 length， 数据长度
//						 u08 flags)， 标志位，
//						标志位包含两个信息：1.数据报文or远程数据请求
//																2.标准格式 or 扩展格式
//   输出：    无                     	                       
//   调用函数：	mcp2515_write_register_p()
//							SPI_MasterInit()
//							mcp2515_write_register()                    						              
//*************************************************************
extern void can_send_message( u32 id, u08 *data, u08 length, u08 flags);


//************************************************************* 
//   函数说明：CAN BUS接收消息程序 
//						
//						            			            
//   输入：    u32 *id   ，  接收到的节点标示位信息指针
//						 u08 *data ，  接收到的数据
//						 u08 *length， 数据长度
//						 u08 *flags，  标志位信息，
//						标志位包含信息：1.接收到的是哪个接收缓冲器
//																
//   输出：    无                     	                       
//   调用函数：	mcp2515_read_register_p()
//							SPI_MasterInit()
//							mcp2515_read_register()                    						              
//*************************************************************
extern void can_read_message( u32 *ID, u08 *data, u08 *length, u08 *flags);

#endif 	// _CAN_H
