#include "bsp_can.h"
#include "bsp_encoder.h"
#include "gpio.h"
#include "w25qxx.h"
#include "iap.h"
#include "stmflash.h"
#include "sys.h"
#include "fattester.h"
#include "exfuns.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "crc_check.h"
#include "includes.h"
#include "log.h"
#include "rtc.h"


CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;
uint8_t size_can = sizeof(hcan1);

const uint8_t data_size = sizeof(hcan1.pRxMsg->StdId) + sizeof(hcan1.pRxMsg->Data);

uint32_t CAN_RX_CNT = 0;
u8  NEW_CODE_RX_BUF[CAN_REC_LEN] __attribute__ ((at(0X20005000)));
uint32_t new_code_len = 0;
// u8  W256Q_SAVE_BUF[CAN_REC_LEN] __attribute__ ((at(0X80D0000)));
u8 break_light_flag=1;
uint8_t read_data[data_size];
uint32_t receive_data_cnt = 0;
uint32_t read_data_cnt = 0;
uint32_t speed_cnt = 0;

static u32 Device_Serial[3];



void can_init(CAN_HandleTypeDef* _hcan)
{
  //can1 &can2 use same filter config
  CAN_FilterConfTypeDef  CAN_FilterConfigStructure;
  static CanTxMsgTypeDef Tx1Message;
  static CanRxMsgTypeDef Rx1Message;
  static CanTxMsgTypeDef Tx2Message;
  static CanRxMsgTypeDef Rx2Message;

  CAN_FilterConfigStructure.FilterNumber         = 0;
  CAN_FilterConfigStructure.FilterMode           = CAN_FILTERMODE_IDMASK;
  CAN_FilterConfigStructure.FilterScale          = CAN_FILTERSCALE_32BIT;
  CAN_FilterConfigStructure.FilterIdHigh         = 0x0000;
  CAN_FilterConfigStructure.FilterIdLow          = 0x0000;
  CAN_FilterConfigStructure.FilterMaskIdHigh     = 0x0000;
  CAN_FilterConfigStructure.FilterMaskIdLow      = 0x0000;
  CAN_FilterConfigStructure.FilterFIFOAssignment = CAN_FilterFIFO0;
  CAN_FilterConfigStructure.BankNumber           = 14; //can1(0-13)and can2(14-27)each get half filter
  CAN_FilterConfigStructure.FilterActivation     = ENABLE;

  if (HAL_CAN_ConfigFilter(_hcan, &CAN_FilterConfigStructure) != HAL_OK)
  {
    //err_deadloop();
  }

  //filter config for can2
  //can1(0-13),can2(14-27)
  CAN_FilterConfigStructure.FilterNumber = 14;
  if (HAL_CAN_ConfigFilter(_hcan, &CAN_FilterConfigStructure) != HAL_OK)
  {

  }

  if (_hcan == &hcan1)
  {
    _hcan->pTxMsg = &Tx1Message;
    _hcan->pRxMsg = &Rx1Message;
    HAL_CAN_Receive_IT(&hcan1, CAN_FIFO0);
  }

  if (_hcan == &hcan2)
  {
    _hcan->pTxMsg = &Tx2Message;
    _hcan->pRxMsg = &Rx2Message;
    HAL_CAN_Receive_IT(&hcan2, CAN_FIFO0);
  }
}

void MX_CAN1_Init(void)
{

  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 15;   //波特率=45M/15/(1+4+7)=250k
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SJW = CAN_SJW_1TQ;
  hcan1.Init.BS1 = CAN_BS1_4TQ;
  hcan1.Init.BS2 = CAN_BS2_7TQ;
  hcan1.Init.TTCM = DISABLE;
  hcan1.Init.ABOM = ENABLE;
  hcan1.Init.AWUM = DISABLE;
  hcan1.Init.NART = DISABLE;
  hcan1.Init.RFLM = DISABLE;
  hcan1.Init.TXFP = DISABLE;
  HAL_CAN_Init(&hcan1);

}

