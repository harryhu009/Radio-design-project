//******************************************************
//功能：以LCD1602为显示的TEA5767模块的收音机
//编译软件：KELI C 
//单片机：STC89C52 晶振：13.56MHZ
//设计者：兴向荣电子zhengzhongxing39
//日期：2018.02.21
#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char  //无符号字符型 宏定义	变量范围0~255
#define uint  unsigned int	 //无符号整型 宏定义	变量范围0~65535
#include "eeprom52.h"

#define DELAY5US _nop_();_nop_();_nop_();_nop_();_nop_();
//**************TEA5767模块接线方法********************
sbit SDA=P1^0;     
sbit SCL=P1^1;
//*************频率调节按键接线************************
sbit Key1=P3^5;
sbit Key2=P3^4;
/*******数码管管脚配置********/
sbit  smg_we1 = P2^7;		//数码管位选
sbit  smg_we2 = P2^6;
sbit  smg_we3 = P2^4;
sbit  smg_we4 = P2^5;

sbit SMG_A = P0^0;
sbit SMG_B = P0^7;
sbit SMG_C = P0^5;
sbit SMG_D = P0^3;
sbit SMG_E = P0^2;
sbit SMG_F = P0^1;
sbit SMG_G = P0^6;
sbit SMG_DP =P0^4;
#define ON	0	//共阳数码管段码，0亮
#define off  1//共阳数码管段码，1灭
#define  display_on  5 //按键下后，数码管亮的时间控制，单位为秒

