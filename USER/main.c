#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "includes.h"
#include "gpio.h"
#include "led.h"
#include "bsp_encoder.h"
#include "bsp_can.h"
#include "w25qxx.h"
#include "malloc.h"
#include "ff.h"
#include "exfuns.h"
#include "fattester.h"
#include "string.h"
#include "rtc.h"
#include "log.h"
#include "iwdg.h"


//�������ȼ�
#define START_TASK_PRIO     3
//�����ջ��С
#define START_STK_SIZE      512
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

//�������ȼ�
#define ENCODER_TASK_PRIO       4
//�����ջ��С
#define ENCODER_STK_SIZE        256
//������ƿ�
OS_TCB EncoderTaskTCB;
//�����ջ
CPU_STK ENCODER_TASK_STK[ENCODER_STK_SIZE];
void encoder_task(void *p_arg);

//�������ȼ�
#define LED_TASK_PRIO       5
//�����ջ��С
#define LED_STK_SIZE       128
//������ƿ�
OS_TCB LedTaskTCB;
//�����ջ
CPU_STK LED_TASK_STK[LED_STK_SIZE];
//������
void led_task(void *p_arg);


//static int encoder_count = 0;
uint8_t cnt[8]={1,2,3,4,5,6,7,8};
uint8_t cnt2[4096];
u8 count[6] = {0, 0, 0, 0, 0, 0};
u8 read_buffer[100];
u8 result = 0;
uint8_t lin2_send_id = 0x26;
u8 tbuf[40];
int main(void)
{
  //SCB->VTOR = FLASH_BASE | 0x10000;
  OS_ERR err;
  CPU_SR_ALLOC();
  INTX_ENABLE();
  Stm32_Clock_Init(360, 25, 2, 8); //����ʱ��,180Mhz
  HAL_Init();                     //��ʼ��HAL��
  delay_init(180);                //��ʼ����ʱ����
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
  Get_SerialNum();                //Get chip_id
  MX_USART2_UART_Init();              //��ʼ��USART
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();             //��ʼ��USART


  LED_Init();                   //��ʼ��LED



  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  can_init(&hcan1);
  can_init(&hcan2);

  W25QXX_Init();


  my_mem_init(SRAMIN);            //��ʼ���ڲ��ڴ��
  // my_mem_init(SRAMCCM);           //��ʼ��CCM�ڴ��

  exfuns_init();                  //Ϊfatfs��ر��������ڴ�

  delay_ms(1000);

  result = f_mount(fs[0], "0:", 1);              //����FLASH.
  if (result == 0X0D) //FLASH����,FAT�ļ�ϵͳ����,���¸�ʽ��FLASH
  {
    result = f_mkfs("0:", 1, 4096); //��ʽ��FLASH,1,�̷�;1,����Ҫ������,8������Ϊ1����
    f_setlabel((const TCHAR *)"0:UnityDrive");  //����Flash���̵�����Ϊ��UnityDrive
    delay_ms(1000);
    printf("reinitialize the disk completed!");

  }

  // // f_deldir("0:");
  // // remove_files("0:");

  mf_scan_files("0:");
  mf_showfree("0:UnityDrive");

  //rm_not_bin("0:");
  mf_scan_files("0:");
  mf_showfree("0:UnityDrive");


  // open_append(file, "00000000000000.log");
  // mf_write(cnt, 4096);
  // f_close(file);

//   f_mkdir("log_file");
//    f_mkdir("bin_file");
//    mf_opendir("log_file");


// mf_unlink("20181019163933.log");
// mf_unlink("20181019163943.log");
// mf_unlink("20181019163953.log");
// mf_unlink("20181019164013.log");
// mf_unlink("20181019164023.log");
// mf_unlink("20181019164033.log");
// mf_unlink("20181019164033.log");


  OSInit(&err);       //��ʼ��UCOSIII
  OS_CRITICAL_ENTER();//�����ٽ���


  // //������ʼ����
  OSTaskCreate((OS_TCB    * )&StartTaskTCB,       //������ƿ�
               (CPU_CHAR  * )"start task",        //��������
               (OS_TASK_PTR )start_task,          //������
               (void      * )0,                   //���ݸ��������Ĳ���
               (OS_PRIO     )START_TASK_PRIO,     //�������ȼ�
               (CPU_STK   * )&START_TASK_STK[0],  //�����ջ����ַ
               (CPU_STK_SIZE)START_STK_SIZE / 10, //�����ջ�����λ
               (CPU_STK_SIZE)START_STK_SIZE,      //�����ջ��С
               (OS_MSG_QTY  )0,                   //�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
               (OS_TICK     )0,                   //��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
               (void      * )0,                   //�û�����Ĵ洢��
               (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP, //����ѡ��,Ϊ�˱���������������񶼱��渡��Ĵ�����ֵ
               (OS_ERR    * )&err);               //��Ÿú�������ʱ�ķ���ֵ
  OS_CRITICAL_EXIT(); //�˳��ٽ���
  OSStart(&err);      //����UCOSIII


  while (1)
  {
  }
}

//��ʼ������
void start_task(void *p_arg)
{
  OS_ERR err;
  CPU_SR_ALLOC();
  p_arg = p_arg;

  CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
  OSStatTaskCPUUsageInit(&err);    //ͳ������
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN      //���ʹ���˲����жϹر�ʱ��
  CPU_IntDisMeasMaxCurReset();
#endif

#if OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
  //ʹ��ʱ��Ƭ��ת���ȹ���,����Ĭ�ϵ�ʱ��Ƭ����s
  OSSchedRoundRobinCfg(DEF_ENABLED, 5, &err);
#endif

  OS_CRITICAL_ENTER();    //�����ٽ���
  //����ENCODER����
  OSTaskCreate((OS_TCB    * )&EncoderTaskTCB,
               (CPU_CHAR  * )"encoder task",
               (OS_TASK_PTR )encoder_task,
               (void      * )0,
               (OS_PRIO     )ENCODER_TASK_PRIO,
               (CPU_STK   * )&ENCODER_TASK_STK[0],
               (CPU_STK_SIZE)ENCODER_STK_SIZE / 10,
               (CPU_STK_SIZE)ENCODER_STK_SIZE,
               (OS_MSG_QTY  )0,
               (OS_TICK     )0,
               (void      * )0,
               (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP,
               (OS_ERR    * )&err);

  //����LED����
  OSTaskCreate((OS_TCB    * )&LedTaskTCB,
               (CPU_CHAR  * )"led task",
               (OS_TASK_PTR )led_task,
               (void      * )0,
               (OS_PRIO     )LED_TASK_PRIO,
               (CPU_STK   * )&LED_TASK_STK[0],
               (CPU_STK_SIZE)LED_STK_SIZE / 10,
               (CPU_STK_SIZE)LED_STK_SIZE,
               (OS_MSG_QTY  )0,
               (OS_TICK     )0,
               (void      * )0,
               (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP,
               (OS_ERR    * )&err);


  OS_CRITICAL_EXIT(); //�����ٽ���
  OS_TaskSuspend((OS_TCB*)&StartTaskTCB, &err);       //����ʼ����
}



//led������
void led_task(void *p_arg)
{

  OS_ERR err;
  static u8 led_count = 0;
  IWDG_Init(IWDG_PRESCALER_64,500);
  while (1)
  {

     update_light_state();
    led_count ++;
    if (led_count >= 100)
    {
      LED = ~LED;
      led_count = 0;
    }
    IWDG_Feed();        //Feed watch dog��at least 1s
    HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);
    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err); //��ʱ1s

  }
}


void encoder_task(void *p_arg)
{
  OS_ERR err;
  // // byte temp_buf[4] = {0x55, 0x09, 0x1f, calcChecksum(0x0A)};
  // for (int i = 0; i < 4096; i++)
  // {
  //   cnt[i] = i % 10;
  //   cnt2[i] = i % 10 + 100;
  // }
  while (1)
  {
    // static u16 space_count=0;
    if (read_log_file_flag == 1)
    {
      f_open(file, read_log_file_name, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
      mf_read(f_size(file));
      f_close(file);
      read_log_file_flag = 0;
    }
    else if (new_file_setted_flag == 1 )  //flag=1��ʾ���ļ�����
    {
      new_file_setted_flag = 2;
      mf_unlink((u8 *)new_log_file_name); //ɾ��ԭ��ͬ���ļ�
    }
    else if (new_file_setted_flag == 2)   //flag=2������д�����ļ�
    {
      ensure_free_space("0:");
      Save_Log_Data_To_Flash();
      // open_append(file, new_log_file_name);
      // mf_write(cnt, 4096);
      // mf_write(cnt2, 4096);
      // f_close(file);
    }

// Usart1Send(temp_buf, sizeof(temp_buf));



// HAL_UART_Transmit(&huart1, (uint8_t*)cnt, 8, 1000); //���ͽ��յ�������
// while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET);   //�ȴ����ͽ���


// LIN2_Send_Data(lin2_send_id,cnt);

 //can_send_data(&hcan2,CAN_SEND_LOG_MSG_ID, cnt, 8);
// W25QXX_Write(cnt, count * 8, 8);


// BSP_EncoderRead();


// W25QXX_Read(read_buffer, count * 8, 7);

//        if ( !PEin(0))    count[0]++;
//        if ( !PEin(1))    count[1]++;
//        if ( !PIin(4))    count[2]++;
//        if ( !PIin(5))    count[3]++;
//        if ( !PIin(6))    count[4]++;
//        if ( !PIin(7))    count[5]++;

         can_send_current_state();
    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);


  }
}
