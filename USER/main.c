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


//任务优先级
#define START_TASK_PRIO     3
//任务堆栈大小
#define START_STK_SIZE      512
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define ENCODER_TASK_PRIO       4
//任务堆栈大小
#define ENCODER_STK_SIZE        256
//任务控制块
OS_TCB EncoderTaskTCB;
//任务堆栈
CPU_STK ENCODER_TASK_STK[ENCODER_STK_SIZE];
void encoder_task(void *p_arg);

//任务优先级
#define LED_TASK_PRIO       5
//任务堆栈大小
#define LED_STK_SIZE       128
//任务控制块
OS_TCB LedTaskTCB;
//任务堆栈
CPU_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
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
  Stm32_Clock_Init(360, 25, 2, 8); //设置时钟,180Mhz
  HAL_Init();                     //初始化HAL库
  delay_init(180);                //初始化延时函数
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
  Get_SerialNum();                //Get chip_id
  MX_USART2_UART_Init();              //初始化USART
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();             //初始化USART


  LED_Init();                   //初始化LED



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


  my_mem_init(SRAMIN);            //初始化内部内存池
  // my_mem_init(SRAMCCM);           //初始化CCM内存池

  exfuns_init();                  //为fatfs相关变量申请内存

  delay_ms(1000);

  result = f_mount(fs[0], "0:", 1);              //挂载FLASH.
  if (result == 0X0D) //FLASH磁盘,FAT文件系统错误,重新格式化FLASH
  {
    result = f_mkfs("0:", 1, 4096); //格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
    f_setlabel((const TCHAR *)"0:UnityDrive");  //设置Flash磁盘的名字为：UnityDrive
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


  OSInit(&err);       //初始化UCOSIII
  OS_CRITICAL_ENTER();//进入临界区


  // //创建开始任务
  OSTaskCreate((OS_TCB    * )&StartTaskTCB,       //任务控制块
               (CPU_CHAR  * )"start task",        //任务名字
               (OS_TASK_PTR )start_task,          //任务函数
               (void      * )0,                   //传递给任务函数的参数
               (OS_PRIO     )START_TASK_PRIO,     //任务优先级
               (CPU_STK   * )&START_TASK_STK[0],  //任务堆栈基地址
               (CPU_STK_SIZE)START_STK_SIZE / 10, //任务堆栈深度限位
               (CPU_STK_SIZE)START_STK_SIZE,      //任务堆栈大小
               (OS_MSG_QTY  )0,                   //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
               (OS_TICK     )0,                   //当使能时间片轮转时的时间片长度，为0时为默认长度，
               (void      * )0,                   //用户补充的存储区
               (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP, //任务选项,为了保险起见，所有任务都保存浮点寄存器的值
               (OS_ERR    * )&err);               //存放该函数错误时的返回值
  OS_CRITICAL_EXIT(); //退出临界区
  OSStart(&err);      //开启UCOSIII


  while (1)
  {
  }
}

//开始任务函数
void start_task(void *p_arg)
{
  OS_ERR err;
  CPU_SR_ALLOC();
  p_arg = p_arg;

  CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
  OSStatTaskCPUUsageInit(&err);    //统计任务
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN      //如果使能了测量中断关闭时间
  CPU_IntDisMeasMaxCurReset();
#endif

#if OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
  //使能时间片轮转调度功能,设置默认的时间片长度s
  OSSchedRoundRobinCfg(DEF_ENABLED, 5, &err);
#endif

  OS_CRITICAL_ENTER();    //进入临界区
  //创建ENCODER任务
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

  //创建LED任务
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


  OS_CRITICAL_EXIT(); //进入临界区
  OS_TaskSuspend((OS_TCB*)&StartTaskTCB, &err);       //挂起开始任务
}



//led任务函数
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
    IWDG_Feed();        //Feed watch dog，at least 1s
    HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);
    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err); //延时1s

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
    else if (new_file_setted_flag == 1 )  //flag=1表示新文件生成
    {
      new_file_setted_flag = 2;
      mf_unlink((u8 *)new_log_file_name); //删除原本同名文件
    }
    else if (new_file_setted_flag == 2)   //flag=2，允许写入新文件
    {
      ensure_free_space("0:");
      Save_Log_Data_To_Flash();
      // open_append(file, new_log_file_name);
      // mf_write(cnt, 4096);
      // mf_write(cnt2, 4096);
      // f_close(file);
    }

// Usart1Send(temp_buf, sizeof(temp_buf));



// HAL_UART_Transmit(&huart1, (uint8_t*)cnt, 8, 1000); //发送接收到的数据
// while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET);   //等待发送结束


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
