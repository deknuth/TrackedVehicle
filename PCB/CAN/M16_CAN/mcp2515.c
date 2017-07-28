/*============================================================
        C file about MCP2515   V1.00
==============================================================
Chip: 					MCP2515
Function:				The controller of CAN-BUS
Writer:					Fenghui Zhu
Data:						2009.3.31
Reference:			"mcp2515.c" of Fabian Greif
=============================================================*/

//********************* 
//* 头 文 件 配 置 区 * 
//*********************
#include "mcp2515.h"

//********************** 
//*   函 数 声 明 区   * 
//**********************


void mcp2515_init(void);

void mcp2515_write_register(u08 data, u08 adress);

u08 mcp2515_read_register(u08 adress);

void mcp2515_bit_modify(u08 data, u08 mask, u08 adress);

void mcp2515_write_register_p( u08 adress, u08 *data, u08 length );

//********************** 
//*   函 数 定 义 区   * 
//**********************

//**********************************************************// 
//   函数说明：MCP2515初始化程序                            // 
//   输入：    无									                          // 
//   输出：    无                                 	        // 
//   调用函数：                         									//
//**********************************************************//
void mcp2515_init(void)
{
	//初始化MCU的SPI总线
	//SPI_MasterInit();
	
	// MCP2515 启动前进行软件复位
	SPI_PORT &= ~(1<<SPI_CS);		//MCP2515的CS有效
	SPI_MasterTransmit( SPI_RESET );
	SPI_PORT |= (1<<SPI_CS);		//MCP2515的CS无效
	
	//使用位修改指令将MCP2515设置为配置模式
	//也就是将CANCTRL寄存器的REQOP[2:0]设置为100
	mcp2515_bit_modify( CANCTRL, 0xE0, (1<<REQOP2) );
	
	/*
	//计算并设置MCP2515的位时间
	
	// 时钟频率：Fosc  = 16MHz
	// 分频控制器 CNF1.BRP[5:0] = 7
	// 最小时间份额 TQ = 2 * ( BRP + 1 ) / Fosc   = 2*(7+1)/16M = 1uS
	// 同步段 Sync Seg   = 1TQ
	// 传播段 Prop Seg   = ( PRSEG + 1 ) * TQ  = 1 TQ
	// 相位缓冲段 Phase Seg1 = ( PHSEG1 + 1 ) * TQ = 3 TQ
	// 相位缓冲段 Phase Seg2 = ( PHSEG2 + 1 ) * TQ = 3 TQ
	// 同步跳转长度设置为 CNF1.SJW[1:0] = 00, 即 1TQ
	// 总线波特率 NBR = Fbit =  1/(sync seg + Prop seg + PS1 + PS2 )
	//                       = 1/(8TQ) = 1/8uS = 125kHz
	
	//设置分频控制器CNF1.BRP[5:0] = 7，同步跳转长度设置为 CNF1.SJW[1:0] = 00
	mcp2515_write_register( CNF1, (1<<BRP0)|(1<<BRP1)|(1<<BRP2) );
	// 设置传播段 Prop Seg 为00，即1TQ，相位缓冲段 Phase Seg1的长度3TQ
	mcp2515_write_register( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	// 设置 相位缓冲段 Phase Seg2为 3TQ ， 禁用唤醒滤波器
	mcp2515_write_register( CNF3, (1<<PHSEG21) );
	
	*/
	
	//设置为500kbps ，TQ = 1/8us
	//设置分频控制器CNF1.BRP[5:0] = 0，同步跳转长度设置为 CNF1.SJW[1:0] = 01
//	mcp2515_write_register( CNF1, (1<<BRP0)|(1<<SJW0) );    // 500kbps
	mcp2515_write_register( CNF1, (1<<SJW0) );               //1Mbps
		// 设置传播段 Prop Seg 为00，即1TQ，相位缓冲段 Phase Seg1的长度3TQ
	mcp2515_write_register( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	// 设置 相位缓冲段 Phase Seg2为 3TQ ， 禁用唤醒滤波器
	mcp2515_write_register( CNF3, (1<<PHSEG21) );
	
	
	// 设置MCP2515中断使能寄存器，禁用所有中断
//	mcp2515_write_register( CANINTE, /*(1<<RX1IE)|(1<<RX0IE)*/ 0 );
	
	// 设置MCP2515中断使能寄存器,使能接收缓冲器中断
	mcp2515_write_register( CANINTE, (1<<RX1IE)|(1<<RX0IE) );
	

	//设置数据接收相关寄存器	
	
	// 设置RXM[1:0]=11,关闭接收缓冲器0屏蔽/滤波功能，接收所有报文；禁止滚存功能
	mcp2515_write_register( RXB0CTRL, (1<<RXM1)|(1<<RXM0) );
	
	// 设置RXM[1:0]=11,关闭接收缓冲器1屏蔽/滤波功能，接收所有报文；
	mcp2515_write_register( RXB1CTRL, (1<<RXM1)|(1<<RXM0) );

	u08 temp[4] = { 0, 0, 0, 0 };
	
	//设置6个验收滤波寄存器为0，
	mcp2515_write_register_p( RXF0SIDH, temp, 4 );
	mcp2515_write_register_p( RXF1SIDH, temp, 4 );
	mcp2515_write_register_p( RXF2SIDH, temp, 4 );
	mcp2515_write_register_p( RXF3SIDH, temp, 4 );
	mcp2515_write_register_p( RXF4SIDH, temp, 4 );
	mcp2515_write_register_p( RXF5SIDH, temp, 4 );
	
	//设置2个验收滤波寄存器为0，
	mcp2515_write_register_p( RXM0SIDH, temp, 4 );
	mcp2515_write_register_p( RXM1SIDH, temp, 4 );
	
	//配置引脚
	//设置接收相关引脚控制寄存器，配置它们禁用第二功能
	mcp2515_write_register( BFPCTRL, 0 );
	
	//调试使用，设置BFPCTRL使RX0BF,RX1BF设置为数字输出。
	//mcp2515_bit_modify( BFPCTRL, (1<<B1BFE)|(1<<B0BFE)|(1<<B1BFM)|(1<<B0BFM), (1<<B1BFE)|(1<<B0BFE) );
	
	
	//设置发送相关引脚控制寄存器，配置它们禁用第二功能
	mcp2515_write_register( TXRTSCTRL, 0 );
	
	//MCP2515进入环回模式，进行功能测试
	//mcp2515_bit_modify( CANCTRL, 0XE0, (1<<REQOP1) );
	
	//MCP2515进入正常模式
	mcp2515_bit_modify( CANCTRL, 0xE0, 0);
}


//**********************************************************// 
//   函数说明：MCP2515写控制寄存器程序                      // 
//   输入：    寄存器地址，写入数据                         // 
//   输出：    无                           	              // 
//   调用函数：SPI发送程序SPI_MasterTransmit                //
//**********************************************************//

void mcp2515_write_register( u08 adress, u08 data )
{
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_WRITE); // 发送SPI写寄存器控制字 
	
	SPI_MasterTransmit(adress);		//发送寄存器地址
	
	SPI_MasterTransmit(data);			//发送寄存器数据
	
	//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
}

