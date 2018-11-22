#include "usart.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////
//���ʹ��os,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os ʹ��	  
#endif


//********************************************************************************
//V1.0�޸�˵��
//////////////////////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#if 1
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;
};

FILE __stdout;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
	x = x;
}
//�ض���fputc����
int fputc(int ch, FILE *f)
{
	while ((USART2->SR & 0X40) == 0); //ѭ������,ֱ���������
	USART2->DR = (u8) ch;
	return ch;
}
#endif

#if EN_USART2_RX

u8 aRxBuffer1[RXBUFFERSIZE];
u8 aRxBuffer2[RXBUFFERSIZE];
u8 aRxBuffer6[RXBUFFERSIZE];
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
u8 USART_RX_BUF_1[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 USART_RX_BUF_2[USART_REC_LEN];
u8 USART_RX_BUF_6[USART_REC_LEN];
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA_1 = 0;     //����״̬���
u16 USART_RX_STA_2 = 0;
u16 USART_RX_STA_6 = 0;

UART_HandleTypeDef huart2; //UART���
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart6;

uint16_t UsartRxBuffSize = 50;
unsigned short Rx_Head = 0 , Rx_Tail = 0;
char Rx[50];
char all_lin_data[256];

static u8 Lin_Current_State = IDLE;
static u8 data_available_flag = 0;

/* USART1 init function */
void MX_USART1_UART_Init(void)
{

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 19200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	// HAL_UART_Init(&huart1);
	HAL_LIN_Init(&huart1, UART_LINBREAKDETECTLENGTH_11B);
	HAL_UART_Receive_IT(&huart1, (u8 *)aRxBuffer1, RXBUFFERSIZE);

	__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_LBD);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_LBD);


}

//��ʼ��IO ����1
//bound:������
void MX_USART2_UART_Init(void)
{
	//UART ��ʼ������
	huart2.Instance = USART2;					  //USART2
	huart2.Init.BaudRate = 115200;				  //������
	huart2.Init.WordLength = UART_WORDLENGTH_8B; //�ֳ�Ϊ8λ���ݸ�ʽ
	huart2.Init.StopBits = UART_STOPBITS_1;	  //һ��ֹͣλ
	huart2.Init.Parity = UART_PARITY_NONE;		  //����żУ��λ
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE; //��Ӳ������
	huart2.Init.Mode = UART_MODE_TX_RX;		  //�շ�ģʽ
	HAL_UART_Init(&huart2);					    //HAL_UART_Init()��ʹ��UART2

	HAL_UART_Receive_IT(&huart2, (u8 *)aRxBuffer2, RXBUFFERSIZE);
}

/* USART6 init function */
void MX_USART6_UART_Init(void)
{

	huart6.Instance = USART6;
	huart6.Init.BaudRate = 19200;
	huart6.Init.WordLength = UART_WORDLENGTH_8B;
	huart6.Init.StopBits = UART_STOPBITS_1;
	huart6.Init.Parity = UART_PARITY_NONE;
	huart6.Init.Mode = UART_MODE_TX_RX;
	huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart6.Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_LIN_Init(&huart6, UART_LINBREAKDETECTLENGTH_11B);
	HAL_UART_Receive_IT(&huart6, (u8 *)aRxBuffer6, RXBUFFERSIZE);


}


