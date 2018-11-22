#include "spi.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//SPI��������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/16/16
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

SPI_HandleTypeDef SPI5_Handler;  //SPI���
DMA_HandleTypeDef  SPI5TxDMA_Handler;      //DMA���
DMA_HandleTypeDef  SPI5RxDMA_Handler;

//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ
//SPI�ڳ�ʼ��
//�������Ƕ�SPI5�ĳ�ʼ��
void SPI5_Init(void)
{
    SPI5_Handler.Instance = SPI5;                       //SP5
    SPI5_Handler.Init.Mode = SPI_MODE_MASTER;           //����SPI����ģʽ������Ϊ��ģʽ
    SPI5_Handler.Init.Direction = SPI_DIRECTION_2LINES; //����SPI�������˫�������ģʽ:SPI����Ϊ˫��ģʽ
    SPI5_Handler.Init.DataSize = SPI_DATASIZE_8BIT;     //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI5_Handler.Init.CLKPolarity = SPI_POLARITY_HIGH;  //����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
    SPI5_Handler.Init.CLKPhase = SPI_PHASE_2EDGE;       //����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
    SPI5_Handler.Init.NSS = SPI_NSS_SOFT;               //NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI5_Handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; //���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ4
    SPI5_Handler.Init.FirstBit = SPI_FIRSTBIT_MSB;      //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI5_Handler.Init.TIMode = SPI_TIMODE_DISABLE;      //�ر�TIģʽ
    SPI5_Handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; //�ر�Ӳ��CRCУ��
    SPI5_Handler.Init.CRCPolynomial = 7;                //CRCֵ����Ķ���ʽ
    HAL_SPI_Init(&SPI5_Handler);//��ʼ��

    __HAL_SPI_ENABLE(&SPI5_Handler);                    //ʹ��SPI5

    // SPI5_ReadWriteByte(0Xff);                           //��������
}

//SPI5�ײ�������ʱ��ʹ�ܣ���������
//�˺����ᱻHAL_SPI_Init()����
//hspi:SPI���
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOF_CLK_ENABLE();       //ʹ��GPIOFʱ��
    __HAL_RCC_SPI5_CLK_ENABLE();        //ʹ��SPI5ʱ��

    //PF7,8,9
    GPIO_Initure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;            //�����������
    GPIO_Initure.Pull = GPIO_PULLUP;                //����
    GPIO_Initure.Speed = GPIO_SPEED_FAST;           //����
    GPIO_Initure.Alternate = GPIO_AF5_SPI5;         //����ΪSPI5
    HAL_GPIO_Init(GPIOF, &GPIO_Initure);
}

//SPI�ٶ����ú���
//SPI�ٶ�=fAPB1/��Ƶϵ��
//@ref SPI_BaudRate_Prescaler:SPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
//fAPB1ʱ��һ��Ϊ45Mhz��
void SPI5_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//�ж���Ч��
    __HAL_SPI_DISABLE(&SPI5_Handler);            //�ر�SPI
    SPI5_Handler.Instance->CR1 &= 0XFFC7;        //λ3-5���㣬�������ò�����
    SPI5_Handler.Instance->CR1 |= SPI_BaudRatePrescaler; //����SPI�ٶ�
    __HAL_SPI_ENABLE(&SPI5_Handler);             //ʹ��SPI

}

//SPI5 ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI5_ReadWriteByte(u8 TxData)
{
    u8 Rxdata;
    u8 ret;
    // HAL_SPI_TransmitReceive(&SPI5_Handler, &TxData, &Rxdata, 1, 1000);
    // HAL_SPI_TransmitReceive_IT(&SPI5_Handler,&TxData,&Rxdata,1);
    // HAL_DMA_Start(SPI5_Handler.hdmarx, (uint32_t)&SPI5_Handler.Instance->DR, (uint32_t)&Rxdata, 1);
    // SPI5_Handler.Instance->CR2 |= SPI_CR2_RXDMAEN;
    // HAL_DMA_Start(SPI5_Handler.hdmatx, (uint32_t)&TxData, (uint32_t)&SPI5_Handler.Instance->DR, 1);
    // SPI5_Handler.Instance->CR2 |= SPI_CR2_TXDMAEN;

    ret = HAL_SPI_TransmitReceive_DMA(&SPI5_Handler, &TxData, &Rxdata, 1);
    while (__HAL_SPI_GET_FLAG(&SPI5_Handler, SPI_FLAG_TXE) != SET)
    {
        ;
    }
    return Rxdata;                      //�����յ�������
}


