#ifndef __CRC_CHECK_H
#define __CRC_CHECK_H
#include "sys.h"

uint16_t Verify_CRC8_Check_Sum(uint8_t* pchMessage, uint32_t dwLength);
uint32_t Verify_CRC16_Check_Sum(uint8_t* pchMessage, uint32_t dwLength);
void Append_CRC8_Check_Sum(uint8_t* pchMessage, uint32_t dwLength);

#endif