/* CAN2 init function */
void MX_CAN2_Init(void)
{

  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 15;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SJW = CAN_SJW_1TQ;
  hcan2.Init.BS1 = CAN_BS1_4TQ;
  hcan2.Init.BS2 = CAN_BS2_7TQ;
  hcan2.Init.TTCM = DISABLE;
  hcan2.Init.ABOM = ENABLE;
  hcan2.Init.AWUM = DISABLE;
  hcan2.Init.NART = DISABLE;
  hcan2.Init.RFLM = DISABLE;
  hcan2.Init.TXFP = DISABLE;
  HAL_CAN_Init(&hcan2);

}

void CAN1_RX0_IRQHandler(void)
{

  HAL_CAN_IRQHandler(&hcan1);
  Log_Can_Data(&hcan1);
  // speed_cnt++;
  switch (hcan1.pRxMsg->StdId)
  {
  case CAN_LIGHT_HORN_CONTROL_ID:
  {
    rece_light_state1 = hcan1.pRxMsg->Data[0];
    rece_light_state2 = hcan1.pRxMsg->Data[1];
  } break;

  case CAN_NEW_CODE_ID:
  {
    //code length: 4byte, new code: n byte, crc8: 1 byte
    //超时处理：清空接收缓存，获取上一帧接收和这一帧时间，超时1s，清空缓存
    OS_ERR err;
    static char new_code_flag = 0;
    static u32 last_frame_time, this_frame_time = 0;
    this_frame_time = OSTimeGet(&err);
    if (this_frame_time - last_frame_time > 1000)
    {
      CAN_RX_CNT = 0;
      read_data_cnt = 0;
      new_code_flag = 0;
    }
    if (new_code_flag == 0)
    {
      new_code_len = hcan1.pRxMsg->Data[0] | hcan1.pRxMsg->Data[1] << 8 | hcan1.pRxMsg->Data[2] << 16 | hcan1.pRxMsg->Data[3] << 24;
      new_code_flag = 1;
    }
    if (read_data_cnt < CAN_REC_LEN)
    {
      for (int i = 0; i < hcan1.pRxMsg->DLC; i++) {
        NEW_CODE_RX_BUF[CAN_RX_CNT * 8 + i] = hcan1.pRxMsg->Data[i];
        read_data_cnt++;
      }
      CAN_RX_CNT++;

    }
    if (read_data_cnt == new_code_len)
    {
      if (Verify_CRC8_Check_Sum(NEW_CODE_RX_BUF, read_data_cnt))
      {
        iap_write_appbin(FLASH_NEW_CODE_ADDR, NEW_CODE_RX_BUF, read_data_cnt);
        // STMFLASH_Write(UPDATE_FLAG_ADDR, (u32 *)&new_code_len, 1);
        INTX_DISABLE();
        iap_load_app(FLASH_BOOTLOADER_ADDR);
        printf("update success");
      }
      else
      {
        CAN_RX_CNT = 0;
        read_data_cnt = 0;
        new_code_flag = 0;
        printf("update new code error");

      }

    }
    last_frame_time = this_frame_time;
  } break;

  case CAN_UPDATE_SYS_TIME_ID:
  {
    RTC_Init();                     //初始化RTC
    RTC_Set_Date(hcan1.pRxMsg->Data[0], hcan1.pRxMsg->Data[1], hcan1.pRxMsg->Data[2], 7);                        //设置日期
    RTC_Set_Time(hcan1.pRxMsg->Data[3], hcan1.pRxMsg->Data[4], hcan1.pRxMsg->Data[5], RTC_HOURFORMAT12_PM);      //设置时间 ,根据实际时间修改
    HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);
    memset(new_log_file_name, 0, 18);
    sprintf((char*)new_log_file_name, "20%02d%02d%02d%02d%02d%02d.log", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date,
            RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
    new_file_setted_flag = 1;
    printf("generate file:");
    printf("%s\r\n", new_log_file_name);
    RTC_Set_WakeUp(RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 15); //配置WAKE UP中断,1分钟中断一次
  } break;

  case CAN_READ_LOG_FILE_ID:
  {
    memset(read_log_file_name, 0, 18);
    sprintf((char*)read_log_file_name, "20%02d%02d%02d%02d%02d%02d.log", hcan1.pRxMsg->Data[0], hcan1.pRxMsg->Data[1], hcan1.pRxMsg->Data[2],
            hcan1.pRxMsg->Data[3], hcan1.pRxMsg->Data[4], hcan1.pRxMsg->Data[5]);
    read_log_file_flag = 1;
  } break;

  case CAN_GET_VERSION_CHIPID_ID:
  {
    if (hcan1.pRxMsg->Data[0] == 0x01)
    {
      u8 version_chipid[15] = {(u8)MAJOR_VERSION_NUM, (u8)MINOR_VERSION_NUM, (u8)REVISON_VERSION_NUM};
      for(int i=0;i<3;++i)
			{
				version_chipid[i*4+3] = (u8)(Device_Serial[i]>>24);
        version_chipid[i*4+4] = (u8)(Device_Serial[i]>>16);
        version_chipid[i*4+5] = (u8)(Device_Serial[i]>>8);
        version_chipid[i*4+6] = (u8)Device_Serial[i];
			}
      can_send_data(&hcan1, CAN_SEND_VERSION_CHIPID_ID, version_chipid, 15);

    }
  } break;
  case CAN_SEND_MOTION_ID:
  {
    if(hcan1.pRxMsg->Data[0]&&0x08) 
      {
        BREAK_LIGHT_SLAVE=0;
        break_light_flag=0;
      }
    else 
      {
        BREAK_LIGHT_SLAVE=1;
        break_light_flag=1;
      }
  }break;
    case CAN_RECE_MOTION_ID:
  {
      if(hcan1.pRxMsg->Data[1]&&0x08) 
       {
        BREAK_LIGHT_SLAVE=0;
        break_light_flag=0;
      }
    else 
      {
        BREAK_LIGHT_SLAVE=1;
        break_light_flag=1;
      }
  }break;
  default: break;

  }
  __HAL_CAN_ENABLE_IT(&hcan1, CAN_IT_FMP0);
}

