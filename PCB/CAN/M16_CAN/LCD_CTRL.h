/*============================================================
        h file about LCD1602   V1.00
==============================================================
Chip: 					ATmega16+LCD1602(SMC1602A)
Function:				Define some besic function of LCD control
Writer:					Fenghui Zhu
Data:						2009-4-1 
==============================================================
SMC1602A(16*2)ģ����߽��߷�ʽ
������ͼ: 
       ---------------------------------------------------
       |LCD-----M16  |  LCD-----M16 | LCD------M16   |
       ----------------------------------------------|
       |DB0-----PA0 | DB4-----PA4 | RW-------PC0     |
       |DB1-----PA1 | DB5-----PA5 | RS-------PC1     |
       |DB2-----PA2 | DB6-----PA6 | E--------PC7     |
       |DB3-----PA3 | DB7-----PA7 | VLCD��1K���赽GND|
       ---------------------------------------------------

=============================================================*/
#ifndef _LCD_CTRL_h_
#define _LCD_CTRL_h_

//********************* 
//* ͷ �� �� �� �� �� * 
//*********************
#include <ctype.h>
#include <avr/io.h>
#include <avr/iom16.h>

//******************** 
//*  ϵ ͳ �� �� ��  * 
//******************** 
#define CTLPORT PORTC   //ģ����������PC�ڵ�PC0,PC1,PC7
#define LCD_RS  1       // pin-1 on CTLPORT
#define LCD_RW  0       // pin-0 on CTLPORT
#define LCD_E   7       // pin-7 on CTLPORT

#define DataPortfx      DDRA
#define Busy            0x80
#define LCD_Data_out        PORTA                // ���ݶ˿�PB��
#define LCD_Data_in       PINA                 // ���ݶ˿�PB��

//********************** 
//*   �� �� �� �� ��   * 
//**********************
#define SET_LCD_E       CTLPORT|=(1<<LCD_E)        // LCD: E = 1
#define CLR_LCD_E       CTLPORT&=~(1<<LCD_E)       // LCD: E = 0
#define SET_LCD_RW      CTLPORT|=(1<<LCD_RW)       // LCD: R/W = 1 (read)
#define CLR_LCD_RW      CTLPORT&=~(1<<LCD_RW)      // LCD: R/W = 0 (write)
#define SET_LCD_RS      CTLPORT|=(1<<LCD_RS)       // LCD: R/S = 1 (data reg)
#define CLR_LCD_RS      CTLPORT&=~(1<<LCD_RS)      // LCD: R/S = 0 (status reg)


//********************** 
//*   �� �� �� �� ��   * 
//**********************
void WriteDataLCD(unsigned char WDLCD);
void WriteCommandLCD(unsigned char WCLCD,unsigned char BuysC);
unsigned char ReadDataLCD(void);
unsigned char ReadStatusLCD(void);
void LCDInit(void);
void DisplayOneChar(unsigned char X, unsigned char Y, unsigned char DData);
void DisplayListChar(unsigned char X, unsigned char Y, unsigned char const *DData);
void Delay5Ms(void);
void Delay400Ms(void);
void DisplayrowChar(unsigned char Y, unsigned char const *DData);		//��ʾһ���ַ�






#endif