/*============================================================
        c file about LCD1602   V1.00
==============================================================
Chip: 					ATmega16+LCD1602(SMC1602A)
Function:				Define some besic function of LCD control
Writer:					Fenghui Zhu
Data:						2009-4-1 
==============================================================
SMC1602A(16*2)模拟口线接线方式
连接线图: 
       ---------------------------------------------------
       |LCM-----M16  |  LCM-----M16 | LCM------M16   |
       ----------------------------------------------|
       |DB0-----PA0 | DB4-----PA4 | RW-------PC0     |
       |DB1-----PA1 | DB5-----PA5 | RS-------PC1     |
       |DB2-----PA2 | DB6-----PA6 | E--------PC7     |
       |DB3-----PA3 | DB7-----PA7 | VLCD接1K电阻到GND|
       ---------------------------------------------------

=============================================================*/
//********************* 
//* 头 文 件 配 置 区 * 
//*********************
#include "LCD_CTRL.h"

//********************** 
//*   函 数 定 义 区   * 
//**********************

//**********************************************************// 
//   函数说明：向LCD写数据											            //
//   输入：    数据，unsigned char格式	                    // 
//   输出：    无               		                        // 
//   调用函数：无                                           //
//**********************************************************//
//写数据
void WriteDataLCD(unsigned char WDLCD)
{
 ReadStatusLCD(); //检测忙
 LCD_Data_out = WDLCD;
 SET_LCD_RS;
 CLR_LCD_RW;
 SET_LCD_RS;
 CLR_LCD_RW;
 CLR_LCD_E; //若晶振速度太高可以在这后加小的延时
 CLR_LCD_E; //延时
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 SET_LCD_E;
 SET_LCD_E;
}

//**********************************************************// 
//   函数说明：向LCD写指令											            // 
//   输入：    控制指令 WCLCD，忙检测标志位，BuysC          // 
//   输出：    无               		                        // 
//   调用函数：无                                           //
//**********************************************************//
void WriteCommandLCD(unsigned char WCLCD,unsigned char BuysC) //BuysC为0时忽略忙检测
{
 if (BuysC) ReadStatusLCD(); //根据需要检测忙
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
//   函数说明：从LCD读数据寄存器								            // 
//   输入：    无																	          // 
//   输出：    寄存器数据        		                        // 
//   调用函数：无                                           //
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
//   函数说明：从LCD读状态寄存器								            // 
//   输入：    无																	          // 
//   输出：    LCD状态          		                        // 
//   调用函数：无                                           //
//**********************************************************//
unsigned char ReadStatusLCD(void)
{
 DDRA=0x00; //输入
 CLR_LCD_RS;
 SET_LCD_RW;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 CLR_LCD_E;
 SET_LCD_E;
 while (LCD_Data_in & Busy); //检测忙信号
 DDRA=0xff;//变回输出
 return(LCD_Data_in);
}


//**********************************************************// 
//   函数说明：LC初始化								           					 // 
//   输入：    无																	          // 
//   输出：    无          		                        			// 
//   调用函数：无                                           //
//**********************************************************//
void LCDInit(void) //LCD初始化
{
 LCD_Data_out = 0;
 WriteCommandLCD(0x38,0); //三次显示模式设置，不检测忙信号
 Delay5Ms(); 
 WriteCommandLCD(0x38,0);
 Delay5Ms(); 
 WriteCommandLCD(0x38,0);
 Delay5Ms(); 

 WriteCommandLCD(0x38,1); //显示模式设置,开始要求每次检测忙信号
 WriteCommandLCD(0x08,1); //关闭显示
 WriteCommandLCD(0x01,1); //显示清屏
 WriteCommandLCD(0x06,1); // 显示光标移动设置
 WriteCommandLCD(0x0C,1); // 显示开及光标设置
}


//**********************************************************// 
//   函数说明：按指定位置显示一个字符		         					 // 
//   输入：    X坐标，Y坐标，显示数据							          // 
//   输出：    无          		                        			// 
//   调用函数：WriteCommandLCD(), WriteDataLCD()            //
//**********************************************************//
void DisplayOneChar(unsigned char X, unsigned char Y, unsigned char DData)
{
 Y &= 0x1;
 X &= 0xF; //限制X不能大于15，Y不能大于1
 if (Y) X |= 0x40; //当要显示第二行时地址码+0x40;
 X |= 0x80; //算出指令码
 WriteCommandLCD(X, 0); //这里不检测忙信号，发送地址码
 WriteDataLCD(DData);
 WriteCommandLCD(X, 0); //光标返回，调节时很重要
}


//**********************************************************// 
//   函数说明：按指定位置显示一串字符		         					 // 
//   输入：    X坐标，Y坐标，显示数据指针							      // 
//   输出：    无          		                        			// 
//   调用函数：DisplayOneChar()                             //
//**********************************************************//
void DisplayListChar(unsigned char X, unsigned char Y, unsigned char const *DData)
{
 unsigned char ListLength;
 ListLength = 0;
 Y &= 0x1;
 X &= 0xF; //限制X不能大于15，Y不能大于1
 while (DData[ListLength]>0x20) //若到达字串尾则退出
		{
			if(X<=0xF)
    	{
    	DisplayOneChar(X, Y, DData[ListLength]); //显示单个字符
     	ListLength++;
     	X++;
    	}
    }
}

//**********************************************************// 
//   函数说明：显示一行字符          		         					 // 
//   输入：    Y坐标，显示数据指针					     		      // 
//   输出：    无          		                        			// 
//   调用函数：DisplayOneChar()                             //
//**********************************************************//
void DisplayrowChar(unsigned char Y, unsigned char const *DData)		//按指定位置显示一行字符
{
 unsigned char ListLength,X;
 ListLength = 0;
 Y &= 0x1; //限制Y不能大于1
 for(X=0;X<=0xf;X++) //若到达字串尾则退出
		{

    	DisplayOneChar(X, Y, DData[ListLength]); //显示单个字符
     	ListLength++;

    }
  
}

//**********************************************************// 
//   函数说明：5ms 延时程序          		         					 // 
//   输入：    无														     		      // 
//   输出：    无          		                        			// 
//   调用函数：无                          							   //
//**********************************************************//
void Delay5Ms(void)
{
 unsigned int TempCyc = 5552;
 while(TempCyc--);
}


//**********************************************************// 
//   函数说明：400ms延时程序          		         					 // 
//   输入：    无														     		      // 
//   输出：    无          		                        			// 
//   调用函数：无                          							   //
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
