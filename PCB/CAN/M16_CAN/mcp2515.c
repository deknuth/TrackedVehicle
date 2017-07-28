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
//* ͷ �� �� �� �� �� * 
//*********************
#include "mcp2515.h"

//********************** 
//*   �� �� �� �� ��   * 
//**********************


void mcp2515_init(void);

void mcp2515_write_register(u08 data, u08 adress);

u08 mcp2515_read_register(u08 adress);

void mcp2515_bit_modify(u08 data, u08 mask, u08 adress);

void mcp2515_write_register_p( u08 adress, u08 *data, u08 length );

//********************** 
//*   �� �� �� �� ��   * 
//**********************

//**********************************************************// 
//   ����˵����MCP2515��ʼ������                            // 
//   ���룺    ��									                          // 
//   �����    ��                                 	        // 
//   ���ú�����                         									//
//**********************************************************//
void mcp2515_init(void)
{
	//��ʼ��MCU��SPI����
	//SPI_MasterInit();
	
	// MCP2515 ����ǰ���������λ
	SPI_PORT &= ~(1<<SPI_CS);		//MCP2515��CS��Ч
	SPI_MasterTransmit( SPI_RESET );
	SPI_PORT |= (1<<SPI_CS);		//MCP2515��CS��Ч
	
	//ʹ��λ�޸�ָ�MCP2515����Ϊ����ģʽ
	//Ҳ���ǽ�CANCTRL�Ĵ�����REQOP[2:0]����Ϊ100
	mcp2515_bit_modify( CANCTRL, 0xE0, (1<<REQOP2) );
	
	/*
	//���㲢����MCP2515��λʱ��
	
	// ʱ��Ƶ�ʣ�Fosc  = 16MHz
	// ��Ƶ������ CNF1.BRP[5:0] = 7
	// ��Сʱ��ݶ� TQ = 2 * ( BRP + 1 ) / Fosc   = 2*(7+1)/16M = 1uS
	// ͬ���� Sync Seg   = 1TQ
	// ������ Prop Seg   = ( PRSEG + 1 ) * TQ  = 1 TQ
	// ��λ����� Phase Seg1 = ( PHSEG1 + 1 ) * TQ = 3 TQ
	// ��λ����� Phase Seg2 = ( PHSEG2 + 1 ) * TQ = 3 TQ
	// ͬ����ת��������Ϊ CNF1.SJW[1:0] = 00, �� 1TQ
	// ���߲����� NBR = Fbit =  1/(sync seg + Prop seg + PS1 + PS2 )
	//                       = 1/(8TQ) = 1/8uS = 125kHz
	
	//���÷�Ƶ������CNF1.BRP[5:0] = 7��ͬ����ת��������Ϊ CNF1.SJW[1:0] = 00
	mcp2515_write_register( CNF1, (1<<BRP0)|(1<<BRP1)|(1<<BRP2) );
	// ���ô����� Prop Seg Ϊ00����1TQ����λ����� Phase Seg1�ĳ���3TQ
	mcp2515_write_register( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	// ���� ��λ����� Phase Seg2Ϊ 3TQ �� ���û����˲���
	mcp2515_write_register( CNF3, (1<<PHSEG21) );
	
	*/
	
	//����Ϊ500kbps ��TQ = 1/8us
	//���÷�Ƶ������CNF1.BRP[5:0] = 0��ͬ����ת��������Ϊ CNF1.SJW[1:0] = 01
//	mcp2515_write_register( CNF1, (1<<BRP0)|(1<<SJW0) );    // 500kbps
	mcp2515_write_register( CNF1, (1<<SJW0) );               //1Mbps
		// ���ô����� Prop Seg Ϊ00����1TQ����λ����� Phase Seg1�ĳ���3TQ
	mcp2515_write_register( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	// ���� ��λ����� Phase Seg2Ϊ 3TQ �� ���û����˲���
	mcp2515_write_register( CNF3, (1<<PHSEG21) );
	
	
	// ����MCP2515�ж�ʹ�ܼĴ��������������ж�
//	mcp2515_write_register( CANINTE, /*(1<<RX1IE)|(1<<RX0IE)*/ 0 );
	
	// ����MCP2515�ж�ʹ�ܼĴ���,ʹ�ܽ��ջ������ж�
	mcp2515_write_register( CANINTE, (1<<RX1IE)|(1<<RX0IE) );
	

	//�������ݽ�����ؼĴ���	
	
	// ����RXM[1:0]=11,�رս��ջ�����0����/�˲����ܣ��������б��ģ���ֹ���湦��
	mcp2515_write_register( RXB0CTRL, (1<<RXM1)|(1<<RXM0) );
	
	// ����RXM[1:0]=11,�رս��ջ�����1����/�˲����ܣ��������б��ģ�
	mcp2515_write_register( RXB1CTRL, (1<<RXM1)|(1<<RXM0) );

	u08 temp[4] = { 0, 0, 0, 0 };
	
	//����6�������˲��Ĵ���Ϊ0��
	mcp2515_write_register_p( RXF0SIDH, temp, 4 );
	mcp2515_write_register_p( RXF1SIDH, temp, 4 );
	mcp2515_write_register_p( RXF2SIDH, temp, 4 );
	mcp2515_write_register_p( RXF3SIDH, temp, 4 );
	mcp2515_write_register_p( RXF4SIDH, temp, 4 );
	mcp2515_write_register_p( RXF5SIDH, temp, 4 );
	
	//����2�������˲��Ĵ���Ϊ0��
	mcp2515_write_register_p( RXM0SIDH, temp, 4 );
	mcp2515_write_register_p( RXM1SIDH, temp, 4 );
	
	//��������
	//���ý���������ſ��ƼĴ������������ǽ��õڶ�����
	mcp2515_write_register( BFPCTRL, 0 );
	
	//����ʹ�ã�����BFPCTRLʹRX0BF,RX1BF����Ϊ���������
	//mcp2515_bit_modify( BFPCTRL, (1<<B1BFE)|(1<<B0BFE)|(1<<B1BFM)|(1<<B0BFM), (1<<B1BFE)|(1<<B0BFE) );
	
	
	//���÷���������ſ��ƼĴ������������ǽ��õڶ�����
	mcp2515_write_register( TXRTSCTRL, 0 );
	
	//MCP2515���뻷��ģʽ�����й��ܲ���
	//mcp2515_bit_modify( CANCTRL, 0XE0, (1<<REQOP1) );
	
	//MCP2515��������ģʽ
	mcp2515_bit_modify( CANCTRL, 0xE0, 0);
}


//**********************************************************// 
//   ����˵����MCP2515д���ƼĴ�������                      // 
//   ���룺    �Ĵ�����ַ��д������                         // 
//   �����    ��                           	              // 
//   ���ú�����SPI���ͳ���SPI_MasterTransmit                //
//**********************************************************//

void mcp2515_write_register( u08 adress, u08 data )
{
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_WRITE); // ����SPIд�Ĵ��������� 
	
	SPI_MasterTransmit(adress);		//���ͼĴ�����ַ
	
	SPI_MasterTransmit(data);			//���ͼĴ�������
	
	//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
}

