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
//* �û����Ͷ�����    * 
//*********************

//********************* 
//* ͷ �� �� �� �� �� * 
//*********************
#include <avr/io.h>
#include <avr/iom16.h>
#include "Habit_Frank.h"
//******************** 
//*  ϵ ͳ �� �� ��  * 
//******************** 
#define  SPI_PORT   PORTB			//SPI�˿�
#define  SPI_DDR   DDRB 

#define  SS     4     			//  SPI�洢ʱ��                                                                                       
#define  MOSI   5           //	SPI �������ͣ��ӻ����ܶ˿�			 
#define  MISO		6						//  SPI �ӻ����ͣ��������ܶ˿�
#define  SCK	  7						//	SPI��λʱ��   
#define  CS     4     			//  SPI����CS�ź�

#define  SPI_MOSI   5           //	SPI �������ͣ��ӻ����ܶ˿�			 
#define  SPI_MISO		6						//  SPI �ӻ����ͣ��������ܶ˿�
#define  SPI_SCK	  7						//	SPI��λʱ��   
#define  SPI_CS     4     			//  SPI����CS�ź�

//********************* 
//*   �� �� �� �� ��  * 
//*********************

//********************** 
//*   �� �� �� �� ��   * 
//**********************

//********************** 
//*   ȫ�ֱ���������   * 
//**********************

//********************** 
//*   �� �� �� �� ��   * 
//**********************

/******************** 
*   ȫ�ֺ���������  * 
********************/ 
unsigned char SPI_MasterTransmit(unsigned char cData);			//SPI�������ͳ���
void SPI_MasterInit(void);								//SPI��ʼ����������
void SPI_Disable(void);									//ֹͣSPI
void SPI_Enable(void);										//ʹ��SPI
void SPI_MasterTransmit_P(char cData); 	//SPI�������ͳ���,�г�ʱ����	


#endif
