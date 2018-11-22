#include "fattester.h"
#include "usart.h"
#include "exfuns.h"
#include "malloc.h"
#include "ff.h"
#include "string.h"
#include "bsp_can.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

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

//为磁盘注册工作区
//path:磁盘路径，比如"0:"、"1:"
//mt:0，不立即注册（稍后注册）；1，立即注册
//返回值:执行结果
// u8 mf_mount(u8* path,u8 mt)
// {
// 	return f_mount(fs[1],(const TCHAR*)path,mt);
// }
//打开路径下的文件
//path:路径+文件名
//mode:打开模式
//返回值:执行结果

u16 file_num = 0;
u32 free_space;

u8 mf_open(u8*path, u8 mode)
{
	u8 res;
	res = f_open(file, (const TCHAR*)path, mode); //打开文件夹
	return res;
}
//关闭文件
//返回值:执行结果
u8 mf_close(void)
{
	f_close(file);
	return 0;
}
//读出数据
//len:读出的长度
//返回值:执行结果
u8 mf_read(u32 len)
{
	u16 i, t;
	u8 res = 0;
	u32 tlen = 0;
	printf("\r\nRead file data is:\r\n");
	for (i = 0; i < len / 4096; i++)
	{
		res = f_read(file, fatbuf, 4096, &br);
		if (res)
		{
			printf("Read Error:%d\r\n", res);
			break;
		} else
		{
			tlen += br;
			can_send_data(&hcan1, CAN_SEND_LOG_MSG_ID,fatbuf, 4096);
			for (t = 0; t < br; t++)printf("%c", fatbuf[t]);
		}
	}
	if (len % 4096)
	{
		res = f_read(file, fatbuf, len % 4096, &br);
		if (res)	//读数据出错了
		{
			printf("\r\nRead Error:%d\r\n", res);
		} else
		{	can_send_data(&hcan1, CAN_SEND_LOG_MSG_ID,fatbuf, len % 4096);
			tlen += br;
			// for (t = 0; t < br; t++)printf("%c", fatbuf[t]);
		}
	}
	if (tlen)printf("\r\nReaded data len:%d\r\n", tlen); //读到的数据长度
	printf("Read data over\r\n");
	return res;
}
//写入数据
//dat:数据缓存区
//len:写入长度
//返回值:执行结果
u8 mf_write(u8*dat, u16 len)
{
	u8 res;

	printf("\r\nBegin Write file...\r\n");
	printf("Write data len:%d\r\n", len);
	res = f_write(file, dat, len, &bw);
	if (res)
	{
		printf("Write Error:%d\r\n", res);
	} else printf("Writed data len:%d\r\n", bw);
	printf("Write data over.\r\n");
	return res;
}

//打开目录
//path:路径
//返回值:执行结果
u8 mf_opendir(u8* path)
{
	return f_opendir(&dir, (const TCHAR*)path);
}
//关闭目录
//返回值:执行结果
u8 mf_closedir(void)
{
	return f_closedir(&dir);
}
//打读取文件夹
//返回值:执行结果
u8 mf_readdir(void)
{
	u8 res;
	res = f_readdir(&dir, &fileinfo);	//读取一个文件的信息
	if (res != FR_OK)return res;		//出错了
//	printf("\r\n DIR info:\r\n");

//	printf("dir.dptr:%d\r\n",dir.dptr);
//	printf("dir.obj.id:%d\r\n",dir.obj.id);
//	printf("dir.obj.sclust:%d\r\n",dir.obj.sclust);
//	printf("dir.obj.objsize:%lld\r\n",dir.obj.objsize);
//	printf("dir.obj.c_ofs:%d\r\n",dir.obj.c_ofs);
//	printf("dir.clust:%d\r\n",dir.clust);
//	printf("dir.sect:%d\r\n",dir.sect);
//	printf("dir.blk_ofs:%d\r\n",dir.blk_ofs);

//	printf("\r\n");
//	printf("File Name is:%s\r\n",fileinfo.fname);
//	printf("File Size is:%lld\r\n",fileinfo.fsize);
//	printf("File data is:%d\r\n",fileinfo.fdate);
//	printf("File time is:%d\r\n",fileinfo.ftime);
//	printf("File Attr is:%d\r\n",fileinfo.fattrib);
//	printf("\r\n");
	return 0;
}

