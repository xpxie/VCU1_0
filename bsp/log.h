#ifndef __LOG_H
#define __LOG_H
#include "sys.h"


#define LOG_LEN 	1024			//缓存大小1024*16=16K
extern  u8 new_file_setted_flag;
extern  u8 read_log_file_flag;
extern  char new_log_file_name[18];
extern char read_log_file_name[18];

typedef struct {
	u32 can_id;
	u8 len;
	u8 data[8];
	u8 hour;
	u8 min;
	u8 sec;	
} LOG_MSG_TypeDef;

void Log_Can_Data(CAN_HandleTypeDef* _hcan);
void Save_Log_Data_To_Flash(void);
#endif

