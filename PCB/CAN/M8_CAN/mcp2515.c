
#include "mcp2515.h"

unsigned char DLC,dummy;

#define	SPI_CS		2
#define	SPI_MISO	4
#define	SPI_MOSI	3
#define	SPI_SCK		5
#define	MCP2515_CS_L   PORTB&=~(1<<SPI_CS) //ѡ��MCP2515	
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

//��MCP2515ָ����ַaddrдһ���ֽ�����value
void MCP2515_ByteWrite(unsigned char addr,unsigned char value)
{
 	MCP2515_CS_L;
  	SPI2_ReadWriteByte(CAN_WRITE);
  	SPI2_ReadWriteByte(addr);
  	SPI2_ReadWriteByte(value);
 	MCP2515_CS_H;
}

//��MCP2515ָ����ַaddr��ȡһ���ֽ�����value
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

//MCP2515��λ
void MCP2515_Reset(void)
{
  MCP2515_CS_L;
  SPI2_ReadWriteByte(CAN_RESET);
  MCP2515_CS_H;
}


//MCP2515��ʼ��
void Init2515(unsigned char baudrate)
{
	spi_init();
  MCP2515_Reset();

  MCP2515_ByteWrite(CANCTRL,0x80);//CAN����������ģʽ
 
  MCP2515_ByteWrite(RXM0SIDH,0x00);
  MCP2515_ByteWrite(RXM0SIDL,0x00);

  MCP2515_ByteWrite(RXF0SIDH,0x00);
  MCP2515_ByteWrite(RXF0SIDL,0x00);
  //���ò�����
  //set CNF1,SJW=00,����Ϊ1TQ,BRP=49,TQ=[2*(BRP+1)]/Fsoc=2*50/8M=12.5us
  MCP2515_ByteWrite(CNF1,baudrate);
  //set CNF2,SAM=0,�ڲ���������߽���һ�β�����PHSEG1=(2+1)TQ=3TQ,PRSEG=(0+1)TQ=1TQ
  MCP2515_ByteWrite(CNF2,0x80|PHSEG1_3TQ|PRSEG_1TQ);
  //set CNF3,PHSEG2=(2+1)TQ=3TQ,ͬʱ��CANCTRL.CLKEN=1ʱ�趨CLKOUT����Ϊʱ�����ʹ��λ
  MCP2515_ByteWrite(CNF3,PHSEG2_3TQ);

  //set TXB0�����÷��ͻ�����0�ı�ʶ���ͷ��͵����ݣ��Լ����͵����ݳ���
  MCP2515_ByteWrite(TXB0SIDH,0xFF);//���÷��ͻ�����0�ı�׼��ʶ�������޸�***
  MCP2515_ByteWrite(TXB0SIDL,0xE0);//�õ���׼��ʶ��

  
  //���ý��ջ�����0�ı�ʶ���ͳ�ʼ������
  MCP2515_ByteWrite(RXB0SIDH,0x00);//���ý��ջ�����0�ı�׼��ʶ�������޸�***
  MCP2515_ByteWrite(RXB0SIDL,0x60);//�õ���׼��ʶ��
  MCP2515_ByteWrite(RXB0CTRL,0x60);//�������ձ�׼��ʶ������Ч��Ϣ��FIILHIT0=0��ʾRXB0 ������FILHIT0
  MCP2515_ByteWrite(RXB0DLC,DLC_8);//���ý������ݵĳ���Ϊ8���ֽ�


  //���ý��ջ�����0�ж�
  MCP2515_ByteWrite(CANINTF,0x00);//����жϱ�־λ
  MCP2515_ByteWrite(CANINTE,0x01);//���ջ�����0���ж�ʹ��λ

  //RX0BF�ж�ʹ��
  MCP2515_ByteWrite(BFPCTRL,B0BFE|B0BFM);//���ջ�����0�ж�ʹ��λ
  
  MCP2515_ByteWrite(CANCTRL,REQOP_NORMAL|CLKOUT_ENABLED);//��������ģʽ

  dummy=MCP2515_ByteRead(CANSTAT);
  if(OPMODE_NORMAL!=(dummy&&0xE0))
  {
    MCP2515_ByteWrite(CANCTRL,REQOP_NORMAL|CLKOUT_ENABLED);//�жϽ�����������ģʽ
  }
}

