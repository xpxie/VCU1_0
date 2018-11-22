#include "rtc.h"
#include "delay.h"
#include "led.h"
#include "log.h"
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//RTC��������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/13
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

RTC_HandleTypeDef RTC_Handler;  //RTC���
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
//RTCʱ������
//hour,min,sec:Сʱ,����,����
//ampm:@RTC_AM_PM_Definitions:RTC_HOURFORMAT12_AM/RTC_HOURFORMAT12_PM
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ��
HAL_StatusTypeDef RTC_Set_Time(u8 hour, u8 min, u8 sec, u8 ampm)
{
    RTC_TimeTypeDef RTC_TimeStructure;

    RTC_TimeStructure.Hours = hour;
    RTC_TimeStructure.Minutes = min;
    RTC_TimeStructure.Seconds = sec;
    RTC_TimeStructure.TimeFormat = ampm;
    RTC_TimeStructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    RTC_TimeStructure.StoreOperation = RTC_STOREOPERATION_RESET;
    return HAL_RTC_SetTime(&RTC_Handler, &RTC_TimeStructure, RTC_FORMAT_BIN);
}

//RTC��������
//year,month,date:��(0~99),��(1~12),��(0~31)
//week:����(1~7,0,�Ƿ�!)
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ��
HAL_StatusTypeDef RTC_Set_Date(u8 year, u8 month, u8 date, u8 week)
{
    RTC_DateTypeDef RTC_DateStructure;

    RTC_DateStructure.Date = date;
    RTC_DateStructure.Month = month;
    RTC_DateStructure.WeekDay = week;
    RTC_DateStructure.Year = year;
    return HAL_RTC_SetDate(&RTC_Handler, &RTC_DateStructure, RTC_FORMAT_BIN);
}

//RTC��ʼ��
//����ֵ:0,��ʼ���ɹ�;
//       2,�����ʼ��ģʽʧ��;
u8 RTC_Init(void)
{


    RTC_Handler.Instance = RTC;
    RTC_Handler.Init.HourFormat = RTC_HOURFORMAT_24; //RTC����Ϊ24Сʱ��ʽ
    RTC_Handler.Init.AsynchPrediv = 0X7C;         //RTC�첽��Ƶϵ��(1~0X7F)   1s=25M/25/(7C+1)/(1f3f+1)
    RTC_Handler.Init.SynchPrediv = 0X1F3F;          //RTCͬ����Ƶϵ��(0~7FFF)
    RTC_Handler.Init.OutPut = RTC_OUTPUT_DISABLE;
    RTC_Handler.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    RTC_Handler.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&RTC_Handler) != HAL_OK) return 2;

    // if(HAL_RTCEx_BKUPRead(&RTC_Handler,RTC_BKP_DR0)!=0X5050)//�Ƿ��һ������
    // {
    // RTC_Set_Time(23,59,56,RTC_HOURFORMAT12_PM);         //����ʱ�� ,����ʵ��ʱ���޸�
    // RTC_Set_Date(15,12,27,7);                           //��������
    //     HAL_RTCEx_BKUPWrite(&RTC_Handler,RTC_BKP_DR0,0X5050);//����Ѿ���ʼ������
    // }
    return 0;
}

//RTC�ײ�������ʱ������
//�˺����ᱻHAL_RTC_Init()����
//hrtc:RTC���
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
    __HAL_RCC_PWR_CLK_ENABLE();  //ʹ��PWRʱ��
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_HSE_DIV25);
    __HAL_RCC_RTC_ENABLE();
}

//��������ʱ��(����������,24Сʱ��)
//week:���ڼ�(1~7) @ref  RTC_WeekDay_Definitions
//hour,min,sec:Сʱ,����,����
void RTC_Set_AlarmA(u8 week, u8 hour, u8 min, u8 sec)
{
    RTC_AlarmTypeDef RTC_AlarmSturuct;

    RTC_AlarmSturuct.AlarmTime.Hours = hour; //Сʱ
    RTC_AlarmSturuct.AlarmTime.Minutes = min; //����
    RTC_AlarmSturuct.AlarmTime.Seconds = sec; //��
    RTC_AlarmSturuct.AlarmTime.SubSeconds = 0;
    RTC_AlarmSturuct.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;

    RTC_AlarmSturuct.AlarmMask = RTC_ALARMMASK_NONE; //��ȷƥ�����ڣ�ʱ����
    RTC_AlarmSturuct.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
    RTC_AlarmSturuct.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY; //������
    RTC_AlarmSturuct.AlarmDateWeekDay = week; //����
    RTC_AlarmSturuct.Alarm = RTC_ALARM_A;   //����A
    HAL_RTC_SetAlarm_IT(&RTC_Handler, &RTC_AlarmSturuct, RTC_FORMAT_BIN);

    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0x01, 0x02); //��ռ���ȼ�1,�����ȼ�2
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

//�����Ի��Ѷ�ʱ������
/*wksel:  @ref RTCEx_Wakeup_Timer_Definitions
#define RTC_WAKEUPCLOCK_RTCCLK_DIV16        ((uint32_t)0x00000000)
#define RTC_WAKEUPCLOCK_RTCCLK_DIV8         ((uint32_t)0x00000001)
#define RTC_WAKEUPCLOCK_RTCCLK_DIV4         ((uint32_t)0x00000002)
#define RTC_WAKEUPCLOCK_RTCCLK_DIV2         ((uint32_t)0x00000003)
#define RTC_WAKEUPCLOCK_CK_SPRE_16BITS      ((uint32_t)0x00000004)
#define RTC_WAKEUPCLOCK_CK_SPRE_17BITS      ((uint32_t)0x00000006)
*/
//cnt:�Զ���װ��ֵ.����0,�����ж�.
void RTC_Set_WakeUp(u32 wksel, u16 cnt)
{
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RTC_Handler, RTC_FLAG_WUTF);//���RTC WAKE UP�ı�־

    HAL_RTCEx_SetWakeUpTimer_IT(&RTC_Handler, cnt, wksel);          //������װ��ֵ��ʱ��

    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0x01, 0x02); //��ռ���ȼ�1,�����ȼ�2
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
}

//RTC�����жϷ�����
void RTC_Alarm_IRQHandler(void)
{
    HAL_RTC_AlarmIRQHandler(&RTC_Handler);
}

//RTC����A�жϴ���ص�����
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    printf("ALARM A!\r\n");
}

//RTC WAKE UP�жϷ�����
void RTC_WKUP_IRQHandler(void)
{
    HAL_RTCEx_WakeUpTimerIRQHandler(&RTC_Handler);
}

//RTC WAKE UP�жϴ���
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    LED0 = !LED0;
    if (HAL_RTC_GetState(hrtc) == HAL_RTC_STATE_READY && RTC_DateStruct.Year != 0xFF)
    {
        memset(new_log_file_name, 0, 18);
        sprintf((char*)new_log_file_name, "20%02d%02d%02d%02d%02d%02d.log", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date,
                RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
        new_file_setted_flag = 1;
        printf("generate file:");
        printf("%s\r\n", new_log_file_name);
    }
}