void LIN_Data_Analyse(u8 LIN_Data)
{
	u8  TempData = 0;
	static u8 TempLen = 0;
	static u16 TempCheckSum = 0;
	unsigned short tempRx_Head = (Rx_Head + 1) % UsartRxBuffSize;
	unsigned short tempRx_Tail = Rx_Tail;
	
	switch (Lin_Current_State)
	{
	case 1:
		if (LIN_Data == 0x55)
		{
			Lin_Current_State =  ID_LEN;/*��һ��������ID��Ϣ*/
		} else {
			Lin_Current_State = IDLE;
		}
		break;

	case 2:
		TempData = (~(BIT(LIN_Data, 1)^BIT(LIN_Data, 3)^BIT(LIN_Data, 4)^BIT(LIN_Data, 5))) << 7;
		TempData |= (BIT(LIN_Data, 0)^BIT(LIN_Data, 1)^BIT(LIN_Data, 2)^BIT(LIN_Data, 4)) << 6;
		if (TempData == (LIN_Data & 0xC0))
		{
// 			TempLinData.CMD_ID = LIN_Data;
			TempCheckSum = LIN_Data;
// 			TempCheckSum = TempLinData.CMD_ID;

// 			for (TempNum = 0; TempNum < LIN_ID_NUM; TempNum++)
// 			{
// 				if (TempLinData.CMD_ID == LIN_ID_List[TempNum][0])
// 				{
// 					TempLinData.WorkMode = LIN_ID_List[TempNum][1];
// #if (LIN_LEN_MODE == 0)
// 					TempLinData.Len = LIN_Len[((LIN_Data >> 4) & 0x03)];
// #else
// 					TempLinData.Len = LIN_ID_List[TempNum][2];
// #endif
// 					if (TempLinData.WorkMode == SLAVE_RX_MODE)
// 					{
// 						TempLen = 1;
			Lin_Current_State = DATA_GET;
			// 		} else if (TempLinData.WorkMode == SLAVE_TX_MODE)
			// 		{
			// 			Lin_Current_State = IDLE;
			// 			COMM_LIN_QPushStr(&LIN_RxMsg, &TempLinData);
			// 			OSSemPost(LINRxSem);
			// 		}
			// 		return;
			// 	}
			// }
			// Lin_Current_State = IDLE; TempLinData.CMD_ID = 0; TempLinData.Len = 0;
		} else {
			Lin_Current_State = IDLE;
		}
		break;

	case DATA_GET:

		if (tempRx_Head == tempRx_Tail)
		{
			__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE); //���ܻ������ˣ�ֹͣ��������
		}
		else {
			Rx[Rx_Head] = LIN_Data;
			Rx_Head = tempRx_Head;
		}

		TempCheckSum += LIN_Data;
		if (TempCheckSum & 0xFF00)
			TempCheckSum = (TempCheckSum & 0x00FF) + 1;
		if ( TempLen < 1) //���ݳ���
		{
			TempLen++;
		} else {
			Lin_Current_State = CHECKSUM;
			TempCheckSum = (u8)((~TempCheckSum) & 0x00ff);
			TempLen = 0;
		}
		break;

	case CHECKSUM:
		Lin_Current_State = IDLE;
		if (TempCheckSum != LIN_Data)
		{
			Usart_Flush();
		} else
		{
			data_available_flag = 1;
		}
		break;

	default:
		break;
	}
}

void USART1_IRQHandler(void)
{
	u8 Res;
	static u8 count=0;
#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntEnter();
#endif
	if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_LBD) != RESET))
	{
		// __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_LBD);
		__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_LBD);
		//__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
		Lin_Current_State = SYNCH;
		// __HAL_UART_DISABLE_IT(&huart1, UART_FLAG_TXE);

	}
	if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET))
	{

		__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
		Res = (huart1.Instance->DR & (uint8_t)0x00FF);
		
		all_lin_data[count]=Res;
		count++;
		if(count>255) count =0;
		// unsigned short tempRx_Head = (Rx_Head + 1) % UsartRxBuffSize;
		// unsigned short tempRx_Tail = Rx_Tail;
		// if (tempRx_Head == tempRx_Tail)
		// {
		// 	__HAL_UART_DISABLE_IT(&huart1, UART_IT_LBD); //���ܻ������ˣ�ֹͣ��������
		// }
		// else {
		// 	Rx[Rx_Head] = Res;
		// 	Rx_Head = tempRx_Head;
		// }
		// if (mode == 1)
		
		//LIN_Data_Analyse(Res);

	}
	if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE) != RESET))
	{
		__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_ORE);
		HAL_UART_Receive(&huart1, &Res, 1, 1000);
	}


#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntExit();
#endif
}

//����1�жϷ������
void USART2_IRQHandler(void)
{
	u8 Res;
#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntEnter();
#endif
	if ((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)) //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		HAL_UART_Receive(&huart2, &Res, 1, 1);
		USART_RX_BUF_2[USART_RX_STA_2] = Res ;
		USART_RX_STA_2++;
		if (USART_RX_STA_2 > (USART_REC_LEN - 1))USART_RX_STA_2 = 0;


	}
	HAL_UART_IRQHandler(&huart2);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntExit();
#endif
}

void USART6_IRQHandler(void)
{
	//TODO:�жϽ���ǰ��֡0x00,0x55,���ν���ID,���ݺ�У���;
	u8 Res;
#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntEnter();
#endif
	if ((__HAL_UART_GET_FLAG(&huart6, UART_IT_LBD) != RESET)) //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		HAL_UART_Receive(&huart6, &Res, 1, 1);
		USART_RX_BUF_6[USART_RX_STA_6] = Res ;
		USART_RX_STA_6++;
		if (USART_RX_STA_6 > (USART_REC_LEN - 1))USART_RX_STA_6 = 0;


	}
	HAL_UART_IRQHandler(&huart6);
	__HAL_UART_ENABLE_IT(&huart6, UART_IT_LBD);