/*=================================================================================
Function��λ�޸��Ӻ���(Ӧ���ڿ��ƼĴ�����INTE/INTF��CNF1/2/3)
��ڲ�����ָ����Ԫ��ַ�������ֽڣ������ֽ�
�������ڣ�2008.10.17
===================================================================================*/
void BitModiMcp2515(char addr,char mask,char udata)
{
     unsigned char i;
     MCP2515_CS_L;
     for(i = 10;i > 0;i--) ;       //������ʱ
     SPI2_ReadWriteByte(CAN_BIT_MODIFY);     //λ�޸�������
     SPI2_ReadWriteByte(addr);
     SPI2_ReadWriteByte(mask);
     SPI2_ReadWriteByte(udata);
     MCP2515_CS_H;
     return;
}


/*************************************************************/
/*                       CAN2����                            */
/*************************************************************/
unsigned char MSCAN2SendMsg(struct can_msg msg)
{
  unsigned char temp;
 
  // ������ݳ���
  if(msg.len > 8)
    return(0);

  // ��� CANЭ�鱨��ģʽ ��һ��/��չ�� ��ʶ��
  if(msg.id > 0x7FF)       
  {
    // ������͵�����չ֡,д���ʶ�� 
    
    // RTR = 1ΪԶ��֡
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
    // д���ʶ��
    MCP2515_ByteWrite(TXB0SIDH,(unsigned int)(msg.id>>3));
    temp =  (unsigned char)(msg.id<<5);
    
    // RTR = 1ΪԶ��֡
    if(msg.RTR)   temp |= 0x10;
    MCP2515_ByteWrite(TXB0SIDL,temp);  
  }
  
  //����DLC��RTR
	MCP2515_ByteWrite(TXB0DLC,msg.len);           //���÷�������֡����
	
  //��������
	MCP2515_ByteWrite(TXB0D0,msg.data[0]);	      //���ͻ�����д����
  MCP2515_ByteWrite(TXB0D1,msg.data[1]);
	MCP2515_ByteWrite(TXB0D2,msg.data[2]);
	MCP2515_ByteWrite(TXB0D3,msg.data[3]);
	MCP2515_ByteWrite(TXB0D4,msg.data[4]);
	MCP2515_ByteWrite(TXB0D5,msg.data[5]);
	MCP2515_ByteWrite(TXB0D6,msg.data[6]);
	MCP2515_ByteWrite(TXB0D7,msg.data[7]);
	
	//��������
	MCP2515_CS_L;
	SPI2_ReadWriteByte(CAN_RTS_TXB0);
	
	while(!(SPI2_ReadWriteByte(TXB0CTRL) & TXREQ));  //�ȴ��������  
  
 	MCP2515_CS_H;
  return 1;
}

//CAN�������ⳤ�ȵ�����

/*
void CAN2_Receive(struct can_msg *msg)
{
        
  unsigned char i;  
  ulong id;
  
  //û���յ��µ�����֡�򷵻�
  if(!MCP2515_ByteRead(CANINTF)&0x01)   
	return;
  
  //��ȡ��׼/��չ֡��ʶ��
  if(MCP2515_ByteRead(RXB0SIDL)&0x08) 
  {  
    //��ȡ��չ֡ID 
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
    // ����ʶ��
    id = (unsigned int)((MCP2515_ByteRead(RXB0SIDH))<<3) | 
         (unsigned char)((MCP2515_ByteRead(RXB0SIDL))>>5);
  }
  
  if(MCP2515_ByteRead(RXB0CTRL)&RXRTR_REMOTE)
     msg->RTR = TRUE;
  else
     msg->RTR = FALSE; 
    
   msg->id = id;
  
  //��ȡ���ݳ���
  DLC=MCP2515_ByteRead(RXB0DLC); 
  
  msg->len = DLC;
  GPRS_LED=~GPRS_LED;
  for(i=0;i<DLC;i++)
  {
    //�ѽ��ջ�����������ݣ��ŵ��ڲ�RAM������
    msg->data[i]=MCP2515_ByteRead(RXB0D0+i);
  }

  //������ձ�־
  MCP2515_ByteWrite(CANINTF,0); 
    
}   
*/
