#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
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

typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.   

//Current version 1.0.0
#define MAJOR_VERSION_NUM		1
#define MINOR_VERSION_NUM		0
#define REVISON_VERSION_NUM		0


#define FLASH_NEW_CODE_ADDR		0x080E0000  	//��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)
											//����0X08000000~0X0800FFFF�Ŀռ�ΪBootloaderʹ��(��64KB)	
#define FLASH_BOOTLOADER_ADDR	0x08000000   
//#define UPDATE_FLAG_ADDR  0x80C0000 
void iap_load_app(u32 app_address);			//��ת��APP����ִ��
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//��ָ����ַ��ʼ,д��bin
#endif







