#if SYSTEM_SUPPORT_OS	 	//ʹ��OS
	OSIntExit();
#endif
}




uint8_t LINCalID(uint8_t id)

{
	uint8_t parity, p0, p1;
	parity = id;
	p0 = (BIT(parity, 0) ^ BIT(parity, 1) ^ BIT(parity, 2) ^ BIT(parity, 4)) << 6;
	p1 = (!(BIT(parity, 1) ^ BIT(parity, 3) ^ BIT(parity, 4) ^ BIT(parity, 5))) << 7;
	parity |= (p0 | p1);
	return parity;
}



uint8_t LINCalChecksum(uint8_t id, uint8_t *data)

{
	uint32_t sum = id;
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		sum += data[i];
		if (sum & 0xFF00)
		{
			sum = (sum & 0x00FF) + 1;
		}
	}
	sum ^= 0x00FF;
	return (uint8_t)sum;
}

void LIN2_Send_Data( uint8_t ID_before, uint8_t *pData)
{
	OS_ERR err;
	uint8_t ID_after, check_sum;
	uint8_t lin_synchro_field = 0x55;

	ID_after = LINCalID(ID_before);
	check_sum = LINCalChecksum(ID_after, pData);


	HAL_GPIO_WritePin( GPIOG, GPIO_PIN_1, GPIO_PIN_SET);
	OSTimeDlyHMSM(0, 0, 0, 1, OS_OPT_TIME_HMSM_STRICT, &err);
	HAL_GPIO_WritePin( GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
	OSTimeDlyHMSM(0, 0, 0, 1, OS_OPT_TIME_HMSM_STRICT, &err);
	HAL_GPIO_WritePin( GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

	HAL_LIN_SendBreak(&huart6);
	HAL_UART_Transmit(&huart6, &lin_synchro_field, 1, 10);
	HAL_UART_Transmit(&huart6, &ID_after, 1, 10);
	HAL_UART_Transmit(&huart6, (uint8_t*)pData, 8, 10);
	HAL_UART_Transmit(&huart6, &check_sum, 1, 10);

	HAL_GPIO_WritePin( GPIOG, GPIO_PIN_1, GPIO_PIN_RESET);

}






short Usart1_DataAvailable()
{
	uint16_t tempHead  = Rx_Head;
	uint16_t tempTail = Rx_Tail;
	short RxDelta =0;
	if (data_available_flag)
	{
		RxDelta = tempHead - tempTail;
		if (RxDelta < 0)  RxDelta = UsartRxBuffSize + RxDelta;
		
	}
return RxDelta;
}
uint8_t Usart1_Getch()
{
	uint8_t ans;

	__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
	// ans = Rx[Rx_Tail];
	ans = Rx[Rx_Tail];
	Rx_Tail = (Rx_Tail + 1) % UsartRxBuffSize;
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	return ans;
}

void Usart_Flush()
{
	Rx_Tail = Rx_Head = 0;
	memset(Rx, 0, sizeof(Rx));
}

void Usart1Send(unsigned char *Str, int len)
{
	HAL_GPIO_WritePin( GPIOG, GPIO_PIN_0, GPIO_PIN_SET);
	delay_us(1000);
	HAL_GPIO_WritePin( GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
	delay_us(1000);
	HAL_GPIO_WritePin( GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);


	HAL_LIN_SendBreak(&huart1);
	HAL_UART_Transmit(&huart1, Str, len, 1000); //���ͽ��յ�������
	while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET);   //�ȴ����ͽ���
	// for (int i = 0; i < len; i++)
	// {
	// 	HAL_UART_Transmit(&huart1,&Str[i], 1, 1000); //���ͽ��յ�������
	// 	while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET);
	// 	delay_us(100);

	// }
	HAL_GPIO_WritePin( GPIOG, GPIO_PIN_0, GPIO_PIN_RESET);

}
int read(void) {

	if (Usart1_DataAvailable())
	{
		return Usart1_Getch();
	} else
	{
		return 0;
	}

}



#endif
//========================================================================================================

// void LIN_INT_FUNC(void)
// {
// 	uint8_t ret;
// 	if (USART_GetITStatus(LIN_CHANNEL, USART_IT_RXNE))
// 	{
// 		USART_ClearITPendingBit(LIN_CHANNEL, USART_IT_RXNE);
// 		ret = USART_ReceiveData(LIN_CHANNEL);
// 		//LINQueuePush(&lin_recv, ret);
// 	}
// 	if (USART_GetITStatus(LIN_CHANNEL, USART_IT_LBD))
// 	{
// 		USART_ClearITPendingBit(LIN_CHANNEL, USART_IT_LBD); // ��⵽ͬ�������
// 		//LinStatusSet(SYNCH);
// 	}
// 	if (USART_GetFlagStatus(LIN_CHANNEL, USART_FLAG_ORE) == SET) // ���
// 	{
// 		USART_ClearFlag(LIN_CHANNEL, USART_FLAG_ORE);
// 		USART_ReceiveData(LIN_CHANNEL);
// 	}
// }



// void USART1_IRQHandler(void)
// {
// #if SYSTEM_SUPPORT_OS	 	//ʹ��OS
// 	OSIntEnter();
// #endif
// 	if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET))
// 	{
// 		unsigned short tempRx_Head = (Rx_Head + 1) % UsartRxBuffSize;
// 		unsigned short tempRx_Tail = Rx_Tail;
// 		uint8_t data;			//��ȡ���� ע�⣺������Ҫ�������ܹ�����жϱ�־λ
// 		HAL_UART_Receive(&huart1, &data, 1,1);
// 		if (tempRx_Head == tempRx_Tail)
// 		{
// 			__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE); //���ܻ������ˣ�ֹͣ��������
// 		}
// 		else {
// 			Rx[Rx_Head] = data;
// 			Rx_Head = tempRx_Head;
// 		}
// 	}
// 	HAL_UART_IRQHandler(&huart1);
// 	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
// #if SYSTEM_SUPPORT_OS	 	//ʹ��OS
// 	OSIntExit();
// #endif
// }


// void USART1_IRQHandler(void)
// {
// 	u32 timeout = 0;
// 	u32 maxDelay = 0x1FFFF;
// #if SYSTEM_SUPPORT_OS	 	//ʹ��OS
// 	OSIntEnter();
// #endif

// 	HAL_UART_IRQHandler(&huart1);	//����HAL���жϴ����ú���
// 	__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_LBD);
// 	timeout = 0;
// 	while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY)//�ȴ�����
// 	{
// 		timeout++;////��ʱ����
// 		if (timeout > maxDelay) break;
// 	}

// 	timeout = 0;
// 	while (HAL_UART_Receive_IT(&huart1, (u8 *)aRxBuffer1, RXBUFFERSIZE) != HAL_OK) //һ�δ������֮�����¿����жϲ�����RxXferCountΪ1
// 	{
// 		timeout++; //��ʱ����
// 		if (timeout > maxDelay) break;
// 	}

// #if SYSTEM_SUPPORT_OS	 	//ʹ��OS
// 	OSIntExit();
// #endif
// }

// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
// if (huart->Instance == USART1) //����Ǵ���1
// {
// 	unsigned short tempRx_Head = (Rx_Head + 1) % UsartRxBuffSize;
// 	unsigned short tempRx_Tail = Rx_Tail;
// 	uint8_t data = aRxBuffer1[0] ;

// 	if (tempRx_Head == tempRx_Tail)
// 	{
// 		__HAL_UART_DISABLE_IT(&huart1, UART_IT_LBD); //���ܻ������ˣ�ֹͣ��������
// 	}
// 	else {
// 		Rx[Rx_Head] = data;
// 		Rx_Head = tempRx_Head;
// 	}
// }
// }

// void USART1_IRQHandler(void)
// {
// 	u8 Res;
// #if SYSTEM_SUPPORT_OS	 	//ʹ��OS
// 	OSIntEnter();
// #endif
// 	if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_LBD) != RESET)) //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
// 	{
// 		__HAL_UART_DISABLE_IT(&huart1, UART_IT_LBD);
// 		HAL_UART_Receive(&huart1, &Res, 1, 1000);
// 		USART_RX_BUF_1[USART_RX_STA_1] = Res ;
// 		USART_RX_STA_1++;
// 		if (USART_RX_STA_1 > (USART_REC_LEN - 1))USART_RX_STA_1 = 0;


// 	}
// 	HAL_UART_IRQHandler(&huart1);
// 	__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_LBD);
// 	__HAL_UART_ENABLE_IT(&huart1, UART_IT_LBD);
// #if SYSTEM_SUPPORT_OS	 	//ʹ��OS
// 	OSIntExit();
// #endif
// }



