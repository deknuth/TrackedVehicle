
#include "mcp2515.h"

unsigned char DLC,dummy;

#define	SPI_CS		2
#define	SPI_MISO	4
#define	SPI_MOSI	3
#define	SPI_SCK		5
#define	MCP2515_CS_L   PORTB&=~(1<<SPI_CS) //选中MCP2515	
#define	MCP2515_CS_H   PORTB|=(1<<SPI_CS)

/******* just for atmega48p ***********/
void spi_init(void)		
{
    DDRB |= (1<<SPI_CS)|(1<<SPI_SCK)|(1<<SPI_MOSI);
    PORTB |= (1<<SPI_CS);
    PORTB &= ~(1<<SPI_SCK)|(1<<SPI_MOSI)|(1<<SPI_MISO);
    // set SPI master interfaces, fosc = fclk/4
    SPCR = (1<<SPE)|(1<<MSTR)/*|(1<<SPIE)*/;
}
unsigned char SPI2_ReadWriteByte( unsigned char data )
{
    SPDR = data;
    while( !( SPSR & (1<<SPIF) ) );
    return SPDR;
}
/**********************/

//向MCP2515指定地址addr写一个字节数据value
void MCP2515_ByteWrite(unsigned char addr,unsigned char value)
{
 	MCP2515_CS_L;
  	SPI2_ReadWriteByte(CAN_WRITE);
  	SPI2_ReadWriteByte(addr);
  	SPI2_ReadWriteByte(value);
 	MCP2515_CS_H;
}

//从MCP2515指定地址addr读取一个字节数据value
unsigned char MCP2515_ByteRead(unsigned char addr)
{
  unsigned char tempdata;
 	MCP2515_CS_L;
  SPI2_ReadWriteByte(CAN_READ);
  SPI2_ReadWriteByte(addr);
  tempdata=SPI2_ReadWriteByte(0xFF);
  MCP2515_CS_H;
  return tempdata;
}

//MCP2515复位
void MCP2515_Reset(void)
{
  MCP2515_CS_L;
  SPI2_ReadWriteByte(CAN_RESET);
  MCP2515_CS_H;
}


//MCP2515初始化
void Init2515(unsigned char baudrate)
{
	spi_init();
  MCP2515_Reset();

  MCP2515_ByteWrite(CANCTRL,0x80);//CAN工作在配置模式
 
  MCP2515_ByteWrite(RXM0SIDH,0x00);
  MCP2515_ByteWrite(RXM0SIDL,0x00);

  MCP2515_ByteWrite(RXF0SIDH,0x00);
  MCP2515_ByteWrite(RXF0SIDL,0x00);
  //设置波特率
  //set CNF1,SJW=00,长度为1TQ,BRP=49,TQ=[2*(BRP+1)]/Fsoc=2*50/8M=12.5us
  MCP2515_ByteWrite(CNF1,baudrate);
  //set CNF2,SAM=0,在采样点对总线进行一次采样，PHSEG1=(2+1)TQ=3TQ,PRSEG=(0+1)TQ=1TQ
  MCP2515_ByteWrite(CNF2,0x80|PHSEG1_3TQ|PRSEG_1TQ);
  //set CNF3,PHSEG2=(2+1)TQ=3TQ,同时当CANCTRL.CLKEN=1时设定CLKOUT引脚为时间输出使能位
  MCP2515_ByteWrite(CNF3,PHSEG2_3TQ);

  //set TXB0，设置发送缓冲器0的标识符和发送的数据，以及发送的数据长度
  MCP2515_ByteWrite(TXB0SIDH,0xFF);//设置发送缓冲器0的标准标识符，待修改***
  MCP2515_ByteWrite(TXB0SIDL,0xE0);//用到标准标识符

  
  //设置接收缓冲器0的标识符和初始化数据
  MCP2515_ByteWrite(RXB0SIDH,0x00);//设置接收缓冲器0的标准标识符，待修改***
  MCP2515_ByteWrite(RXB0SIDL,0x60);//用到标准标识符
  MCP2515_ByteWrite(RXB0CTRL,0x60);//仅仅接收标准标识符的有效信息，FIILHIT0=0表示RXB0 ，采用FILHIT0
  MCP2515_ByteWrite(RXB0DLC,DLC_8);//设置接收数据的长度为8个字节


  //设置接收缓冲器0中断
  MCP2515_ByteWrite(CANINTF,0x00);//清空中断标志位
  MCP2515_ByteWrite(CANINTE,0x01);//接收缓冲器0满中断使能位

  //RX0BF中断使能
  MCP2515_ByteWrite(BFPCTRL,B0BFE|B0BFM);//接收缓冲器0中断使能位
  
  MCP2515_ByteWrite(CANCTRL,REQOP_NORMAL|CLKOUT_ENABLED);//设置正常模式

  dummy=MCP2515_ByteRead(CANSTAT);
  if(OPMODE_NORMAL!=(dummy&&0xE0))
  {
    MCP2515_ByteWrite(CANCTRL,REQOP_NORMAL|CLKOUT_ENABLED);//判断进入正常工作模式
  }
}

