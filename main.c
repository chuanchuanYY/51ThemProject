#include <REGX52.H>
#include "Delay.h"
#include "UART.h"
#include "DS18B20.h"
void SendMessage(unsigned char* msg,int len);
unsigned int crc_cal_value(unsigned char* data_value, unsigned char data_length);
unsigned int CRC;
void writeCoils(int CoilsAddr,int onoff);
 void resultKeepRegister(int StartAddr,int RegisterDataPort);
unsigned char Message[8]={0x01,0x03,0x00,0x00,0x00,0x01};//用于接收报文
int mesgIndex=0;
char mesgFlag=0;
int LEDFlag=0;
float T;//DS18B20温度值
char SlaveAddr=0x01;//从机地址
//unsigned char FunCode[10]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};//功能码
int Addr;//线圈地址或寄存器起始地址
unsigned char Register[2];//一个寄存器2个字节
int RegiNumber;//寄存器数量
void main()
{  
	 DS18B20_ConvertT();		//上电先转换一次温度，防止第一次读数据错误
	 Delay(1000);
   UART_Init();		//串口初始化
	 CRC=crc_cal_value(Message,sizeof(Message)-2);
	 Message[6]=(unsigned char)CRC;
	 Message[7]=(unsigned char)(CRC>>8);
	 SendMessage(Message,sizeof(Message));
	P1_0=0;
	while(1)
	{  
				DS18B20_ConvertT();	//转换温度
		    T=DS18B20_ReadT();	//读取温度
	  	  Register[1]=(char)T;
		   /*  Delay(1000);
		  	UART_SendByte((char)T);
		   */
			if(mesgFlag ==1 )//接收到8位报文
			{
				mesgIndex=0;
				mesgFlag=0;
			  CRC=crc_cal_value(Message,sizeof(Message)-2);
				if(Message[6]==(unsigned char)CRC &&  Message[7]==(unsigned char)(CRC>>8))//验证CRC
				{
					  Addr=(Message[2]<<8);//
					  Addr|=Message[3];
					   RegiNumber=(Message[4]<<8);
					  RegiNumber|=Message[5];
						switch(Message[1])//判断功能码
						{
							case 0x05:writeCoils(Addr,Message[4]==0xFF?0:1);
								break;
							case 0x03:resultKeepRegister(Addr,RegiNumber);
								break;
						}
				}
			}
	//	P2_5=0;给0亮
	}
}

void SendMessage(unsigned char* msg,int len)
{
	int i=0;
	for(i=0;i<len;i++)
	{
		UART_SendByte(msg[i]);
	}
}
void UART_Routine() interrupt 4
{
	if(RI==1)					//如果接收标志位为1，接收到了数据
	{
		  // P2=~SBUF;				//读取数据，取反后输出到LED
		if(mesgIndex>=8)return;
			Message[mesgIndex]=SBUF;//
		  mesgIndex++;
			if(mesgIndex>=8)
			{
				mesgFlag=1;
				mesgIndex=0;
			}
		RI=0;					//接收标志位清0
	}
}
unsigned int crc_cal_value(unsigned char* data_value, unsigned char data_length)
{
		int i;
		unsigned short crc_value = 0xffff;
		while (data_length--) 
		{
				crc_value ^= *data_value++;
				for (i = 0; i < 8; i++)
				{
						if (crc_value & 0x0001)
								crc_value = (crc_value >> 1) ^ 0xA001;
						else
								crc_value = crc_value >> 1;
				}
		}
		return(crc_value);
 }

 //写线圈
 void writeCoils(int CoilsAddr,int onoff)
 {
	 switch(CoilsAddr)
	 {
		  case 0: P2_0=onoff;
			 break;
		  case 1: P2_1=onoff;
			 break;
		  case 2: P2_2=onoff;
			 break;
		  case 3: P2_3=onoff;
			 break;
		  case 4: P2_4=onoff;
			 break;
		  case 5: P2_5=onoff;
			 break;
		  case 6: P2_6=onoff;
			 break;
		  case 7: P2_7=onoff;
			 break;
		 case 8: P1_0=onoff==1?0:1;//风扇
			 break;
	 }
	 SendMessage(Message,sizeof(Message));
 }

 //读保持型寄存器
 int j=0;
 unsigned int crcValue;
 void resultKeepRegister(int StartAddr,int RegisterDataPort)//起始地址 和寄存器数
 {
	 unsigned char retuleMsg[80];
	 int retuleMsgLenth=3;
	 retuleMsg[0]=SlaveAddr;
	 retuleMsg[1]=0x03;
	 retuleMsg[2]=((char)RegisterDataPort)*2;
	 j=0;
	 for(j=StartAddr;j<RegisterDataPort*2;j++)
	 {
		 if(j>=sizeof(Register))break;
		 retuleMsg[j+3] =Register[j];
		 retuleMsgLenth++;
	 }
	   crcValue =crc_cal_value(retuleMsg,retuleMsgLenth);
	   retuleMsg[retuleMsgLenth++]=(unsigned char)crcValue;
	   retuleMsg[retuleMsgLenth++]=(unsigned char)(crcValue>>8);
	   SendMessage(retuleMsg,retuleMsgLenth);
 }
 
  