//DMAx�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
//DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMAͨ��ѡ��,@ref DMA_channel DMA_CHANNEL_0~DMA_CHANNEL_7
void W25Q_SPI_DMA_Config(void)
{
    u8 res = HAL_ERROR;

    __HAL_RCC_DMA2_CLK_ENABLE();//DMA2ʱ��ʹ��

    //Tx DMA����
    SPI5TxDMA_Handler.Instance = DMA2_Stream4;                          //������ѡ��
    SPI5TxDMA_Handler.Init.Channel = DMA_CHANNEL_2;                              //ͨ��ѡ��
    SPI5TxDMA_Handler.Init.Direction = DMA_MEMORY_TO_PERIPH;           //�洢��������
    SPI5TxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //���������ģʽ
    SPI5TxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //�洢������ģʽ
    SPI5TxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //�������ݳ���:8λ
    SPI5TxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //�洢�����ݳ���:8λ
    SPI5TxDMA_Handler.Init.Mode = DMA_NORMAL;                          //������ͨģʽ
    SPI5TxDMA_Handler.Init.Priority = DMA_PRIORITY_MEDIUM;             //�е����ȼ�
    SPI5TxDMA_Handler.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    SPI5TxDMA_Handler.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    SPI5TxDMA_Handler.Init.MemBurst = DMA_MBURST_SINGLE;               //�洢��ͻ�����δ���
    SPI5TxDMA_Handler.Init.PeriphBurst = DMA_PBURST_SINGLE;            //����ͻ�����δ���

    HAL_DMA_DeInit(&SPI5TxDMA_Handler);
    res = HAL_DMA_Init(&SPI5TxDMA_Handler);
    __HAL_LINKDMA(&SPI5_Handler, hdmatx, SPI5TxDMA_Handler);  //��DMA��USART1��ϵ����(����DMA)


    //Tx DMA����
    SPI5RxDMA_Handler.Instance = DMA2_Stream3;                          //������ѡ��
    SPI5RxDMA_Handler.Init.Channel = DMA_CHANNEL_2;                              //ͨ��ѡ��
    SPI5RxDMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;           //���赽�洢��
    SPI5RxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //���������ģʽ
    SPI5RxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //�洢������ģʽ
    SPI5RxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //�������ݳ���:8λ
    SPI5RxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //�洢�����ݳ���:8λ
    SPI5RxDMA_Handler.Init.Mode = DMA_NORMAL;                          //������ͨģʽ
    SPI5RxDMA_Handler.Init.Priority = DMA_PRIORITY_MEDIUM;             //�е����ȼ�
    SPI5RxDMA_Handler.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    SPI5RxDMA_Handler.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    SPI5RxDMA_Handler.Init.MemBurst = DMA_MBURST_SINGLE;               //�洢��ͻ�����δ���
    SPI5RxDMA_Handler.Init.PeriphBurst = DMA_PBURST_SINGLE;            //����ͻ�����δ���

    HAL_DMA_DeInit(&SPI5RxDMA_Handler);
    res = HAL_DMA_Init(&SPI5RxDMA_Handler);
    __HAL_LINKDMA(&SPI5_Handler, hdmarx, SPI5RxDMA_Handler);  //��DMA��USART1��ϵ����(����DMA)

    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 1, 4);
    HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

}

void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&SPI5RxDMA_Handler);
}


void DMA2_Stream4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&SPI5TxDMA_Handler);
}

// //����һ��DMA����
// //huart:���ھ��
// //pData�����������ָ��
// //Size:�����������
// void MYDMA_USART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
// {
//     HAL_DMA_Start(huart->hdmatx, (u32)pData, (uint32_t)&huart->Instance->DR, Size);//����DMA����

//     huart->Instance->CR3 |= USART_CR3_DMAT;//ʹ�ܴ���DMA����
// }
