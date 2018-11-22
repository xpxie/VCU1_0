#ifndef __BSP_CAN_H
#define __BSP_CAN_H
#include "sys.h"


#define CAN_REC_LEN  			1024*128//128K

typedef enum {

	CAN_SEND_MOTION_ID			= 0X1F1,
	CAN_RECE_MOTION_ID			= 0X2F2,

	CAN_LIGHT_HORN_CONTROL_ID 	= 0x301,
	CAN_UPDATE_SYS_TIME_ID 		= 0X302,
	CAN_READ_LOG_FILE_ID 		= 0X303,
	CAN_NEW_CODE_ID 			= 0x304,
	CAN_GET_VERSION_CHIPID_ID	= 0X305,

	CAN_SEND_STATUS_ID 			= 0x306,
	CAN_FUNCTION_FLAG_ID		= 0X307,
	CAN_SEND_LOG_MSG_ID			= 0X308,
	CAN_SEND_VERSION_CHIPID_ID	= 0X309,

} eCAN_MSG_ID;




extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;




void MX_CAN1_Init(void);
void MX_CAN2_Init(void);
void can_init(CAN_HandleTypeDef* _hcan);
void can_send_data(CAN_HandleTypeDef* _hcan, u32 send_id, u8* msg, u32 msg_size);
void can_send_current_state(void);
void get_file_name(u8* date, char* file_name);
char* itoa(int num, char* str, int radix);
void Get_SerialNum(void);
#endif // __BSP_CAN_H 