//*****************参数定义*****************************
unsigned long int  FM_FREQ;  //频率
unsigned short int FM_PLL;   //PLL
uchar t0_crycle,second_count,msecond_count,display_flag;
uchar idata sbuf[5];   		// 数据发送缓冲区
uchar idata numbyte;
uchar idata numbyte_AMP;
uchar idata ADDRESS_AMP;
uchar idata ADDRESS_SEND;    //ＴＥＡ５7６７发送地址
uchar idata ADDRESS_RECEIVE; //ＴＥＡ５7６７接收地址
uchar idata rbuf[5];   		// 数据接收缓冲区
uchar idata ampint[5];
uchar bdata PLL_HIGH;  
uchar bdata PLL_LOW;   //设定用于搜索和预设的可编程频率合成器
uchar bdata I2C_byte1;//发送的五字节ＴＥＡ５7６７可位寻址的设置值
uchar bdata I2C_byte2;
uchar bdata I2C_byte3;
uchar bdata I2C_byte4;
uchar bdata I2C_byte5;
sbit MUTE =I2C_byte1^7;//如果MUTE=1，则左右声道被静音；MUTE=0，左右声道正常工作	 
//sbit SM = I2C_byte1^6; //SM=1,则处于搜索模式；SM=0，不处于搜索模式
//sbit SUD=I2C_byte3^7;  //SUD=1，增加频率搜索；SUD=0，减小频率搜索
uchar   byte1;  
uchar   byte2;
uchar   byte3;
uchar   byte4;
uchar   byte5;
uchar num1,num2,num3,num4;
uchar   tab1[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
bit   bdata NACK;    		// 错误标志位
uint set_d;	            //频率
//**********相关函数声明**************************************
void    init(void);	    //TEA5767初始化
void    delay600ms(void);  //延迟600ms
void    delay100ms(void);	//延迟100ms
void    delay10ms();
void    delay1ms(void);	//延迟1ms
void    sendnbyte(uchar idata *sla, uchar n);//与sendbyte函数构成I2C　数据发送函数
void    I2C_start(void);	//I2C 传输开始
void    sendbyte(uchar idata *ch);	
void    checkack(void);   //检查应答讯号
void    stop(void);		//I2C传输结束
void    AMP_sendnbyte(uchar idata *sla,uchar numbyte_AMP);
void    key_scan(void);	//键扫描
void    search_up(void);   //接收频率向上加
void    search_down(void); //接收频率向下减
void    setByte1Byte2(void);  //设置第一第二字节频率
void    LCMInit(void);//LCD初始
void    DelayMs(uint Ms);//1MS基准延时程序
void    WriteDataLCM		(uchar WDLCM);//LCD模块写数据
void    WriteCommandLCM	(uchar WCLCM,BuysC); //LCD模块写指令
uchar   ReadStatusLCM(void);//读LCD模块的忙标
void    DisplayOneChar(uchar X,uchar Y,uchar ASCII);//在第X+1行的第Y+1位置显示一个字符
void    LCDshow(void);
void    DelayMs(uint Ms);
void    read_eeprom();
void    write_eeprom();
void init_eeprom();
void SMG_Num(uint n);	//数码管段码 小数点不显示
void SMG_Num_dp(uint n);	//数码管段码 小数点显示
void smg_we_switch(uchar i);//数码管位选
void display();
void init_t0();
/***********************数码显示函数*****************************/


//*****************主程序******************************
void main(void)
{  
    uchar i;
    display_flag=1;
    numbyte = 5;
	 numbyte_AMP=5;
    
    ADDRESS_SEND = 0xC0;// TEA5767写地址 1100 0000
    ADDRESS_RECEIVE=0XC1;//ＴＥＡ５７６７读地址 1100 0001
    ADDRESS_AMP=0X8E;
     for(i=0;i<50;i++)display();//上电延时一会儿，待电源稳定，用显示程序代替延时
    init_t0();
    init_eeprom();
    init();      //  初始化TEA5767
    TR0=1;//开启时钟
    while(1)
    {          
	    key_scan();		 //键扫描
       if(display_flag==1)//判断数码管是否应该亮，超过规定时间关闭数码管，减小用电量
       {//数码管显示
          for(i=0;i<10;i++)display();
           //数码管显示频率程序，调用10次显示程序后执行一次按键检查程序，这样能控制频率修改的速度
        }

       }

}
void timer0() interrupt 1
{
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;
	t0_crycle++;
	if(t0_crycle==2)// 0.1秒
	{
	  t0_crycle=0;
     msecond_count++;
      if(msecond_count==10)//1秒
      { 
        msecond_count=0;
        second_count++;
        if(second_count>=display_on)
        {
          TR0=0;//关闭时钟
          display_flag=0;//数码管显示标准复位
        }
      }    
	}
}
//********************************************************************************************
void init_t0()
{
   TMOD=0x01;//设定定时器工作方式1，定时器定时50毫秒
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;
	EA=1;//开总中断
   ET0=1;//允许定时器0中断
	t0_crycle=0;//定时器中断次数计数单元
}
//**************按键扫描程序**************************
void key_scan(void)
{
  if(Key1==0)
  {
 	 display();
    display();//用数码管显示程序代替延时去按键抖动，这样可以减轻按按键时数码管闪烁的问题
 	 if(Key1==0)
 	 {
        TR0=1;//开启时钟
          display_flag=1;//数码管显示标准置位
        second_count=0;
    	  search_up();	  //频率向上
       
  	  }
  }
  if(Key2==0)
  {
 	 display();
    display();//用数码管显示程序代替延时去按键抖动，这样可以减轻按按键时数码管闪烁的问题
 	 if(Key2==0)
 	 {  
         TR0=1;//开启时钟
          display_flag=1;//数码管显示标准置位
second_count=0;
         search_down();  //频率向下
 	  }
  }
}
//****************************************
//向上搜索	
void search_up(void)
{ 
   MUTE=1;			//静音
 //  SUD=1;	        //搜索标志位设为向上
   if(FM_FREQ>108000000){FM_FREQ=87500000;} //	判断频率是否到顶
   FM_FREQ=FM_FREQ+100000;			//频率加100K
   FM_PLL=(unsigned short)((4000*(FM_FREQ/1000+225))/32768);//计算ＰＬＬ值
   setByte1Byte2();	//设置I2C第一第二字节PLL	值
   set_d=FM_FREQ/100000; //
        write_eeprom();			   //保存数据
}
//*******************************
// 向下搜索
void search_down(void)
{  
  
   MUTE=1;	//静音
  // SUD=0;//搜索标志位设为向下
   if(FM_FREQ<87500000){FM_FREQ=108000000;}  //	判断频率是否到底
   FM_FREQ=FM_FREQ-100000;				 //频率减100K
   FM_PLL=(unsigned short)((4000*(FM_FREQ/1000+225))/32768); 	//计算ＰＬＬ值
   setByte1Byte2();		//设置I2C第一第二字节PLL	值
   set_d=FM_FREQ/100000; //
        write_eeprom();			   //保存数据

}
/**************开机自检eeprom初始化*****************/
void init_eeprom()
{
	read_eeprom();		//先读
	if(a_a != 1)		//新的单片机初始eeprom
	{
		set_d = 875;//新的单片机第一次默认频率为87.5M HZ
		a_a = 1;
		write_eeprom();	   //保存数据
	}	
   if(set_d>1087)set_d=875;
   if(set_d<875)set_d=1087;
}
//******************************************
void init(void)
{ 
    uchar idata sbuf[5]={0XF0,0X2C,0XD0,0X10,0X40};  //ＦＭ模块预设值
    uchar idata rbuf[5]={0X00,0X00,0X00,0X00,0X00};
    uchar idata ampint[5]={0X27,0X40,0X42,0X46,0XC3};
    FM_PLL=0X302C;
    FM_FREQ=set_d*100000; //开机预设频率　
    PLL_HIGH=0;
    PLL_LOW=0;
    delay100ms();
    delay100ms();
    P1=0XFF;
    P2=0XFF;
    I2C_byte1=0XF0;  //ＦＭ模块预设值
    I2C_byte2=0X2C;
    I2C_byte3=0XD0;
    I2C_byte4=0X10;
    I2C_byte5=0X40;
    byte1=0X27;  
    byte2=0X40;
    byte3=0X42;
    byte4=0X46;
    byte5=0XC3;
    sendnbyte(&ADDRESS_SEND,numbyte);
    delay100ms();
    AMP_sendnbyte(&ADDRESS_AMP,numbyte_AMP);
    FM_PLL=(unsigned short)((4000*(FM_FREQ/1000+225))/32768); 	//计算ＰＬＬ值
    setByte1Byte2();		//设置I2C第一第二字节PLL	值
}
/******************把数据保存到单片机内部eeprom中******************/
void write_eeprom()
{
	SectorErase(0x2000);
	byte_write(0x2000, set_d % 256);
	byte_write(0x2001, set_d / 256);
	byte_write(0x2058, a_a);	
}

/******************把数据从单片机内部eeprom中读出来*****************/
void read_eeprom()
{
	set_d  = byte_read(0x2001);
	set_d <<= 8;
	set_d  |= byte_read(0x2000);
	a_a      = byte_read(0x2058);
}
/*
//*********************LCD1602显示程序*********************
void LCDshow(void)
{
      num1=FM_FREQ/100000000;
      num2=(FM_FREQ%100000000)/10000000;
      num3=(FM_FREQ%10000000)/1000000;
      num4=(FM_FREQ%1000000)/100000;
      DisplayOneChar(0, 4,'F');//
       DisplayOneChar(0, 5,'M');//
       DisplayOneChar(0, 6,'R');//
       DisplayOneChar(0, 7,'a');//
       DisplayOneChar(0, 8,'d');//
       DisplayOneChar(0, 9,'i');//
       DisplayOneChar(0, 10,'o');//
       DisplayOneChar(1, 4, tab1[num1]);
       DisplayOneChar(1, 5, tab1[num2]);
       DisplayOneChar(1, 6, tab1[num3]);
       DisplayOneChar(1, 7, '.');
       DisplayOneChar(1, 8, tab1[num4]);
       DisplayOneChar(1, 9,'M');//
       DisplayOneChar(1, 10,'H');//
       DisplayOneChar(1, 11,'Z');//
}*/



/*====================================================================
  设定延时时间:x*1ms
====================================================================*/
void DelayMs(uint Ms)
{
  uint i,TempCyc;
  for(i=0;i<Ms;i++)
  {
    TempCyc = 250;
    while(TempCyc--);
  }
}

//************************************************
//送n字节数据子程序 
void sendnbyte(uchar idata *sla, uchar n)
{          
	uchar idata *p;
    sbuf[0]=I2C_byte1;
    sbuf[1]=I2C_byte2;
    sbuf[2]=I2C_byte3;
    sbuf[3]=I2C_byte4;
	I2C_start();			// 发送启动信号
	sendbyte(sla);    		// 发送从器件地址字节
	checkack();    			// 检查应答位
    if(F0 == 1)
	{ 
		NACK = 1;
		return;    		// 若非应答表明器件错误置错误标志位NACK
	}
	p = &sbuf[0];
	while(n--)
	{ 
		sendbyte(p);
		checkack();    	// 检查应答位
		if (F0 == 1)
		{
			NACK=1;
			return;    	// 若非应答表明器件错误置错误标志位NACK
		}
		p++;
	}
	stop();    			// 全部发完则停止
}
//***********************************************
////延迟100ms
void delay100ms()				
{
	uchar i;
	for(i=100;i>0;i--){delay1ms();}
}
//**********************************************
//延迟1ms

void delay1ms(void)	          	
{
	uchar i;
	for(i=1000;i>0;i--){;}
}
//*************************************************
//在ＳＣＬ为高时，ＳＤＡ由高变低即为I2C传输开始
void I2C_start(void)   
{
   SDA=1;
   SCL=1;
   DELAY5US;
   SDA=0;
   DELAY5US;
   SCL=0;
}
//****************************************************
//发送一个字节数据子函数
void sendbyte(uchar idata *ch)

{ 
	uchar idata n = 8;  
	uchar idata temp;
	temp = *ch;
	while(n--)
	{ 
		if((temp&0x80) == 0x80)    // 若要发送的数据最高位为1则发送位1
		{
			SDA = 1;    // 传送位1
			SCL = 1;
			DELAY5US;
		    SCL = 0; 
			SDA = 0;
		}
		else
		{  
			SDA = 0;    // 否则传送位0
			SCL = 1;
			DELAY5US;
			SCL = 0;  
		}
		temp = temp<<1;    // 数据左移一位
	}
}
//发送n字节数据子程序
void AMP_sendnbyte(uchar idata *sla, uchar n)
{          
	uchar idata *p;
    ampint[0]=byte1;
    ampint[1]=byte2;
    ampint[2]=byte3;
    ampint[3]=byte4;
    ampint[4]=byte5;	
	I2C_start();			// 发送启动信号
	sendbyte(sla);    		// 发送从器件地址字节
	checkack();    			// 检查应答位
    if(F0 == 1)
	{ 
		NACK = 1;
		return;    		// 若非应答表明器件错误置错误标志位NACK
	}
	p=&ampint[0];
	while(n--)
	{ 
		sendbyte(p);
		checkack();    	// 检查应答位
		if (F0 == 1)
		{
			NACK=1;
			return;    	// 若非应答表明器件错误置错误标志位NACK
		}
		p++;																		  
	}
	stop();    			// 全部发完则停止
}

void delay10ms()				//延迟10ms
{
     uchar i,j;
	 for(i=900;i>0;i--)
	 {for(j=100;j>0;j--){;}}
}
//***************************************************
void delay600ms()
{
   uchar i;
   for(i=600;i>0;i--){delay1ms();}
}
void stop(void)	 //在ＳＣＬ为高时，ＳＤＡ由低变高即为I2C传输结束
{
	SDA=0;
	SCL=1;
	DELAY5US;
	SDA=1;
	DELAY5US;
	SCL=0;
}
//应答位检查子函数
void checkack(void)
{ 
	SDA = 1;    		// 应答位检查（将p1.0设置成输入，必须先向端口写1）
	SCL = 1;
	F0 = 0;
	DELAY5US;
	if(SDA == 1)    	// 若SDA=1表明非应答，置位非应答标志F0
    F0 = 1;
	SCL = 0;
}
//第一第二字节ＰＬＬ值设定
void setByte1Byte2(void)
{
   PLL_HIGH=(uchar)((FM_PLL >> 8)&0X3f);	 //PLL高字节值
   I2C_byte1=(I2C_byte1&0XC0)|PLL_HIGH;		 //I2C第一字节值
   PLL_LOW=(uchar)FM_PLL;					 //PLL低字节值
   I2C_byte2= PLL_LOW;						 //I2C第二字节值
   sendnbyte(&ADDRESS_SEND,numbyte);	     //I2C数据发送
   MUTE=0;
   delay100ms();						  //延时100ms
   sendnbyte(&ADDRESS_SEND,numbyte);	  //I2C	数据发送
   DELAY5US;
}
void SMG_Num(uint n)	//数码管段码 小数点不显示
{
	switch (n)
	{
		case 0:	
         SMG_A = ON; 
         SMG_B = ON; 
         SMG_C = ON; 
         SMG_D = ON;
         SMG_E = ON;
         SMG_F = ON;
         SMG_G = off;
         SMG_DP =off;
			break;
		case 1: 
         SMG_A = off; 
         SMG_B = ON; 
         SMG_C = ON; 
         SMG_D = off;
         SMG_E = off;
         SMG_F = off;
         SMG_G = off;
         SMG_DP =off;
			break;
		case 2:
         SMG_A = ON; 
         SMG_B = ON; 
         SMG_C = off; 
         SMG_D = ON;
         SMG_E = ON;
         SMG_F = off;
         SMG_G = ON;
         SMG_DP =off;
			break;
		case 3:	
         SMG_A = ON;
         SMG_B = ON; 
         SMG_C = ON; 
         SMG_D = ON; 
         SMG_E = off;
         SMG_F = off;
         SMG_G = ON;
         SMG_DP =off;
			break;
		case 4: 
         SMG_A = off;
         SMG_B = ON;
         SMG_C = ON;
         SMG_D = off;
         SMG_E = off;
         SMG_F = ON; 
         SMG_G = ON;
         SMG_DP =off;
			break;
		case 5:	
        SMG_A = ON;SMG_B = off; SMG_C = ON; SMG_D = ON;SMG_E = off; SMG_F = ON; SMG_G = ON; SMG_DP =off;
			break;
		case 6:	SMG_A = ON; SMG_B = off;SMG_C = ON; SMG_D = ON; SMG_E = ON; SMG_F = ON; SMG_G = ON; SMG_DP =off;
			break;
		case 7:	SMG_A = ON; SMG_B = ON; SMG_C = ON;SMG_D = off; SMG_E = off; SMG_F = off; SMG_G = off; SMG_DP =off;
			break;
		case 8:	SMG_A = ON; SMG_B = ON; SMG_C = ON; SMG_D = ON; SMG_E = ON; SMG_F = ON; SMG_G = ON;SMG_DP =off;
			break;
		case 9:	SMG_A = ON; SMG_B = ON; SMG_C = ON; SMG_D = ON; SMG_E = off;SMG_F = ON; SMG_G = ON;SMG_DP =off;
			break;
	}
}
void SMG_Num_dp(uint n)	//数码管段码 小数点显示
{
	switch (n)
	{
		case 0:	
         SMG_A = ON; 
         SMG_B = ON; 
         SMG_C = ON; 
         SMG_D = ON;
         SMG_E = ON;
         SMG_F = ON;
         SMG_G = off;
         SMG_DP =ON;
			break;
		case 1: 
         SMG_A = off; 
         SMG_B = ON; 
         SMG_C = ON; 
         SMG_D = off;
         SMG_E = off;
         SMG_F = off;
         SMG_G = off;
         SMG_DP =ON;
			break;
		case 2:
         SMG_A = ON; 
         SMG_B = ON; 
         SMG_C = off; 
         SMG_D = ON;
         SMG_E = ON;
         SMG_F = off;
         SMG_G = ON;
         SMG_DP =ON;
			break;
		case 3:	
         SMG_A = ON;
         SMG_B = ON; 
         SMG_C = ON; 
         SMG_D = ON; 
         SMG_E = off;
         SMG_F = off;
         SMG_G = ON;
         SMG_DP =ON;
			break;
		case 4: 
         SMG_A = off;
         SMG_B = ON;
         SMG_C = ON;
         SMG_D = off;
         SMG_E = off;
         SMG_F = ON; 
         SMG_G = ON;
         SMG_DP =ON;
			break;
		case 5:	
        SMG_A = ON;SMG_B = off; SMG_C = ON; SMG_D = ON;SMG_E = off; SMG_F = ON; SMG_G = ON; SMG_DP =ON;
			break;
		case 6:
      	SMG_A = ON; SMG_B = off;SMG_C = ON; SMG_D = ON; SMG_E = ON; SMG_F = ON; SMG_G = ON; SMG_DP =ON;
			break;
		case 7:	SMG_A = ON; SMG_B = ON; SMG_C = ON;SMG_D = off; SMG_E = off; SMG_F = off; SMG_G = off; SMG_DP =ON;
			break;
		case 8:	SMG_A = ON; SMG_B = ON; SMG_C = ON; SMG_D = ON; SMG_E = ON; SMG_F = ON; SMG_G = ON;SMG_DP =ON;
			break;
		case 9:	SMG_A = ON; SMG_B = ON; SMG_C = ON; SMG_D = ON; SMG_E = off;SMG_F = ON; SMG_G = ON;SMG_DP =ON;
			break;
	}
}
/***********************数码位选函数*****************************/
void smg_we_switch(uchar i)
{
	switch(i)
	{
		case 1: smg_we1 = ON;   smg_we2 = off; smg_we3 = off;  smg_we4 = off; break;
		case 2: smg_we1 = off;  smg_we2 =ON;   smg_we3 = off;  smg_we4 = off; break;
		case 3: smg_we1 = off;  smg_we2 = off; smg_we3 = ON;   smg_we4 = off; break;
		case 4: smg_we1 = off;  smg_we2 = off; smg_we3 = off;  smg_we4 = ON; break;
      case 5:  smg_we1 = off;  smg_we2 = off; smg_we3 = off;  smg_we4 = off; break;
	}	
}
/***********************数码显示函数*****************************/
void display()
{ 
    uint i;
    i=FM_FREQ/100000000;//求取频率的百位
    if(i!=0)//判断百位数是否为0，如果为0，第一个数码管不显示
    {
      SMG_Num(i);
      smg_we_switch(1);
      delay1ms();
    }
    smg_we_switch(5);//全灭，
    delay1ms();//全灭延时，起控制数码管的亮度作用，如果要让数码管最亮，删除该句即可
    SMG_Num(FM_FREQ/10000000%10);
    smg_we_switch(2);
    delay1ms();
    smg_we_switch(5);//全灭
    delay1ms();//全灭延时，起控制数码管的亮度作用，如果要让数码管最亮，删除该句即可
    SMG_Num_dp(FM_FREQ/1000000%10);// 数码管第3位带小数点显示
    smg_we_switch(3);
    delay1ms();
    smg_we_switch(5);//全灭
    delay1ms();//全灭延时，起控制数码管的亮度作用，如果要让数码管最亮，删除该句即可
    SMG_Num(FM_FREQ/100000%10);
    smg_we_switch(4);
    delay1ms();
    smg_we_switch(5);//全灭
    delay1ms();//全灭延时，起控制数码管的亮度作用，如果要让数码管最亮，删除该句即可
}