void ensure_free_space(u8 * path)
{
	FRESULT res;
	char *file_date;
	mf_scan_files((u8*)"0:");
	free_space = mf_showfree((u8*)"0:UnityDrive");
	while (free_space < 1024 && file_num > 1)   		//存储小于1000*512=512K时，清空旧文件，内存容量32MB
	{
		uint64_t file_name_vertor[file_num];
		res = f_opendir(&dir, (const TCHAR*)path);
		for (int i = file_num; i > 0; i--)
		{

			res = f_readdir(&dir, &fileinfo);
			if (res != FR_OK || fileinfo.fname[0] == 0) break;
			file_date = strtok(fileinfo.fname, ".log");
			file_name_vertor[i - 1] = atoll(file_date);


			printf("%s/", path);//打印路径
			printf("%s\r\n", fileinfo.fname); //打印文件名

		}

		file_sort_by_date(file_name_vertor, file_num);
		char unlink_filename[18];
		get_unlink_filename(file_name_vertor[file_num - 1], unlink_filename);

		mf_unlink((u8 *)unlink_filename);
		free_space = mf_showfree((u8*)"0:UnityDrive");
		mf_scan_files((u8*)"0:");
	}
}

void file_sort_by_date(uint64_t* file_vet, u8 num)
{
	uint64_t temp;
	for (int i = 0; i < num - 1; i++)
	{
		for (int j = 0; j < num - i - 1; j++)
		{
			if (file_vet[j] < file_vet[j + 1])
			{
				temp = file_vet[j];
				file_vet[j] = file_vet[j + 1];
				file_vet[j + 1] = temp;
			}
		}
	}
}


void get_unlink_filename(uint64_t unlink_date, char* unlink_file)
{
	char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i = 0, j, k;
	do
	{
		unlink_file[i++] = index[unlink_date % 10]; //取unlink_date的最后一位，并设置为unlink_file对应位，指示索引加1
		unlink_date /= 10; //unlink_date去掉最后一位
	} while (unlink_date); //直至unum为0退出循环
	unlink_file[i] = '\0'; //在字符串最后添加'\0'字符，c语言字符串以'\0'结束。
	//将顺序调整过来
	if (unlink_file[0] == '-') k = 1; //如果是负数，符号不用调整，从符号后面开始调整
	else k = 0; //不是负数，全部都要调整
	char temp;//临时变量，交换两个值时用到
	for (j = k; j <= (i - 1) / 2; j++) //头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
	{
		temp = unlink_file[j]; //头部赋值给临时变量
		unlink_file[j] = unlink_file[i - 1 + k - j]; //尾部赋值给头部
		unlink_file[i - 1 + k - j] = temp; //将临时变量的值(其实就是之前的头部值)赋给尾部
	}
	strcat(unlink_file, ".log");
}

u8 remove_files(u8 * path)
{
	FRESULT res;
	res = f_opendir(&dir, (const TCHAR*)path); //打开一个目录

	if (res == FR_OK)
	{
		printf("\r\n");
		while (1)
		{
			res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
			//if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
			f_unlink(fileinfo.fname);
		}

	}
	return res;
}


