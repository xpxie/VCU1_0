#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2015/6/23
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.0�޸�˵�� 
////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define BIT(A,B)       ((A >> B) & 0x01)
#define RXBUFFERSIZE   1 //�����С
#define EN_USART2_RX 			1

extern u8  USART_RX_BUF_1[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u8  USART_RX_BUF_2[USART_REC_LEN]; 
extern u8  USART_RX_BUF_6[USART_REC_LEN]; 
extern u16 USART_RX_STA_1;         		//����״̬���	
extern u16 USART_RX_STA_2;
extern u16 USART_RX_STA_6;
extern UART_HandleTypeDef huart2; //UART���
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

//����봮���жϽ��գ��벻Ҫע�����º궨��

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