//**********************************************************// 
//   函数说明：MCP2515读控制寄存器程序                      // 
//   输入：    寄存器地址，                         				// 
//   输出：    寄存器数据                           	      // 
//   调用函数：SPI发送程序SPI_MasterTransmit                //
//**********************************************************//

u08 mcp2515_read_register(u08 adress)
{
	u08 data;
	
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_READ); // 发送SPI写寄存器控制字
	
	SPI_MasterTransmit(adress);	//发送寄存器地址
	
	data = SPI_MasterTransmit(0xff);	//回读寄存器数据
	
	//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
	
	return data;
}

//**********************************************************// 
//   函数说明：读MCP2515接收缓冲器程序                      // 
//   输入：    缓冲器地址，                         				// 
//   输出：    缓冲器数据                           	      // 
//   调用函数：SPI发送程序SPI_MasterTransmit                //
//**********************************************************//

u08 mcp2515_read_rx_buffer(u08 adress)
{
	u08 data;
	
	// 判断adress是否有效，除了1，2位，其余都应为0
	if (adress & 0xF9)
		return 0;
	
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_READ_RX | adress);	//发送读取控制字
	
	data = SPI_MasterTransmit(0xff);	//读回数据
	
//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
	
	return data;
}

//**********************************************************// 
//   函数说明：MCP2515控制寄存器位修改程序                // 
//   输入：    寄存器地址，修改位，修改数据              		// 
//   输出：    无                           	      				// 
//   调用函数：SPI发送程序SPI_MasterTransmit                //
//**********************************************************//

void mcp2515_bit_modify(u08 adress, u08 mask, u08 data)
{
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_BIT_MODIFY);	//SPI位修改指令
	
	SPI_MasterTransmit(adress);				//发送寄存器地址
	
	SPI_MasterTransmit(mask);					//发送屏蔽字节，
																		//屏蔽字节中“1”表示允许对相应位修改，“0”表示禁止修改
	SPI_MasterTransmit(data);					//发送数据字节
	
//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
}

//**********************************************************// 
//   函数说明：对MCP2515连续寄存器进行连续写操作            // 
//   输入：    连续寄存器起始地址，数据指针，数据长度    		// 
//   输出：    无                           	      				// 
//   调用函数：SPI发送程序SPI_MasterTransmit                //
//**********************************************************//
void mcp2515_write_register_p( u08 adress, u08 *data, u08 length )
{
	u08 i;
	
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_WRITE);		//发送SPI写指令
	
	SPI_MasterTransmit(adress);				//发送起始寄存器地址
	
	for (i=0; i<length ;i++ )
		SPI_MasterTransmit(*data++);		//发送数据
	
	//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
}

//**********************************************************// 
//   函数说明：对MCP2515连续寄存器进行连续读操作            // 
//   输入：    连续寄存器起始地址，数据指针，数据长度    		// 
//   输出：    无                           	      				// 
//   调用函数：SPI发送程序SPI_MasterTransmit                //
//**********************************************************//
void mcp2515_read_register_p( u08 adress, u08 *data, u08 length )
{
	u08 i;
	
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_READ);		//发送SPI读指令
	
	SPI_MasterTransmit(adress);			//发送起始寄存器地址
	
	for (i=0; i<length ;i++ )
		*data++ = SPI_MasterTransmit(0xff);		//数据保存
	
	//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
}