//遍历文件
//path:路径
//返回值:执行结果
u8 mf_scan_files(u8 * path)
{
	FRESULT res;
	res = f_opendir(&dir, (const TCHAR*)path); //打开一个目录

	if (res == FR_OK)
	{

		// printf("\r\n");
		file_num = 0;
		while (1)
		{
			res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
			//if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
			// printf("%s/", path);//打印路径
			// printf("%s\r\n", fileinfo.fname); //打印文件名
			file_num++;
		}
		// printf("%d", file_num );
		//printf("files left !");
	}
	return res;
}
//显示剩余容量
//drv:盘符
//返回值:剩余容量(字节)
u32 mf_showfree(u8 *drv)
{
	FATFS *fs1;
	u8 res;
	u32 fre_clust = 0, fre_sect = 0, tot_sect = 0;
	//得到磁盘信息及空闲簇数量
	res = f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
	if (res == 0)
	{
		tot_sect = (fs1->n_fatent - 2) * fs1->csize;//得到总扇区数
		fre_sect = fre_clust * fs1->csize;			//得到空闲扇区数
#if _MAX_SS!=512
		tot_sect *= fs1->ssize / 512;
		fre_sect *= fs1->ssize / 512;
#endif
		// if (tot_sect < 20480) //总容量小于10M
		// {
		// 	/* Print free space in unit of KB (assuming 512 bytes/sector) */
		// 	printf("\r\n磁盘总容量:%d KB\r\n"
		// 	       "可用空间:%d KB\r\n",
		// 	       tot_sect >> 1, fre_sect >> 1);
		// } else
		// {
		// 	/* Print free space in unit of KB (assuming 512 bytes/sector) */
		// 	printf("\r\n磁盘总容量:%d MB\r\n"
		// 	       "可用空间:%d MB\r\n",
		// 	       tot_sect >> 11, fre_sect >> 11);
		// }
	}
	return fre_sect;
}
//文件读写指针偏移
//offset:相对首地址的偏移量
//返回值:执行结果.
u8 mf_lseek(u32 offset)
{
	return f_lseek(file, offset);
}
//读取文件当前读写指针的位置.
//返回值:位置
u32 mf_tell(void)
{
	return f_tell(file);
}
//读取文件大小
//返回值:文件大小
u32 mf_size(void)
{
	return f_size(file);
}
//创建目录
//pname:目录路径+名字
//返回值:执行结果
u8 mf_mkdir(u8*pname)
{
	return f_mkdir((const TCHAR *)pname);
}
//格式化
//path:磁盘路径，比如"0:"、"1:"
//mode:模式
//au:簇大小
//返回值:执行结果
u8 mf_fmkfs(u8* path, u8 mode, u16 au)
{
	return f_mkfs((const TCHAR*)path, mode, au); //格式化,drv:盘符;mode:模式;au:簇大小
}
//删除文件/目录
//pname:文件/目录路径+名字
//返回值:执行结果
u8 mf_unlink(u8 *pname)
{
	return  f_unlink((const TCHAR *)pname);
}

//修改文件/目录名字(如果目录不同,还可以移动文件哦!)
//oldname:之前的名字
//newname:新名字
//返回值:执行结果
u8 mf_rename(u8 *oldname, u8* newname)
{
	return  f_rename((const TCHAR *)oldname, (const TCHAR *)newname);
}
//获取盘符（磁盘名字）
//path:磁盘路径，比如"0:"、"1:"
void mf_getlabel(u8 *path)
{
	u8 buf[20];
	u32 sn = 0;
	u8 res;
	res = f_getlabel ((const TCHAR *)path, (TCHAR *)buf, (DWORD*)&sn);
	if (res == FR_OK)
	{
		printf("\r\n磁盘%s 的盘符为:%s\r\n", path, buf);
		printf("磁盘%s 的序列号:%X\r\n\r\n", path, sn);
	} else printf("\r\n获取失败，错误码:%X\r\n", res);
}
//设置盘符（磁盘名字），最长11个字符！！，支持数字和大写字母组合以及汉字等
//path:磁盘号+名字，比如"0:ALIENTEK"、"1:OPENEDV"
void mf_setlabel(u8 *path)
{
	u8 res;
	res = f_setlabel ((const TCHAR *)path);
	if (res == FR_OK)
	{
		printf("\r\n磁盘盘符设置成功:%s\r\n", path);
	} else printf("\r\n磁盘盘符设置失败，错误码:%X\r\n", res);
}

//从文件里面读取一段字符串
//size:要读取的长度
void mf_gets(u16 size)
{
	TCHAR* rbuf;
	rbuf = f_gets((TCHAR*)fatbuf, size, file);
	if (*rbuf == 0)return  ; //没有数据读到
	else
	{
		printf("\r\nThe String Readed Is:%s\r\n", rbuf);
	}
}
//需要_USE_STRFUNC>=1
//写一个字符到文件
//c:要写入的字符
//返回值:执行结果
u8 mf_putc(u8 c)
{
	return f_putc((TCHAR)c, file);
}
//写字符串到文件
//c:要写入的字符串
//返回值:写入的字符串长度
u8 mf_puts(u8*c)
{
	return f_puts((TCHAR*)c, file);
}



