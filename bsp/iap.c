#include "sys.h"
#include "delay.h"
#include "stmflash.h"
#include "iap.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//IAP ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/18
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

iapfun jump2app; 
u32 iapbuf[512]; 	//2K�ֽڻ���  
//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u32 t;
	u16 i=0;
	u32 temp;
	u32 fwaddr=appxaddr;//��ǰд��ĵ�ַ
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=4)
	{						   
		temp=(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu+=4;//ƫ��4���ֽ�
		iapbuf[i++]=temp;	    
		if(i==512)
		{
			i=0; 
			STMFLASH_Write(fwaddr,iapbuf,512);
			fwaddr+=2048;//ƫ��2048  512*4=2048
		}
	} 
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//������һЩ�����ֽ�д��ȥ.  
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 app_address)
{ 
	typedef void (*_func)(void);

    __disable_irq();

    /* MCU peripherals re-initial. */
    {
        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;

        GPIO_InitStruct.Pin = GPIO_PIN_All;
        GPIO_InitStruct.Pin &= ~(GPIO_PIN_13 | GPIO_PIN_14); /* SWDIO/SWCLK */
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_All;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

        /* reset systick */
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

        /* disable all peripherals clock. */
        RCC->AHB1ENR = (1<<20); /* 20: F4 CCMDAT ARAMEN. */
        RCC->AHB2ENR = 0;
        RCC->AHB3ENR = 0;
        RCC->APB1ENR = 0;
        RCC->APB2ENR = 0;

        /* Switch to default cpu clock. */
        RCC->CFGR = 0;
    } /* MCU peripherals re-initial. */

    /* Disable MPU */
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

    /* disable and clean up all interrupts. */
    {
        int i;

        for(i = 0; i < 8; i++)
        {
            /* disable interrupts. */
            NVIC->ICER[i] = 0xFFFFFFFF;

            /* clean up interrupts flags. */
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }
    }

    /* Set new vector table pointer */
    SCB->VTOR = app_address;

    /* reset register values */
    __set_BASEPRI(0);
    __set_FAULTMASK(0);

    /* set up MSP and switch to it */
    __set_MSP(*(uint32_t*)app_address);
    __set_PSP(*(uint32_t*)app_address);
    __set_CONTROL(0);

    /* ensure what we have done could take effect */
    __ISB();

    __disable_irq();

    /* never return */
    ((_func)(*(uint32_t*)(app_address + 4)))();
}		 