void CAN2_RX0_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan2);
  switch (hcan2.pRxMsg->StdId)
  {
//  case CAN_Encoder3_ID:
//  {

//  } break;
//  case CAN_Encoder4_ID:
//  {

//  } break;
  default: break;
  }
  __HAL_CAN_ENABLE_IT(&hcan2, CAN_IT_FMP0);
}

void CAN1_TX_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan1);
}

void CAN2_TX_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan2);
}


void can_send_current_state(void)
{
  u32 i = 0;
  static u8 can1_0x301_heart_beat = 0;


  BSP_EncoderRead();

  hcan1.pTxMsg->StdId   = CAN_SEND_STATUS_ID;
  hcan1.pTxMsg->IDE     = CAN_ID_STD;
  hcan1.pTxMsg->RTR     = CAN_RTR_DATA;
  hcan1.pTxMsg->DLC     = 0x08;

  while (hcan1.State == HAL_CAN_STATE_BUSY_TX);
  for (i = 0; i < 4 ; i++)
  {
    hcan1.pTxMsg->Data[i] = (uint8_t)(acc[i] + 127);
  }
  send_light_state1=rece_light_state1|(((u8)~BREAK_LIGHT)&0x01)<<3;
  hcan1.pTxMsg->Data[4] = send_light_state1;
  hcan1.pTxMsg->Data[5] = rece_light_state2;
  hcan1.pTxMsg->Data[6] = (!PIin(5)) | (!PIin(6) << 1) | (!PIin(7) << 2) | (!PEin(0) << 3) | (!PEin(1) << 4) | (!PIin(4) << 5);
  hcan1.pTxMsg->Data[7] = can1_0x301_heart_beat;
  HAL_CAN_Transmit(&hcan1, 10);
  if (hcan1.pTxMsg->Data[6] != 0)
  {
    //hit sth,send motion_control to stop
    hcan1.pTxMsg->StdId   = CAN_SEND_MOTION_ID;
    hcan1.pTxMsg->DLC     = 0x01;
    hcan1.pTxMsg->Data[0] = 0x08;
    HAL_CAN_Transmit(&hcan1, 10);
    BREAK_LIGHT_SLAVE=0;
  }
  else if(break_light_flag==1)
  {
    BREAK_LIGHT_SLAVE=1;
  }
  can1_0x301_heart_beat++;
}



