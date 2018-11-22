#include "log.h"
#include "rtc.h"
#include "ff.h"
#include "fattester.h"
#include "exfuns.h"
#include "crc_check.h"
#include "includes.h"

LOG_MSG_TypeDef  LOG_DATA_BUF[LOG_LEN] __attribute__ ((at(0X20001000)));
LOG_MSG_TypeDef log_msg;

u8 new_file_setted_flag = 0;
u8 read_log_file_flag = 0;
char new_log_file_name[18];
char read_log_file_name[18];
u16 log_buf_head, log_buf_tail = 0;

void Log_Can_Data(CAN_HandleTypeDef* _hcan)
{
	HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
	log_msg.can_id = _hcan->pRxMsg->StdId;
	log_msg.len = _hcan->pRxMsg->DLC;
	for (int i = 0; i < _hcan->pRxMsg->DLC; i++)
	{
		log_msg.data[i] = _hcan->pRxMsg->Data[i];
	}
	for (int i = _hcan->pRxMsg->DLC; i < 8 ; i++)
	{
		log_msg.data[i] = 0;
	}
	log_msg.hour = RTC_TimeStruct.Hours;
	log_msg.min = RTC_TimeStruct.Minutes;
	log_msg.sec = RTC_TimeStruct.Seconds;
	// Append_CRC8_Check_Sum((u8 *)&log_msg, 16);
	LOG_DATA_BUF[log_buf_head] = log_msg;
	log_buf_head = (log_buf_head + 1) % LOG_LEN;
}

void Save_Log_Data_To_Flash(void)
{
	OS_ERR err;
	static int16_t buffer_data_len = 0;
	static u32 last_time, cur_time, time_break = 0;
	static char circle_flag = 0;
	static u8 msg_size = sizeof(LOG_MSG_TypeDef);
	static u16 temp_buf_head, temp_buf_tail;

	temp_buf_head = log_buf_head;
	temp_buf_tail = log_buf_tail;
	buffer_data_len = temp_buf_head - temp_buf_tail;
	cur_time = OSTimeGet(&err);
	time_break = cur_time - last_time;
	if (buffer_data_len < 0)
	{
		buffer_data_len += LOG_LEN;
		circle_flag = 1;
	}
	if (buffer_data_len)
	{	//
		if (buffer_data_len >= (4096 / msg_size) || time_break > 1000 )		//缓存超过4K时，再存储至flash,不要频繁反复写入flash,片外flash簇为4K
		{
			open_append(file, new_log_file_name);
			if (circle_flag == 0)
			{
				mf_write((u8*)&LOG_DATA_BUF[temp_buf_tail], buffer_data_len * msg_size);
			}
			else
			{	circle_flag = 0;
				mf_write((u8*)&LOG_DATA_BUF[temp_buf_tail], (LOG_LEN - temp_buf_tail)*msg_size);
				if (temp_buf_head != 0)mf_write((u8*)&LOG_DATA_BUF[0], temp_buf_head * msg_size);
			}
			f_close(file);

			log_buf_tail = temp_buf_head;

			last_time = cur_time;
		}
	}

}

