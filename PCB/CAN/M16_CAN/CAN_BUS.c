/*============================================================
        CAN BUS Function C file V1.00
==============================================================
MCU: 						ATmega16L
CAN chip:       MCP2515+PCA82C250
Frequency:
Function: 			����CAN BUS���ܺ���
Writer:					Fenghui Zhu
Data:						2009-3-31 
Reference:			"can.c" of Fabian Greif
=============================================================*/
//********************* 
//* ͷ �� �� �� �� �� * 
//*********************
#include "CAN_BUS.h"

//********************** 
//*   �� �� �� �� ��   * 
//**********************
void can_send_message( u32 id, u08 *data, u08 length, u08 flags);
//********************** 
//*   �� �� �� �� ��   * 
//**********************

//************************************************************* 
//   ����˵����CAN BUS������Ϣ���� 
//						 ͨ���Ա�־λ�жϣ����жϱ��ĸ�ʽ�ͱ�����;
//						 ��ɱ���ID�����ݵķ��͹���            			            
//   ���룺    u32 id   �� �ڵ��ʾλ��Ϣ
//						 u08 *data �� ��������
//						 u08 length�� ���ݳ���
//						 u08 flags)�� ��־λ��
//						��־λ����������Ϣ��1.���ݱ���orԶ����������
//																2.��׼��ʽ or ��չ��ʽ
//   �����    ��                     	                       
//   ���ú�����	mcp2515_write_register_p()
//							SPI_MasterInit()
//							mcp2515_write_register()                    						              
//*************************************************************
void can_send_message( u32 id, u08 *data, u08 length, u08 flags)
{
	u08 temp[4]; 
	
	if (length > 8)
		length = 0;
	
//���÷��ͻ�����0 �������ȼ���TXP[1:0] = 11 , Ϊ��߷������ȼ�
	mcp2515_bit_modify( TXB0CTRL, (1<<TXP1)|(1<<TXP0), (1<<TXP1)|(1<<TXP0) );
	
// ���ݱ�־λ��ȷ����ID��ʽ����׼��ʽ����չ��ʽ��ͬʱ������ID
	if (flags & CAN_EID) //��չ��ʽ��11+18λID
	{
		temp[0] = (u08) (id>>21);  //ȡ������ID�ĸ�8λ
		//ȡ������ID�ĵ���λ��������չID�ĸ�2λ��ͬʱ������չ��ʾ��EXIDEʹ��
		temp[1] = (((u08) (id>>16)) & 0x03) | (((u08) (id>>13)) & 0xE0) | (1<<EXIDE); 
		
		temp[2] = (u08) (id>>8);  //ȡ��չID
		temp[3] = (u08) id;
	}
	else 				//��׼��ʽ��11λID
	{
		temp[0] = (u08) (id>>3);		//ȡ������ID�ĸ�8λ
		temp[1] = (u08) (id<<5);	//ȡ������ID�ĵ�3λ , ͬʱ������չ��ʾ��EXIDEΪ0��������
		temp[2] = 0;
		temp[3] = 0;
	}
	mcp2515_write_register_p( TXB0SIDH, temp, 4 );		//����ID
	
	
	// ���ݱ�־λ���趨�����Ƿ�ΪԶ�̷�������ͬʱ�趨�����ֽ���
	if (flags & CAN_RTR)
		mcp2515_write_register( TXB0DLC, length | (1<<RTR) );		//�趨���͵ı���ΪԶ�̷������󣬲����屨�������ֽ���
	else
		mcp2515_write_register( TXB0DLC, length );			// �趨���ĵ������ֽ���
	
	//д�뷢������
	mcp2515_write_register_p( TXB0D0, data, length );
	
// �����ͱ��ģ����ͻ�����ΪTXB0
	PORTB &= ~(1<<SPI_CS);
	SPI_MasterTransmit(SPI_RTS | 0x01);
	PORTB |= (1<<SPI_CS);
}



//************************************************************* 
//   ����˵����CAN BUS������Ϣ���� 
//						
//						            			            
//   ���룺    u32 *id   ��  ���յ��Ľڵ��ʾλ��Ϣָ��
//						 u08 *data ��  ���յ�������
//						 u08 *length�� ���ݳ���
//						 u08 *flags��  ��־λ��Ϣ��
//						��־λ������Ϣ��1.���յ������ĸ����ջ�����
//																
//   �����    ��                     	                       
//   ���ú�����	mcp2515_read_register_p()
//							SPI_MasterInit()
//							mcp2515_read_register()                    						              
//*************************************************************
void can_read_message( u32 *ID, u08 *data, u08 *length, u08 *flags)
{
	u08 Intf_temp;
	u08 ID_temp[4] = {0,0,0,0};
	u08 RXB_CTRL_temp;
	Intf_temp = mcp2515_read_register( CANINTF);		//��ȡmcp2515�жϱ�־λ
	Intf_temp = (Intf_temp & ((1<<RX1IF) |(1<<RX0IF)));  //ȡ��CANINTF�е�RX1IF,RX0IF�ж��Ƿ��л�������
	
	if(Intf_temp == (1<<RX0IF))		//�ж��Ƿ�ΪRX0�ж�
		{
			*flags = 0xA0 ;						//��ʾ���յ�����RX0������
			//RXB_CTRL_temp = mcp2515_read_register( RXB0CTRL );		//��ȡ�����˲������
			
			mcp2515_read_register_p( RXB0SIDH , ID_temp , 4);   //��ȡ4���ֽڵ�ID��Ϣ
			
			*length = mcp2515_read_register( RXB0DLC );         //��ȡ�����ֽڳ���
			
			mcp2515_read_register_p( RXB0D0 , data , *length);  //��ȡ����
			
			mcp2515_bit_modify( CANINTF, (1<<RX0IF), 0 );				//���������0����־
		}
		else if(Intf_temp == (1<<RX1IF))		//�ж��Ƿ�ΪRX1�ж�
					{
						*flags = 0xA1 ;						//��ʾ���յ�����RX1������
						
						mcp2515_read_register_p( RXB1SIDH , ID_temp , 4);   //��ȡ4���ֽڵ�ID��Ϣ
			
						*length = mcp2515_read_register( RXB1DLC );         //��ȡ�����ֽڳ���
			
						mcp2515_read_register_p( RXB1D0 , data , *length);   //��ȡ����
			
						mcp2515_bit_modify( CANINTF, (1<<RX1IF), 0 );				//���������1����־
					}
		*ID = (((u32) ID_temp[0]<<24)|((u32) ID_temp[1]<<16)|((u32) ID_temp[2]<<8)|ID_temp[3]);	//��4��8λ���ݻ�ԭΪ32λ
		
}