FRESULT open_append (
    FIL* fp,            /* [OUT] File object to create */
    const char* path    /* [IN]  File name to be opened */
)
{
	FRESULT fr;

	/* Opens an existing file. If not exist, creates a new file. */
	fr = f_open(fp, path, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
	if (fr == FR_OK) {
		/* Seek to end of the file to append data */
		fr = f_lseek(fp, f_size(fp));
		if (fr != FR_OK)
			f_close(fp);
	}
	return fr;
}





//====================================================================================================
//函 数 名 : f_deldir
//函数功能 : 移除一个文件夹，包括其本身和其子文件夹，子文件
//输    入 : const TCHAR *path---指向要移除的空结尾字符串对象的指针
//输    出 : 无
//返 回 值 : FR_OK(0)：           函数成功
//           FR_NO_FILE：         无法找到文件或目录
//           FR_NO_PATH：         无法找到路径
//           FR_INVALID_NAME：    文件名非法
//           FR_INVALID_DRIVE：   驱动器号非法
//           FR_DENIED：          函数由于以下原因被拒绝：
//               对象属性为只读；
//               目录下非空；
//               当前目录。
//           FR_NOT_READY：       磁盘驱动器无法工作，由于驱动器中没有媒体或其他原因
//           FR_WRITE_PROTECTED： 媒体写保护
//           FR_DISK_ERR：        函数失败由于磁盘运行的一个错误
//           FR_INT_ERR：         函数失败由于一个错误的 FAT 结构或内部错误
//           FR_NOT_ENABLED：     逻辑驱动器没有工作区
//           FR_NO_FILESYSTEM：   驱动器上没有合法的 FAT 卷
//           FR_LOCKED：          函数被拒由于文件共享机制（_FS_SHARE）
//备    注 : f_deldir 函数用来移除一个文件夹及其子文件夹、子文件，但不能移除已经打开的对象。
//====================================================================================================

FRESULT f_deldir(const TCHAR *path)
{
	FRESULT res;
	DIR   dir;     /* 文件夹对象 */ //36  bytes
	FILINFO fno;   /* 文件属性 */   //32  bytes
	TCHAR file[_MAX_LFN + 1] = {0};
// #if _USE_LFN
//     TCHAR lname[_MAX_LFN + 1] = {0};
// #endif
// #if _USE_LFN
//     fno.fsize = _MAX_LFN;
//     fno.fname = lname;    //必须赋初值
// #endif
	//打开文件夹
	res = f_opendir(&dir, path);
	//持续读取文件夹内容
	while ((res == FR_OK) && (FR_OK == f_readdir(&dir, &fno)))
	{
		//若是"."或".."文件夹，跳过
		if (0 == strlen(fno.fname))          break;     //若读到的文件名为空
		if (0 == strcmp(fno.fname, "."))     continue;  //若读到的文件名为当前文件夹
		if (0 == strcmp(fno.fname, ".."))    continue;  //若读到的文件名为上一级文件夹
		memset(file, 0, sizeof(file));

		sprintf((char*)file, "%s/%s", path, fno.fname);

		if (fno.fattrib & AM_DIR)
		{	//若是文件夹，递归删除
			res = f_deldir(file);
		}
		else
		{	//若是文件，直接删除
			res = f_unlink(file);
		}
	}
	//删除本身
	if (res == FR_OK)    res = f_unlink(path);
	return res;
}

void rm_not_log(const TCHAR *path)
{
	FRESULT res;
	res = f_opendir(&dir, (const TCHAR*)path); //打开一个目录

	if (res == FR_OK)
	{
		printf("\r\n");
		while (1)
		{
			res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
			//if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
			printf("%s/", path);//打印路径
			printf("%s\r\n", fileinfo.fname); //打印文件名
			if (f_typetell((u8 *)fileinfo.fname) != T_LOG)
			{
				f_unlink(fileinfo.fname);
			}
		}
	}

}

void rm_not_bin(const TCHAR *path)
{
	FRESULT res;
	res = f_opendir(&dir, (const TCHAR*)path); //打开一个目录

	if (res == FR_OK)
	{
		printf("\r\n");
		while (1)
		{
			res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
			//if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
			printf("%s/", path);//打印路径
			printf("%s\r\n", fileinfo.fname); //打印文件名
			if (f_typetell((u8 *)fileinfo.fname) != T_BIN)
			{
				f_unlink(fileinfo.fname);
			}
		}
	}
}



// //扫描flash中的文件数，并保持7个有效文件

// if(flash_file_num<7)
// {
//     memset(file_array[flash_file_num], 0, sizeof(file_array[flash_file_num]));
//     strcpy(file_array[flash_file_num], name_string);
// }
// else
// {

//     res = f_opendir(&dir,"/user");
//     res = f_unlink (file_array[0]);



//     for(i=0;i<6;i++)
//         strcpy(file_array[i],file_array[i+1]);

//     memset(file_array[6], 0, sizeof(file_array[6]));
//     strcpy(file_array[6], name_string);
//  }





