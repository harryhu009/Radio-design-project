//******************************************************
//���ܣ���LCD1602Ϊ��ʾ��TEA5767ģ���������
//���������KELI C 
//��Ƭ����STC89C52 ����13.56MHZ
//����ߣ������ٵ���zhengzhongxing39
//���ڣ�2018.02.21
#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char  //�޷����ַ��� �궨��	������Χ0~255
#define uint  unsigned int	 //�޷������� �궨��	������Χ0~65535
#include "eeprom52.h"

#define DELAY5US _nop_();_nop_();_nop_();_nop_();_nop_();
//**************TEA5767ģ����߷���********************
sbit SDA=P1^0;     
sbit SCL=P1^1;
//*************Ƶ�ʵ��ڰ�������************************
sbit Key1=P3^5;
sbit Key2=P3^4;
/*******����ܹܽ�����********/
sbit  smg_we1 = P2^7;		//�����λѡ
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
#define ON	0	//��������ܶ��룬0��
#define off  1//��������ܶ��룬1��
#define  display_on  5 //�����º����������ʱ����ƣ���λΪ��

//*****************��������*****************************
unsigned long int  FM_FREQ;  //Ƶ��
unsigned short int FM_PLL;   //PLL
uchar t0_crycle,second_count,msecond_count,display_flag;
uchar idata sbuf[5];   		// ���ݷ��ͻ�����
uchar idata numbyte;
uchar idata numbyte_AMP;
uchar idata ADDRESS_AMP;
uchar idata ADDRESS_SEND;    //�ԣţ���7�������͵�ַ
uchar idata ADDRESS_RECEIVE; //�ԣţ���7�������յ�ַ
uchar idata rbuf[5];   		// ���ݽ��ջ�����
uchar idata ampint[5];
uchar bdata PLL_HIGH;  
uchar bdata PLL_LOW;   //�趨����������Ԥ��Ŀɱ��Ƶ�ʺϳ���
uchar bdata I2C_byte1;//���͵����ֽڣԣţ���7������λѰַ������ֵ
uchar bdata I2C_byte2;
uchar bdata I2C_byte3;
uchar bdata I2C_byte4;
uchar bdata I2C_byte5;
sbit MUTE =I2C_byte1^7;//���MUTE=1��������������������MUTE=0������������������	 
//sbit SM = I2C_byte1^6; //SM=1,��������ģʽ��SM=0������������ģʽ
//sbit SUD=I2C_byte3^7;  //SUD=1������Ƶ��������SUD=0����СƵ������
uchar   byte1;  
uchar   byte2;
uchar   byte3;
uchar   byte4;
uchar   byte5;
uchar num1,num2,num3,num4;
uchar   tab1[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
bit   bdata NACK;    		// �����־λ
uint set_d;	            //Ƶ��
//**********��غ�������**************************************
void    init(void);	    //TEA5767��ʼ��
void    delay600ms(void);  //�ӳ�600ms
void    delay100ms(void);	//�ӳ�100ms
void    delay10ms();
void    delay1ms(void);	//�ӳ�1ms
void    sendnbyte(uchar idata *sla, uchar n);//��sendbyte��������I2C�����ݷ��ͺ���
void    I2C_start(void);	//I2C ���俪ʼ
void    sendbyte(uchar idata *ch);	
void    checkack(void);   //���Ӧ��Ѷ��
void    stop(void);		//I2C�������
void    AMP_sendnbyte(uchar idata *sla,uchar numbyte_AMP);
void    key_scan(void);	//��ɨ��
void    search_up(void);   //����Ƶ�����ϼ�
void    search_down(void); //����Ƶ�����¼�
void    setByte1Byte2(void);  //���õ�һ�ڶ��ֽ�Ƶ��
void    LCMInit(void);//LCD��ʼ
void    DelayMs(uint Ms);//1MS��׼��ʱ����
void    WriteDataLCM		(uchar WDLCM);//LCDģ��д����
void    WriteCommandLCM	(uchar WCLCM,BuysC); //LCDģ��дָ��
uchar   ReadStatusLCM(void);//��LCDģ���æ��
void    DisplayOneChar(uchar X,uchar Y,uchar ASCII);//�ڵ�X+1�еĵ�Y+1λ����ʾһ���ַ�
void    LCDshow(void);
void    DelayMs(uint Ms);
void    read_eeprom();
void    write_eeprom();
void init_eeprom();
void SMG_Num(uint n);	//����ܶ��� С���㲻��ʾ
void SMG_Num_dp(uint n);	//����ܶ��� С������ʾ
void smg_we_switch(uchar i);//�����λѡ
void display();
void init_t0();
/***********************������ʾ����*****************************/


//*****************������******************************
void main(void)
{  
    uchar i;
    display_flag=1;
    numbyte = 5;
	 numbyte_AMP=5;
    
    ADDRESS_SEND = 0xC0;// TEA5767д��ַ 1100 0000
    ADDRESS_RECEIVE=0XC1;//�ԣţ�������������ַ 1100 0001
    ADDRESS_AMP=0X8E;
     for(i=0;i<50;i++)display();//�ϵ���ʱһ���������Դ�ȶ�������ʾ���������ʱ
    init_t0();
    init_eeprom();
    init();      //  ��ʼ��TEA5767
    TR0=1;//����ʱ��
    while(1)
    {          
	    key_scan();		 //��ɨ��
       if(display_flag==1)//�ж�������Ƿ�Ӧ�����������涨ʱ��ر�����ܣ���С�õ���
       {//�������ʾ
          for(i=0;i<10;i++)display();
           //�������ʾƵ�ʳ��򣬵���10����ʾ�����ִ��һ�ΰ��������������ܿ���Ƶ���޸ĵ��ٶ�
        }

       }

}
void timer0() interrupt 1
{
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;
	t0_crycle++;
	if(t0_crycle==2)// 0.1��
	{
	  t0_crycle=0;
     msecond_count++;
      if(msecond_count==10)//1��
      { 
        msecond_count=0;
        second_count++;
        if(second_count>=display_on)
        {
          TR0=0;//�ر�ʱ��
          display_flag=0;//�������ʾ��׼��λ
        }
      }    
	}
}
//********************************************************************************************
void init_t0()
{
   TMOD=0x01;//�趨��ʱ��������ʽ1����ʱ����ʱ50����
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;
	EA=1;//�����ж�
   ET0=1;//����ʱ��0�ж�
	t0_crycle=0;//��ʱ���жϴ���������Ԫ
}
//**************����ɨ�����**************************
void key_scan(void)
{
  if(Key1==0)
  {
 	 display();
    display();//���������ʾ���������ʱȥ�����������������Լ��ᰴ����ʱ�������˸������
 	 if(Key1==0)
 	 {
        TR0=1;//����ʱ��
          display_flag=1;//�������ʾ��׼��λ
        second_count=0;
    	  search_up();	  //Ƶ������
       
  	  }
  }
  if(Key2==0)
  {
 	 display();
    display();//���������ʾ���������ʱȥ�����������������Լ��ᰴ����ʱ�������˸������
 	 if(Key2==0)
 	 {  
         TR0=1;//����ʱ��
          display_flag=1;//�������ʾ��׼��λ
second_count=0;
         search_down();  //Ƶ������
 	  }
  }
}
//****************************************
//��������	
void search_up(void)
{ 
   MUTE=1;			//����
 //  SUD=1;	        //������־λ��Ϊ����
   if(FM_FREQ>108000000){FM_FREQ=87500000;} //	�ж�Ƶ���Ƿ񵽶�
   FM_FREQ=FM_FREQ+100000;			//Ƶ�ʼ�100K
   FM_PLL=(unsigned short)((4000*(FM_FREQ/1000+225))/32768);//����У̣�ֵ
   setByte1Byte2();	//����I2C��һ�ڶ��ֽ�PLL	ֵ
   set_d=FM_FREQ/100000; //
        write_eeprom();			   //��������
}
//*******************************
// ��������
void search_down(void)
{  
  
   MUTE=1;	//����
  // SUD=0;//������־λ��Ϊ����
   if(FM_FREQ<87500000){FM_FREQ=108000000;}  //	�ж�Ƶ���Ƿ񵽵�
   FM_FREQ=FM_FREQ-100000;				 //Ƶ�ʼ�100K
   FM_PLL=(unsigned short)((4000*(FM_FREQ/1000+225))/32768); 	//����У̣�ֵ
   setByte1Byte2();		//����I2C��һ�ڶ��ֽ�PLL	ֵ
   set_d=FM_FREQ/100000; //
        write_eeprom();			   //��������

}
/**************�����Լ�eeprom��ʼ��*****************/
void init_eeprom()
{
	read_eeprom();		//�ȶ�
	if(a_a != 1)		//�µĵ�Ƭ����ʼeeprom
	{
		set_d = 875;//�µĵ�Ƭ����һ��Ĭ��Ƶ��Ϊ87.5M HZ
		a_a = 1;
		write_eeprom();	   //��������
	}	
   if(set_d>1087)set_d=875;
   if(set_d<875)set_d=1087;
}
//******************************************
void init(void)
{ 
    uchar idata sbuf[5]={0XF0,0X2C,0XD0,0X10,0X40};  //�ƣ�ģ��Ԥ��ֵ
    uchar idata rbuf[5]={0X00,0X00,0X00,0X00,0X00};
    uchar idata ampint[5]={0X27,0X40,0X42,0X46,0XC3};
    FM_PLL=0X302C;
    FM_FREQ=set_d*100000; //����Ԥ��Ƶ�ʡ�
    PLL_HIGH=0;
    PLL_LOW=0;
    delay100ms();
    delay100ms();
    P1=0XFF;
    P2=0XFF;
    I2C_byte1=0XF0;  //�ƣ�ģ��Ԥ��ֵ
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
    FM_PLL=(unsigned short)((4000*(FM_FREQ/1000+225))/32768); 	//����У̣�ֵ
    setByte1Byte2();		//����I2C��һ�ڶ��ֽ�PLL	ֵ
}
/******************�����ݱ��浽��Ƭ���ڲ�eeprom��******************/
void write_eeprom()
{
	SectorErase(0x2000);
	byte_write(0x2000, set_d % 256);
	byte_write(0x2001, set_d / 256);
	byte_write(0x2058, a_a);	
}

/******************�����ݴӵ�Ƭ���ڲ�eeprom�ж�����*****************/
void read_eeprom()
{
	set_d  = byte_read(0x2001);
	set_d <<= 8;
	set_d  |= byte_read(0x2000);
	a_a      = byte_read(0x2058);
}
/*
//*********************LCD1602��ʾ����*********************
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
  �趨��ʱʱ��:x*1ms
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
//��n�ֽ������ӳ��� 
void sendnbyte(uchar idata *sla, uchar n)
{          
	uchar idata *p;
    sbuf[0]=I2C_byte1;
    sbuf[1]=I2C_byte2;
    sbuf[2]=I2C_byte3;
    sbuf[3]=I2C_byte4;
	I2C_start();			// ���������ź�
	sendbyte(sla);    		// ���ʹ�������ַ�ֽ�
	checkack();    			// ���Ӧ��λ
    if(F0 == 1)
	{ 
		NACK = 1;
		return;    		// ����Ӧ��������������ô����־λNACK
	}
	p = &sbuf[0];
	while(n--)
	{ 
		sendbyte(p);
		checkack();    	// ���Ӧ��λ
		if (F0 == 1)
		{
			NACK=1;
			return;    	// ����Ӧ��������������ô����־λNACK
		}
		p++;
	}
	stop();    			// ȫ��������ֹͣ
}
//***********************************************
////�ӳ�100ms
void delay100ms()				
{
	uchar i;
	for(i=100;i>0;i--){delay1ms();}
}
//**********************************************
//�ӳ�1ms

void delay1ms(void)	          	
{
	uchar i;
	for(i=1000;i>0;i--){;}
}
//*************************************************
//�ڣӣã�Ϊ��ʱ���ӣģ��ɸ߱�ͼ�ΪI2C���俪ʼ
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
//����һ���ֽ������Ӻ���
void sendbyte(uchar idata *ch)

{ 
	uchar idata n = 8;  
	uchar idata temp;
	temp = *ch;
	while(n--)
	{ 
		if((temp&0x80) == 0x80)    // ��Ҫ���͵��������λΪ1����λ1
		{
			SDA = 1;    // ����λ1
			SCL = 1;
			DELAY5US;
		    SCL = 0; 
			SDA = 0;
		}
		else
		{  
			SDA = 0;    // ������λ0
			SCL = 1;
			DELAY5US;
			SCL = 0;  
		}
		temp = temp<<1;    // ��������һλ
	}
}
//����n�ֽ������ӳ���
void AMP_sendnbyte(uchar idata *sla, uchar n)
{          
	uchar idata *p;
    ampint[0]=byte1;
    ampint[1]=byte2;
    ampint[2]=byte3;
    ampint[3]=byte4;
    ampint[4]=byte5;	
	I2C_start();			// ���������ź�
	sendbyte(sla);    		// ���ʹ�������ַ�ֽ�
	checkack();    			// ���Ӧ��λ
    if(F0 == 1)
	{ 
		NACK = 1;
		return;    		// ����Ӧ��������������ô����־λNACK
	}
	p=&ampint[0];
	while(n--)
	{ 
		sendbyte(p);
		checkack();    	// ���Ӧ��λ
		if (F0 == 1)
		{
			NACK=1;
			return;    	// ����Ӧ��������������ô����־λNACK
		}
		p++;																		  
	}
	stop();    			// ȫ��������ֹͣ
}

void delay10ms()				//�ӳ�10ms
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
void stop(void)	 //�ڣӣã�Ϊ��ʱ���ӣģ��ɵͱ�߼�ΪI2C�������
{
	SDA=0;
	SCL=1;
	DELAY5US;
	SDA=1;
	DELAY5US;
	SCL=0;
}
//Ӧ��λ����Ӻ���
void checkack(void)
{ 
	SDA = 1;    		// Ӧ��λ��飨��p1.0���ó����룬��������˿�д1��
	SCL = 1;
	F0 = 0;
	DELAY5US;
	if(SDA == 1)    	// ��SDA=1������Ӧ����λ��Ӧ���־F0
    F0 = 1;
	SCL = 0;
}
//��һ�ڶ��ֽڣУ̣�ֵ�趨
void setByte1Byte2(void)
{
   PLL_HIGH=(uchar)((FM_PLL >> 8)&0X3f);	 //PLL���ֽ�ֵ
   I2C_byte1=(I2C_byte1&0XC0)|PLL_HIGH;		 //I2C��һ�ֽ�ֵ
   PLL_LOW=(uchar)FM_PLL;					 //PLL���ֽ�ֵ
   I2C_byte2= PLL_LOW;						 //I2C�ڶ��ֽ�ֵ
   sendnbyte(&ADDRESS_SEND,numbyte);	     //I2C���ݷ���
   MUTE=0;
   delay100ms();						  //��ʱ100ms
   sendnbyte(&ADDRESS_SEND,numbyte);	  //I2C	���ݷ���
   DELAY5US;
}
void SMG_Num(uint n)	//����ܶ��� С���㲻��ʾ
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
void SMG_Num_dp(uint n)	//����ܶ��� С������ʾ
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
/***********************����λѡ����*****************************/
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
/***********************������ʾ����*****************************/
void display()
{ 
    uint i;
    i=FM_FREQ/100000000;//��ȡƵ�ʵİ�λ
    if(i!=0)//�жϰ�λ���Ƿ�Ϊ0�����Ϊ0����һ������ܲ���ʾ
    {
      SMG_Num(i);
      smg_we_switch(1);
      delay1ms();
    }
    smg_we_switch(5);//ȫ��
    delay1ms();//ȫ����ʱ�����������ܵ��������ã����Ҫ�������������ɾ���þ伴��
    SMG_Num(FM_FREQ/10000000%10);
    smg_we_switch(2);
    delay1ms();
    smg_we_switch(5);//ȫ��
    delay1ms();//ȫ����ʱ�����������ܵ��������ã����Ҫ�������������ɾ���þ伴��
    SMG_Num_dp(FM_FREQ/1000000%10);// ����ܵ�3λ��С������ʾ
    smg_we_switch(3);
    delay1ms();
    smg_we_switch(5);//ȫ��
    delay1ms();//ȫ����ʱ�����������ܵ��������ã����Ҫ�������������ɾ���þ伴��
    SMG_Num(FM_FREQ/100000%10);
    smg_we_switch(4);
    delay1ms();
    smg_we_switch(5);//ȫ��
    delay1ms();//ȫ����ʱ�����������ܵ��������ã����Ҫ�������������ɾ���þ伴��
}


