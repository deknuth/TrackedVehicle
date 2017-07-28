/*============================================================
        CAN BUS Function C file V1.00
==============================================================
MCU: 						ATmega16L
CAN chip:       MCP2515+PCA82C250
Frequency:
Function: 			定义CAN BUS功能函数
Writer:					Fenghui Zhu
Data:						2009-3-31 
Reference:			"can.c" of Fabian Greif
=============================================================*/
//********************* 
//* 头 文 件 配 置 区 * 
//*********************
#include "CAN_BUS.h"

//********************** 
//*   函 数 声 明 区   * 
//**********************
void can_send_message( u32 id, u08 *data, u08 length, u08 flags);
//********************** 
//*   函 数 定 义 区   * 
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
void can_send_message( u32 id, u08 *data, u08 length, u08 flags)
{
	u08 temp[4]; 
	
	if (length > 8)
		length = 0;
	
//设置发送缓冲器0 发送优先级，TXP[1:0] = 11 , 为最高发送优先级
	mcp2515_bit_modify( TXB0CTRL, (1<<TXP1)|(1<<TXP0), (1<<TXP1)|(1<<TXP0) );
	
// 根据标志位，确定其ID格式，标准格式或扩展格式，同时发送其ID
	if (flags & CAN_EID) //扩展格式，11+18位ID
	{
		temp[0] = (u08) (id>>21);  //取出基本ID的高8位
		//取出基本ID的低三位，加上扩展ID的高2位，同时设置扩展标示符EXIDE使能
		temp[1] = (((u08) (id>>16)) & 0x03) | (((u08) (id>>13)) & 0xE0) | (1<<EXIDE); 
		
		temp[2] = (u08) (id>>8);  //取扩展ID
		temp[3] = (u08) id;
	}
	else 				//标准格式，11位ID
	{
		temp[0] = (u08) (id>>3);		//取出基本ID的高8位
		temp[1] = (u08) (id<<5);	//取出基本ID的低3位 , 同时设置扩展标示符EXIDE为0，即禁用
		temp[2] = 0;
		temp[3] = 0;
	}
	mcp2515_write_register_p( TXB0SIDH, temp, 4 );		//发送ID
	
	
	// 根据标志位，设定报文是否为远程发送请求，同时设定数据字节数
	if (flags & CAN_RTR)
		mcp2515_write_register( TXB0DLC, length | (1<<RTR) );		//设定发送的报文为远程发送请求，并定义报文数据字节数
	else
		mcp2515_write_register( TXB0DLC, length );			// 设定报文的数据字节数
	
	//写入发送数据
	mcp2515_write_register_p( TXB0D0, data, length );
	
// 请求发送报文，发送缓冲器为TXB0
	PORTB &= ~(1<<SPI_CS);
	SPI_MasterTransmit(SPI_RTS | 0x01);
	PORTB |= (1<<SPI_CS);
}



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
void can_read_message( u32 *ID, u08 *data, u08 *length, u08 *flags)
{
	u08 Intf_temp;
	u08 ID_temp[4] = {0,0,0,0};
	u08 RXB_CTRL_temp;
	Intf_temp = mcp2515_read_register( CANINTF);		//读取mcp2515中断标志位
	Intf_temp = (Intf_temp & ((1<<RX1IF) |(1<<RX0IF)));  //取出CANINTF中的RX1IF,RX0IF判断是否有缓冲器满
	
	if(Intf_temp == (1<<RX0IF))		//判断是否为RX0中断
		{
			*flags = 0xA0 ;						//表示接收到的是RX0缓冲器
			//RXB_CTRL_temp = mcp2515_read_register( RXB0CTRL );		//读取验收滤波器编号
			
			mcp2515_read_register_p( RXB0SIDH , ID_temp , 4);   //读取4个字节的ID信息
			
			*length = mcp2515_read_register( RXB0DLC );         //读取数据字节长度
			
			mcp2515_read_register_p( RXB0D0 , data , *length);  //读取数据
			
			mcp2515_bit_modify( CANINTF, (1<<RX0IF), 0 );				//清除缓冲器0满标志
		}
		else if(Intf_temp == (1<<RX1IF))		//判断是否为RX1中断
					{
						*flags = 0xA1 ;						//表示接收到的是RX1缓冲器
						
						mcp2515_read_register_p( RXB1SIDH , ID_temp , 4);   //读取4个字节的ID信息
			
						*length = mcp2515_read_register( RXB1DLC );         //读取数据字节长度
			
						mcp2515_read_register_p( RXB1D0 , data , *length);   //读取数据
			
						mcp2515_bit_modify( CANINTF, (1<<RX1IF), 0 );				//清除缓冲器1满标志
					}
		*ID = (((u32) ID_temp[0]<<24)|((u32) ID_temp[1]<<16)|((u32) ID_temp[2]<<8)|ID_temp[3]);	//将4个8位数据还原为32位
		
}