/*=================================================================================
Function：位修改子函数(应用于控制寄存器、INTE/INTF、CNF1/2/3)
入口参数：指定单元地址，屏蔽字节，数据字节
建立日期：2008.10.17
===================================================================================*/
void BitModiMcp2515(char addr,char mask,char udata)
{
     unsigned char i;
     MCP2515_CS_L;
     for(i = 10;i > 0;i--) ;       //短暂延时
     SPI2_ReadWriteByte(CAN_BIT_MODIFY);     //位修改送命令
     SPI2_ReadWriteByte(addr);
     SPI2_ReadWriteByte(mask);
     SPI2_ReadWriteByte(udata);
     MCP2515_CS_H;
     return;
}


/*************************************************************/
/*                       CAN2发送                            */
/*************************************************************/
unsigned char MSCAN2SendMsg(struct can_msg msg)
{
  unsigned char temp;
 
  // 检查数据长度
  if(msg.len > 8)
    return(0);

  // 检测 CAN协议报文模式 （一般/扩展） 标识符
  if(msg.id > 0x7FF)       
  {
    // 如果发送的是扩展帧,写入标识符 
    
    // RTR = 1为远程帧
    if(msg.RTR)   msg.id |= 0x01;    
    
    // SRR = 1 
    //msg.id |= 0x100000;     
    
    MCP2515_ByteWrite(TXB0SIDH,(unsigned char)(msg.id>>21));
    temp = (unsigned char)(msg.id>>13)&0xe0;
    temp|= (unsigned char)(msg.id>>16)&0x03;
    temp|=0x08; 
	  MCP2515_ByteWrite(TXB0SIDL,temp);
	  MCP2515_ByteWrite(TXB0EID8,(unsigned char)(msg.id>>8));
	  MCP2515_ByteWrite(TXB0EID0,(unsigned char)(msg.id));
	 
  } 
  else 
  {
    // 写入标识符
    MCP2515_ByteWrite(TXB0SIDH,(unsigned int)(msg.id>>3));
    temp =  (unsigned char)(msg.id<<5);
    
    // RTR = 1为远程帧
    if(msg.RTR)   temp |= 0x10;
    MCP2515_ByteWrite(TXB0SIDL,temp);  
  }
  
  //设置DLC和RTR
	MCP2515_ByteWrite(TXB0DLC,msg.len);           //设置发送数据帧长度
	
  //发送数据
	MCP2515_ByteWrite(TXB0D0,msg.data[0]);	      //向发送缓冲器写数据
  MCP2515_ByteWrite(TXB0D1,msg.data[1]);
	MCP2515_ByteWrite(TXB0D2,msg.data[2]);
	MCP2515_ByteWrite(TXB0D3,msg.data[3]);
	MCP2515_ByteWrite(TXB0D4,msg.data[4]);
	MCP2515_ByteWrite(TXB0D5,msg.data[5]);
	MCP2515_ByteWrite(TXB0D6,msg.data[6]);
	MCP2515_ByteWrite(TXB0D7,msg.data[7]);
	
	//启动发送
	MCP2515_CS_L;
	SPI2_ReadWriteByte(CAN_RTS_TXB0);
	
	while(!(SPI2_ReadWriteByte(TXB0CTRL) & TXREQ));  //等待发送完毕  
  
 	MCP2515_CS_H;
  return 1;
}

//CAN接收任意长度的数据

/*
void CAN2_Receive(struct can_msg *msg)
{
        
  unsigned char i;  
  ulong id;
  
  //没有收到新的数据帧则返回
  if(!MCP2515_ByteRead(CANINTF)&0x01)   
	return;
  
  //读取标准/扩展帧标识符
  if(MCP2515_ByteRead(RXB0SIDL)&0x08) 
  {  
    //读取扩展帧ID 
    id = MCP2515_ByteRead(RXB0SIDH);
	  id <<= 8;
	  id |= MCP2515_ByteRead(RXB0SIDL);
	  id >>= 5;
	  id <<=2;
	  id = id| MCP2515_ByteRead(RXB0SIDL)&03;
	  id<<=8;                  
	  id |= MCP2515_ByteRead(RXB0EID8);
	  id <<= 8;
	  id |= MCP2515_ByteRead(RXB0EID0);
 
  } 
  else 
  {
    // 读标识符
    id = (unsigned int)((MCP2515_ByteRead(RXB0SIDH))<<3) | 
         (unsigned char)((MCP2515_ByteRead(RXB0SIDL))>>5);
  }
  
  if(MCP2515_ByteRead(RXB0CTRL)&RXRTR_REMOTE)
     msg->RTR = TRUE;
  else
     msg->RTR = FALSE; 
    
   msg->id = id;
  
  //读取数据长度
  DLC=MCP2515_ByteRead(RXB0DLC); 
  
  msg->len = DLC;
  GPRS_LED=~GPRS_LED;
  for(i=0;i<DLC;i++)
  {
    //把接收缓冲区里的数据，放到内部RAM缓冲区
    msg->data[i]=MCP2515_ByteRead(RXB0D0+i);
  }

  //清除接收标志
  MCP2515_ByteWrite(CANINTF,0); 
    
}   
*/
