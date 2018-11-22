#ifndef __FATTESTER_H
#define __FATTESTER_H
#include "sys.h"
#include "ff.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//FATFS 测试代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/1/7
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////
extern u16 file_num;
// u8 mf_mount(u8* path,u8 mt);
u8 mf_open(u8*path, u8 mode);
u8 mf_close(void);
u8 mf_read(u32 len);
u8 mf_write(u8*dat, u16 len);
u8 mf_opendir(u8* path);
u8 mf_closedir(void);
u8 mf_readdir(void);
u8 mf_scan_files(u8 * path);
u32 mf_showfree(u8 *drv);
u8 mf_lseek(u32 offset);
u32 mf_tell(void);
u32 mf_size(void);
u8 mf_mkdir(u8*pname);
u8 mf_fmkfs(u8* path, u8 mode, u16 au);
u8 mf_unlink(u8 *pname);
u8 mf_rename(u8 *oldname, u8* newname);
void mf_getlabel(u8 *path);
void mf_setlabel(u8 *path);
void mf_gets(u16 size);
u8 mf_putc(u8 c);
u8 mf_puts(u8*c);
FRESULT open_append (FIL* fp, const char* path);

void ensure_free_space(u8 * path);
void file_sort_by_date(uint64_t* file_vet,u8 num);
void get_unlink_filename(uint64_t unlink_date, char* unlink_file);
u8 remove_files(u8 * path);
FRESULT f_deldir(const TCHAR *path);
void rm_not_log(const TCHAR *path);
void rm_not_bin(const TCHAR *path);

#endif





























