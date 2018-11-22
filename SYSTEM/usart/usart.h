#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F429开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.csom
//修改日期:2015/6/23
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
//********************************************************************************
//V1.0修改说明 
////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define BIT(A,B)       ((A >> B) & 0x01)
#define RXBUFFERSIZE   1 //缓存大小
#define EN_USART2_RX 			1

extern u8  USART_RX_BUF_1[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART_RX_BUF_2[USART_REC_LEN]; 
extern u8  USART_RX_BUF_6[USART_REC_LEN]; 
extern u16 USART_RX_STA_1;         		//接收状态标记	
extern u16 USART_RX_STA_2;
extern u16 USART_RX_STA_6;
extern UART_HandleTypeDef huart2; //UART句柄
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern u8 aRxBuffer1[RXBUFFERSIZE];
extern u8 aRxBuffer2[RXBUFFERSIZE];
extern u8 aRxBuffer6[RXBUFFERSIZE];


enum LinState {
  IDLE,
  SYNCH,
  ID_LEN,
  DATA_GET,
  CHECKSUM
};

// enum LinErrState {
//   NO_ERR,
//   SYNC_ERR,
//   ID_ERR,
//   CHKSUM_ERR
// };

//如果想串口中断接收，请不要注释以下宏定义

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART6_UART_Init(void);
uint8_t LINCalID(uint8_t id);
uint8_t LINCalChecksum(uint8_t id, uint8_t *data);
void LIN2_Send_Data( uint8_t ID_before, uint8_t *pData);


short Usart1_DataAvailable(void);
void Usart_Flush(void);
int read(void);
void Usart1Send(unsigned char *Str, int len);
void LIN_Data_Analyse(u8 LIN_Data);

#endif