//**********************************************************// 
//   ����˵����MCP2515�����ƼĴ�������                      // 
//   ���룺    �Ĵ�����ַ��                         				// 
//   �����    �Ĵ�������                           	      // 
//   ���ú�����SPI���ͳ���SPI_MasterTransmit                //
//**********************************************************//

u08 mcp2515_read_register(u08 adress)
{
	u08 data;
	
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_READ); // ����SPIд�Ĵ���������
	
	SPI_MasterTransmit(adress);	//���ͼĴ�����ַ
	
	data = SPI_MasterTransmit(0xff);	//�ض��Ĵ�������
	
	//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
	
	return data;
}

//**********************************************************// 
//   ����˵������MCP2515���ջ���������                      // 
//   ���룺    ��������ַ��                         				// 
//   �����    ����������                           	      // 
//   ���ú�����SPI���ͳ���SPI_MasterTransmit                //
//**********************************************************//

u08 mcp2515_read_rx_buffer(u08 adress)
{
	u08 data;
	
	// �ж�adress�Ƿ���Ч������1��2λ�����඼ӦΪ0
	if (adress & 0xF9)
		return 0;
	
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_READ_RX | adress);	//���Ͷ�ȡ������
	
	data = SPI_MasterTransmit(0xff);	//��������
	
//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
	
	return data;
}

//**********************************************************// 
//   ����˵����MCP2515���ƼĴ���λ�޸ĳ���                // 
//   ���룺    �Ĵ�����ַ���޸�λ���޸�����              		// 
//   �����    ��                           	      				// 
//   ���ú�����SPI���ͳ���SPI_MasterTransmit                //
//**********************************************************//

void mcp2515_bit_modify(u08 adress, u08 mask, u08 data)
{
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_BIT_MODIFY);	//SPIλ�޸�ָ��
	
	SPI_MasterTransmit(adress);				//���ͼĴ�����ַ
	
	SPI_MasterTransmit(mask);					//���������ֽڣ�
																		//�����ֽ��С�1����ʾ�������Ӧλ�޸ģ���0����ʾ��ֹ�޸�
	SPI_MasterTransmit(data);					//���������ֽ�
	
//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
}

//**********************************************************// 
//   ����˵������MCP2515�����Ĵ�����������д����            // 
//   ���룺    �����Ĵ�����ʼ��ַ������ָ�룬���ݳ���    		// 
//   �����    ��                           	      				// 
//   ���ú�����SPI���ͳ���SPI_MasterTransmit                //
//**********************************************************//
void mcp2515_write_register_p( u08 adress, u08 *data, u08 length )
{
	u08 i;
	
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_WRITE);		//����SPIдָ��
	
	SPI_MasterTransmit(adress);				//������ʼ�Ĵ�����ַ
	
	for (i=0; i<length ;i++ )
		SPI_MasterTransmit(*data++);		//��������
	
	//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
}

//**********************************************************// 
//   ����˵������MCP2515�����Ĵ�����������������            // 
//   ���룺    �����Ĵ�����ʼ��ַ������ָ�룬���ݳ���    		// 
//   �����    ��                           	      				// 
//   ���ú�����SPI���ͳ���SPI_MasterTransmit                //
//**********************************************************//
void mcp2515_read_register_p( u08 adress, u08 *data, u08 length )
{
	u08 i;
	
	// CS low ,MCP2515 enable
	SPI_PORT &= ~(1<<SPI_CS);
	
	SPI_MasterTransmit(SPI_READ);		//����SPI��ָ��
	
	SPI_MasterTransmit(adress);			//������ʼ�Ĵ�����ַ
	
	for (i=0; i<length ;i++ )
		*data++ = SPI_MasterTransmit(0xff);		//���ݱ���
	
	//CS high ,MCP2515 disable
	SPI_PORT |= (1<<SPI_CS);
}
