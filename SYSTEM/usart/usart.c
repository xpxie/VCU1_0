#include "usart.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////
//如果使用os,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif


//********************************************************************************
//V1.0修改说明
//////////////////////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
	while ((USART2->SR & 0X40) == 0); //循环发送,直到发送完毕
	USART2->DR = (u8) ch;
	return ch;
}
#endif

#if EN_USART2_RX

u8 aRxBuffer1[RXBUFFERSIZE];
u8 aRxBuffer2[RXBUFFERSIZE];
u8 aRxBuffer6[RXBUFFERSIZE];
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF_1[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 USART_RX_BUF_2[USART_REC_LEN];
u8 USART_RX_BUF_6[USART_REC_LEN];
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA_1 = 0;     //接收状态标记
u16 USART_RX_STA_2 = 0;
u16 USART_RX_STA_6 = 0;

UART_HandleTypeDef huart2; //UART句柄
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

//初始化IO 串口1
//bound:波特率
void MX_USART2_UART_Init(void)
{
	//UART 初始化设置
	huart2.Instance = USART2;					  //USART2
	huart2.Init.BaudRate = 115200;				  //波特率
	huart2.Init.WordLength = UART_WORDLENGTH_8B; //字长为8位数据格式
	huart2.Init.StopBits = UART_STOPBITS_1;	  //一个停止位
	huart2.Init.Parity = UART_PARITY_NONE;		  //无奇偶校验位
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE; //无硬件流控
	huart2.Init.Mode = UART_MODE_TX_RX;		  //收发模式
	HAL_UART_Init(&huart2);					    //HAL_UART_Init()会使能UART2

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
			Lin_Current_State =  ID_LEN;/*下一个将接收ID信息*/
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
			__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE); //接受缓存满了，停止接收数据
		}
		else {
			Rx[Rx_Head] = LIN_Data;
			Rx_Head = tempRx_Head;
		}

		TempCheckSum += LIN_Data;
		if (TempCheckSum & 0xFF00)
			TempCheckSum = (TempCheckSum & 0x00FF) + 1;
		if ( TempLen < 1) //数据长度
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
#if SYSTEM_SUPPORT_OS	 	//使用OS
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
		// 	__HAL_UART_DISABLE_IT(&huart1, UART_IT_LBD); //接受缓存满了，停止接收数据
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


#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntExit();
#endif
}

//串口1中断服务程序
void USART2_IRQHandler(void)
{
	u8 Res;
#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntEnter();
#endif
	if ((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		HAL_UART_Receive(&huart2, &Res, 1, 1);
		USART_RX_BUF_2[USART_RX_STA_2] = Res ;
		USART_RX_STA_2++;
		if (USART_RX_STA_2 > (USART_REC_LEN - 1))USART_RX_STA_2 = 0;


	}
	HAL_UART_IRQHandler(&huart2);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntExit();
#endif
}

void USART6_IRQHandler(void)
{
	//TODO:判断接收前两帧0x00,0x55,依次接收ID,数据和校验和;
	u8 Res;
#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntEnter();
#endif
	if ((__HAL_UART_GET_FLAG(&huart6, UART_IT_LBD) != RESET)) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		HAL_UART_Receive(&huart6, &Res, 1, 1);
		USART_RX_BUF_6[USART_RX_STA_6] = Res ;
		USART_RX_STA_6++;
		if (USART_RX_STA_6 > (USART_REC_LEN - 1))USART_RX_STA_6 = 0;


	}
	HAL_UART_IRQHandler(&huart6);
	__HAL_UART_ENABLE_IT(&huart6, UART_IT_LBD);
#if SYSTEM_SUPPORT_OS	 	//使用OS
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
	HAL_UART_Transmit(&huart1, Str, len, 1000); //发送接收到的数据
	while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET);   //等待发送结束
	// for (int i = 0; i < len; i++)
	// {
	// 	HAL_UART_Transmit(&huart1,&Str[i], 1, 1000); //发送接收到的数据
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
// 		USART_ClearITPendingBit(LIN_CHANNEL, USART_IT_LBD); // 检测到同步间隔场
// 		//LinStatusSet(SYNCH);
// 	}
// 	if (USART_GetFlagStatus(LIN_CHANNEL, USART_FLAG_ORE) == SET) // 溢出
// 	{
// 		USART_ClearFlag(LIN_CHANNEL, USART_FLAG_ORE);
// 		USART_ReceiveData(LIN_CHANNEL);
// 	}
// }



// void USART1_IRQHandler(void)
// {
// #if SYSTEM_SUPPORT_OS	 	//使用OS
// 	OSIntEnter();
// #endif
// 	if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET))
// 	{
// 		unsigned short tempRx_Head = (Rx_Head + 1) % UsartRxBuffSize;
// 		unsigned short tempRx_Tail = Rx_Tail;
// 		uint8_t data;			//读取数据 注意：这句必须要，否则不能够清除中断标志位
// 		HAL_UART_Receive(&huart1, &data, 1,1);
// 		if (tempRx_Head == tempRx_Tail)
// 		{
// 			__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE); //接受缓存满了，停止接收数据
// 		}
// 		else {
// 			Rx[Rx_Head] = data;
// 			Rx_Head = tempRx_Head;
// 		}
// 	}
// 	HAL_UART_IRQHandler(&huart1);
// 	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
// #if SYSTEM_SUPPORT_OS	 	//使用OS
// 	OSIntExit();
// #endif
// }


// void USART1_IRQHandler(void)
// {
// 	u32 timeout = 0;
// 	u32 maxDelay = 0x1FFFF;
// #if SYSTEM_SUPPORT_OS	 	//使用OS
// 	OSIntEnter();
// #endif

// 	HAL_UART_IRQHandler(&huart1);	//调用HAL库中断处理公用函数
// 	__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_LBD);
// 	timeout = 0;
// 	while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY)//等待就绪
// 	{
// 		timeout++;////超时处理
// 		if (timeout > maxDelay) break;
// 	}

// 	timeout = 0;
// 	while (HAL_UART_Receive_IT(&huart1, (u8 *)aRxBuffer1, RXBUFFERSIZE) != HAL_OK) //一次处理完成之后，重新开启中断并设置RxXferCount为1
// 	{
// 		timeout++; //超时处理
// 		if (timeout > maxDelay) break;
// 	}

// #if SYSTEM_SUPPORT_OS	 	//使用OS
// 	OSIntExit();
// #endif
// }

// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
// if (huart->Instance == USART1) //如果是串口1
// {
// 	unsigned short tempRx_Head = (Rx_Head + 1) % UsartRxBuffSize;
// 	unsigned short tempRx_Tail = Rx_Tail;
// 	uint8_t data = aRxBuffer1[0] ;

// 	if (tempRx_Head == tempRx_Tail)
// 	{
// 		__HAL_UART_DISABLE_IT(&huart1, UART_IT_LBD); //接受缓存满了，停止接收数据
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
// #if SYSTEM_SUPPORT_OS	 	//使用OS
// 	OSIntEnter();
// #endif
// 	if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_LBD) != RESET)) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
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
// #if SYSTEM_SUPPORT_OS	 	//使用OS
// 	OSIntExit();
// #endif
// }