void can_send_data(CAN_HandleTypeDef* _hcan, u32 send_id, u8* msg, u32 msg_size)
{
  u32 msg_cir = msg_size / 8;
  u32 msg_remain = msg_size % 8;
  u32 i, j = 0;
  _hcan->pTxMsg->StdId   = send_id;
  _hcan->pTxMsg->IDE     = CAN_ID_STD;
  _hcan->pTxMsg->RTR     = CAN_RTR_DATA;
  _hcan->pTxMsg->DLC     = 0x08;
  for (i = 0; i < msg_cir ; i++)
  {
    while (_hcan->State == HAL_CAN_STATE_BUSY_TX);
    for ( j = 0; j < 8 ; j++) {
      _hcan->pTxMsg->Data[j] = msg[8 * i + j];
    }
    HAL_CAN_Transmit(_hcan, 10);
  }
  if (msg_remain)
  {
    for ( j = 0; j < msg_remain ; j++) {
      _hcan->pTxMsg->Data[j] = msg[8 * msg_cir + j];
    }
    for ( j = msg_remain; j < 8 ; j++) {
      _hcan->pTxMsg->Data[j] = 0;
    }
    HAL_CAN_Transmit(_hcan, 10);
  }
}

void get_file_name(u8* date, char* file_name)
{

  char temp2[2];

  memset(file_name, 0x00, sizeof(file_name) * 16);
  for (int i = 0; i < 6; i++)
  {
    itoa(date[i], temp2, 16);
    if (date[i] < 10)
    {
      temp2[1] = temp2[0];
      temp2[0] = 0x30;
    }
    if (i == 0)strcpy(file_name, temp2);
    else strcat(file_name, temp2);
  }

  strcat(file_name, ".log");

}




char* itoa(int num, char* str, int radix)
{
  char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //索引表
  unsigned unum;//存放要转换的整数的绝对值,转换的整数可能是负数
  int i = 0, j, k; //i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。
  //获取要转换的整数的绝对值
  if (radix == 10 && num < 0) //要转换成十进制数并且是负数
  {
    unum = (unsigned) - num; //将num的绝对值赋给unum
    str[i++] = '-'; //在字符串最前面设置为'-'号，并且索引加1
  }
  else unum = (unsigned)num; //若是num为正，直接赋值给unum
  //转换部分，注意转换后是逆序的
  do
  {
    str[i++] = index[unum % (unsigned)radix]; //取unum的最后一位，并设置为str对应位，指示索引加1
    unum /= radix; //unum去掉最后一位
  } while (unum); //直至unum为0退出循环
  str[i] = '\0'; //在字符串最后添加'\0'字符，c语言字符串以'\0'结束。
  //将顺序调整过来
  if (str[0] == '-') k = 1; //如果是负数，符号不用调整，从符号后面开始调整
  else k = 0; //不是负数，全部都要调整
  char temp;//临时变量，交换两个值时用到
  for (j = k; j <= (i - 1) / 2; j++) //头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
  {
    temp = str[j]; //头部赋值给临时变量
    str[j] = str[i - 1 + k - j]; //尾部赋值给头部
    str[i - 1 + k - j] = temp; //将临时变量的值(其实就是之前的头部值)赋给尾部
  }
  return str;//返回转换后的字符串
}

void Get_SerialNum(void)
{
  for(int i=0;i<3;++i)
  {
    Device_Serial[i] = *(vu32*)(0x1fff7a10+4*i);
  }
}

