#include "spi.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F429开发板
//SPI驱动代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/16/16
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

SPI_HandleTypeDef SPI5_Handler;  //SPI句柄
DMA_HandleTypeDef  SPI5TxDMA_Handler;      //DMA句柄
DMA_HandleTypeDef  SPI5RxDMA_Handler;

//以下是SPI模块的初始化代码，配置成主机模式
//SPI口初始化
//这里针是对SPI5的初始化
void SPI5_Init(void)
{
    SPI5_Handler.Instance = SPI5;                       //SP5
    SPI5_Handler.Init.Mode = SPI_MODE_MASTER;           //设置SPI工作模式，设置为主模式
    SPI5_Handler.Init.Direction = SPI_DIRECTION_2LINES; //设置SPI单向或者双向的数据模式:SPI设置为双线模式
    SPI5_Handler.Init.DataSize = SPI_DATASIZE_8BIT;     //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI5_Handler.Init.CLKPolarity = SPI_POLARITY_HIGH;  //串行同步时钟的空闲状态为高电平
    SPI5_Handler.Init.CLKPhase = SPI_PHASE_2EDGE;       //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    SPI5_Handler.Init.NSS = SPI_NSS_SOFT;               //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI5_Handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; //定义波特率预分频的值:波特率预分频值为4
    SPI5_Handler.Init.FirstBit = SPI_FIRSTBIT_MSB;      //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI5_Handler.Init.TIMode = SPI_TIMODE_DISABLE;      //关闭TI模式
    SPI5_Handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; //关闭硬件CRC校验
    SPI5_Handler.Init.CRCPolynomial = 7;                //CRC值计算的多项式
    HAL_SPI_Init(&SPI5_Handler);//初始化

    __HAL_SPI_ENABLE(&SPI5_Handler);                    //使能SPI5

    // SPI5_ReadWriteByte(0Xff);                           //启动传输
}

//SPI5底层驱动，时钟使能，引脚配置
//此函数会被HAL_SPI_Init()调用
//hspi:SPI句柄
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOF_CLK_ENABLE();       //使能GPIOF时钟
    __HAL_RCC_SPI5_CLK_ENABLE();        //使能SPI5时钟

    //PF7,8,9
    GPIO_Initure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;            //复用推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;                //上拉
    GPIO_Initure.Speed = GPIO_SPEED_FAST;           //快速
    GPIO_Initure.Alternate = GPIO_AF5_SPI5;         //复用为SPI5
    HAL_GPIO_Init(GPIOF, &GPIO_Initure);
}

//SPI速度设置函数
//SPI速度=fAPB1/分频系数
//@ref SPI_BaudRate_Prescaler:SPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
//fAPB1时钟一般为45Mhz：
void SPI5_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    __HAL_SPI_DISABLE(&SPI5_Handler);            //关闭SPI
    SPI5_Handler.Instance->CR1 &= 0XFFC7;        //位3-5清零，用来设置波特率
    SPI5_Handler.Instance->CR1 |= SPI_BaudRatePrescaler; //设置SPI速度
    __HAL_SPI_ENABLE(&SPI5_Handler);             //使能SPI

}

//SPI5 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
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
    return Rxdata;                      //返回收到的数据
}


//DMAx的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMA通道选择,@ref DMA_channel DMA_CHANNEL_0~DMA_CHANNEL_7
void W25Q_SPI_DMA_Config(void)
{
    u8 res = HAL_ERROR;

    __HAL_RCC_DMA2_CLK_ENABLE();//DMA2时钟使能

    //Tx DMA配置
    SPI5TxDMA_Handler.Instance = DMA2_Stream4;                          //数据流选择
    SPI5TxDMA_Handler.Init.Channel = DMA_CHANNEL_2;                              //通道选择
    SPI5TxDMA_Handler.Init.Direction = DMA_MEMORY_TO_PERIPH;           //存储器到外设
    SPI5TxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //外设非增量模式
    SPI5TxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //存储器增量模式
    SPI5TxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //外设数据长度:8位
    SPI5TxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //存储器数据长度:8位
    SPI5TxDMA_Handler.Init.Mode = DMA_NORMAL;                          //外设普通模式
    SPI5TxDMA_Handler.Init.Priority = DMA_PRIORITY_MEDIUM;             //中等优先级
    SPI5TxDMA_Handler.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    SPI5TxDMA_Handler.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    SPI5TxDMA_Handler.Init.MemBurst = DMA_MBURST_SINGLE;               //存储器突发单次传输
    SPI5TxDMA_Handler.Init.PeriphBurst = DMA_PBURST_SINGLE;            //外设突发单次传输

    HAL_DMA_DeInit(&SPI5TxDMA_Handler);
    res = HAL_DMA_Init(&SPI5TxDMA_Handler);
    __HAL_LINKDMA(&SPI5_Handler, hdmatx, SPI5TxDMA_Handler);  //将DMA与USART1联系起来(发送DMA)


    //Tx DMA配置
    SPI5RxDMA_Handler.Instance = DMA2_Stream3;                          //数据流选择
    SPI5RxDMA_Handler.Init.Channel = DMA_CHANNEL_2;                              //通道选择
    SPI5RxDMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;           //外设到存储器
    SPI5RxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //外设非增量模式
    SPI5RxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //存储器增量模式
    SPI5RxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //外设数据长度:8位
    SPI5RxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //存储器数据长度:8位
    SPI5RxDMA_Handler.Init.Mode = DMA_NORMAL;                          //外设普通模式
    SPI5RxDMA_Handler.Init.Priority = DMA_PRIORITY_MEDIUM;             //中等优先级
    SPI5RxDMA_Handler.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    SPI5RxDMA_Handler.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    SPI5RxDMA_Handler.Init.MemBurst = DMA_MBURST_SINGLE;               //存储器突发单次传输
    SPI5RxDMA_Handler.Init.PeriphBurst = DMA_PBURST_SINGLE;            //外设突发单次传输

    HAL_DMA_DeInit(&SPI5RxDMA_Handler);
    res = HAL_DMA_Init(&SPI5RxDMA_Handler);
    __HAL_LINKDMA(&SPI5_Handler, hdmarx, SPI5RxDMA_Handler);  //将DMA与USART1联系起来(发送DMA)

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

// //开启一次DMA传输
// //huart:串口句柄
// //pData：传输的数据指针
// //Size:传输的数据量
// void MYDMA_USART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
// {
//     HAL_DMA_Start(huart->hdmatx, (u32)pData, (uint32_t)&huart->Instance->DR, Size);//开启DMA传输

//     huart->Instance->CR3 |= USART_CR3_DMAT;//使能串口DMA发送
// }
