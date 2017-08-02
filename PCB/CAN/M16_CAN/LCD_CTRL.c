/*============================================================
        c file about LCD1602   V1.00
==============================================================
Chip: 					ATmega16+LCD1602(SMC1602A)
Function:				Define some besic function of LCD control
Writer:					Fenghui Zhu
Data:						2009-4-1 
==============================================================
SMC1602A(16*2)ģ����߽��߷�ʽ
������ͼ: 
       ---------------------------------------------------
       |LCM-----M16  |  LCM-----M16 | LCM------M16   |
       ----------------------------------------------|
       |DB0-----PA0 | DB4-----PA4 | RW-------PC0     |
       |DB1-----PA1 | DB5-----PA5 | RS-------PC1     |
       |DB2-----PA2 | DB6-----PA6 | E--------PC7     |
       |DB3-----PA3 | DB7-----PA7 | VLCD��1K���赽GND|
       ---------------------------------------------------

=============================================================*/
//********************* 
//* ͷ �� �� �� �� �� * 
//*********************
#include "LCD_CTRL.h"

//********************** 
//*   �� �� �� �� ��   * 
//**********************

//**********************************************************// 
//   ����˵������LCDд����											            //
//   ���룺    ���ݣ�unsigned char��ʽ	                    // 
//   �����    ��               		                        // 
//   ���ú�������                                           //
//**********************************************************//
//д����
void WriteDataLCD(unsigned char WDLCD)
{
 ReadStatusLCD(); //���æ
 LCD_Data_out = WDLCD;
 SET_LCD_RS;
 CLR_LCD_RW;
 SET_LCD_RS;
 CLR_LCD_RW;
 CLR_LCD_E; //�������ٶ�̫�߿���������С����ʱ
 CLR_LCD_E; //��ʱ
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 SET_LCD_E;
 SET_LCD_E;
}

//**********************************************************// 
//   ����˵������LCDдָ��											            // 
//   ���룺    ����ָ�� WCLCD��æ����־λ��BuysC          // 
//   �����    ��               		                        // 
//   ���ú�������                                           //
//**********************************************************//
void WriteCommandLCD(unsigned char WCLCD,unsigned char BuysC) //BuysCΪ0ʱ����æ���
{
 if (BuysC) ReadStatusLCD(); //������Ҫ���æ
 LCD_Data_out = WCLCD;
 CLR_LCD_RS;
 CLR_LCD_RW; 
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 SET_LCD_E; 
 SET_LCD_E; 
}

//**********************************************************// 
//   ����˵������LCD�����ݼĴ���								            // 
//   ���룺    ��																	          // 
//   �����    �Ĵ�������        		                        // 
//   ���ú�������                                           //
//**********************************************************//
unsigned char ReadDataLCD(void)
{
 SET_LCD_RS; 
 SET_LCD_RW;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 SET_LCD_E;
 SET_LCD_E;
 return(LCD_Data_out);
}

//**********************************************************// 
//   ����˵������LCD��״̬�Ĵ���								            // 
//   ���룺    ��																	          // 
//   �����    LCD״̬          		                        // 
//   ���ú�������                                           //
//**********************************************************//
unsigned char ReadStatusLCD(void)
{
 DDRA=0x00; //����
 CLR_LCD_RS;
 SET_LCD_RW;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 SET_LCD_E;
 while (LCD_Data_in & Busy); //���æ�ź�
 DDRA=0xff;//������
 return(LCD_Data_in);
}


//**********************************************************// 
//   ����˵����LC��ʼ��								           					 // 
//   ���룺    ��																	          // 
//   �����    ��          		                        			// 
//   ���ú�������                                           //
//**********************************************************//
void LCDInit(void) //LCD��ʼ��
{
 LCD_Data_out = 0;
 WriteCommandLCD(0x38,0); //������ʾģʽ���ã������æ�ź�
 Delay5Ms(); 
 WriteCommandLCD(0x38,0);
 Delay5Ms(); 
 WriteCommandLCD(0x38,0);
 Delay5Ms(); 

 WriteCommandLCD(0x38,1); //��ʾģʽ����,��ʼҪ��ÿ�μ��æ�ź�
 WriteCommandLCD(0x08,1); //�ر���ʾ
 WriteCommandLCD(0x01,1); //��ʾ����
 WriteCommandLCD(0x06,1); // ��ʾ����ƶ�����
 WriteCommandLCD(0x0C,1); // ��ʾ�����������
}


//**********************************************************// 
//   ����˵������ָ��λ����ʾһ���ַ�		         					 // 
//   ���룺    X���꣬Y���꣬��ʾ����							          // 
//   �����    ��          		                        			// 
//   ���ú�����WriteCommandLCD(), WriteDataLCD()            //
//**********************************************************//
void DisplayOneChar(unsigned char X, unsigned char Y, unsigned char DData)
{
 Y &= 0x1;
 X &= 0xF; //����X���ܴ���15��Y���ܴ���1
 if (Y) X |= 0x40; //��Ҫ��ʾ�ڶ���ʱ��ַ��+0x40;
 X |= 0x80; //���ָ����
 WriteCommandLCD(X, 0); //���ﲻ���æ�źţ����͵�ַ��
 WriteDataLCD(DData);
 WriteCommandLCD(X, 0); //��귵�أ�����ʱ����Ҫ
}


//**********************************************************// 
//   ����˵������ָ��λ����ʾһ���ַ�		         					 // 
//   ���룺    X���꣬Y���꣬��ʾ����ָ��							      // 
//   �����    ��          		                        			// 
//   ���ú�����DisplayOneChar()                             //
//**********************************************************//
void DisplayListChar(unsigned char X, unsigned char Y, unsigned char const *DData)
{
 unsigned char ListLength;
 ListLength = 0;
 Y &= 0x1;
 X &= 0xF; //����X���ܴ���15��Y���ܴ���1
 while (DData[ListLength]>0x20) //�������ִ�β���˳�
		{
			if(X<=0xF)
    	{
    	DisplayOneChar(X, Y, DData[ListLength]); //��ʾ�����ַ�
     	ListLength++;
     	X++;
    	}
    }
}

//**********************************************************// 
//   ����˵������ʾһ���ַ�          		         					 // 
//   ���룺    Y���꣬��ʾ����ָ��					     		      // 
//   �����    ��          		                        			// 
//   ���ú�����DisplayOneChar()                             //
//**********************************************************//
void DisplayrowChar(unsigned char Y, unsigned char const *DData)		//��ָ��λ����ʾһ���ַ�
{
 unsigned char ListLength,X;
 ListLength = 0;
 Y &= 0x1; //����Y���ܴ���1
 for(X=0;X<=0xf;X++) //�������ִ�β���˳�
		{

    	DisplayOneChar(X, Y, DData[ListLength]); //��ʾ�����ַ�
     	ListLength++;

    }
  
}

//**********************************************************// 
//   ����˵����5ms ��ʱ����          		         					 // 
//   ���룺    ��														     		      // 
//   �����    ��          		                        			// 
//   ���ú�������                          							   //
//**********************************************************//
void Delay5Ms(void)
{
 unsigned int TempCyc = 5552;
 while(TempCyc--);
}


//**********************************************************// 
//   ����˵����400ms��ʱ����          		         					 // 
//   ���룺    ��														     		      // 
//   �����    ��          		                        			// 
//   ���ú�������                          							   //
//**********************************************************//
void Delay400Ms(void)
{
 unsigned char TempCycA = 9;
 unsigned int TempCycB;
 while(TempCycA--)
 {
  TempCycB=7777;
  while(TempCycB--);
 };
}