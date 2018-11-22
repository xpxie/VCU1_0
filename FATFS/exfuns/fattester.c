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
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//FATFS ���Դ���
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/7
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

//Ϊ����ע�Ṥ����
//path:����·��������"0:"��"1:"
//mt:0��������ע�ᣨ�Ժ�ע�ᣩ��1������ע��
//����ֵ:ִ�н��
// u8 mf_mount(u8* path,u8 mt)
// {
// 	return f_mount(fs[1],(const TCHAR*)path,mt);
// }
//��·���µ��ļ�
//path:·��+�ļ���
//mode:��ģʽ
//����ֵ:ִ�н��

u16 file_num = 0;
u32 free_space;

u8 mf_open(u8*path, u8 mode)
{
	u8 res;
	res = f_open(file, (const TCHAR*)path, mode); //���ļ���
	return res;
}
//�ر��ļ�
//����ֵ:ִ�н��
u8 mf_close(void)
{
	f_close(file);
	return 0;
}
//��������
//len:�����ĳ���
//����ֵ:ִ�н��
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
		if (res)	//�����ݳ�����
		{
			printf("\r\nRead Error:%d\r\n", res);
		} else
		{	can_send_data(&hcan1, CAN_SEND_LOG_MSG_ID,fatbuf, len % 4096);
			tlen += br;
			// for (t = 0; t < br; t++)printf("%c", fatbuf[t]);
		}
	}
	if (tlen)printf("\r\nReaded data len:%d\r\n", tlen); //���������ݳ���
	printf("Read data over\r\n");
	return res;
}
//д������
//dat:���ݻ�����
//len:д�볤��
//����ֵ:ִ�н��
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

//��Ŀ¼
//path:·��
//����ֵ:ִ�н��
u8 mf_opendir(u8* path)
{
	return f_opendir(&dir, (const TCHAR*)path);
}
//�ر�Ŀ¼
//����ֵ:ִ�н��
u8 mf_closedir(void)
{
	return f_closedir(&dir);
}
//���ȡ�ļ���
//����ֵ:ִ�н��
u8 mf_readdir(void)
{
	u8 res;
	res = f_readdir(&dir, &fileinfo);	//��ȡһ���ļ�����Ϣ
	if (res != FR_OK)return res;		//������
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
	while (free_space < 1024 && file_num > 1)   		//�洢С��1000*512=512Kʱ����վ��ļ����ڴ�����32MB
	{
		uint64_t file_name_vertor[file_num];
		res = f_opendir(&dir, (const TCHAR*)path);
		for (int i = file_num; i > 0; i--)
		{

			res = f_readdir(&dir, &fileinfo);
			if (res != FR_OK || fileinfo.fname[0] == 0) break;
			file_date = strtok(fileinfo.fname, ".log");
			file_name_vertor[i - 1] = atoll(file_date);


			printf("%s/", path);//��ӡ·��
			printf("%s\r\n", fileinfo.fname); //��ӡ�ļ���

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
		unlink_file[i++] = index[unlink_date % 10]; //ȡunlink_date�����һλ��������Ϊunlink_file��Ӧλ��ָʾ������1
		unlink_date /= 10; //unlink_dateȥ�����һλ
	} while (unlink_date); //ֱ��unumΪ0�˳�ѭ��
	unlink_file[i] = '\0'; //���ַ���������'\0'�ַ���c�����ַ�����'\0'������
	//��˳���������
	if (unlink_file[0] == '-') k = 1; //����Ǹ��������Ų��õ������ӷ��ź��濪ʼ����
	else k = 0; //���Ǹ�����ȫ����Ҫ����
	char temp;//��ʱ��������������ֵʱ�õ�
	for (j = k; j <= (i - 1) / 2; j++) //ͷβһһ�Գƽ�����i��ʵ�����ַ����ĳ��ȣ��������ֵ�ȳ�����1
	{
		temp = unlink_file[j]; //ͷ����ֵ����ʱ����
		unlink_file[j] = unlink_file[i - 1 + k - j]; //β����ֵ��ͷ��
		unlink_file[i - 1 + k - j] = temp; //����ʱ������ֵ(��ʵ����֮ǰ��ͷ��ֵ)����β��
	}
	strcat(unlink_file, ".log");
}

u8 remove_files(u8 * path)
{
	FRESULT res;
	res = f_opendir(&dir, (const TCHAR*)path); //��һ��Ŀ¼

	if (res == FR_OK)
	{
		printf("\r\n");
		while (1)
		{
			res = f_readdir(&dir, &fileinfo);                   //��ȡĿ¼�µ�һ���ļ�
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //������/��ĩβ��,�˳�
			//if (fileinfo.fname[0] == '.') continue;             //�����ϼ�Ŀ¼
			f_unlink(fileinfo.fname);
		}

	}
	return res;
}


//�����ļ�
//path:·��
//����ֵ:ִ�н��
u8 mf_scan_files(u8 * path)
{
	FRESULT res;
	res = f_opendir(&dir, (const TCHAR*)path); //��һ��Ŀ¼

	if (res == FR_OK)
	{

		// printf("\r\n");
		file_num = 0;
		while (1)
		{
			res = f_readdir(&dir, &fileinfo);                   //��ȡĿ¼�µ�һ���ļ�
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //������/��ĩβ��,�˳�
			//if (fileinfo.fname[0] == '.') continue;             //�����ϼ�Ŀ¼
			// printf("%s/", path);//��ӡ·��
			// printf("%s\r\n", fileinfo.fname); //��ӡ�ļ���
			file_num++;
		}
		// printf("%d", file_num );
		//printf("files left !");
	}
	return res;
}
//��ʾʣ������
//drv:�̷�
//����ֵ:ʣ������(�ֽ�)
u32 mf_showfree(u8 *drv)
{
	FATFS *fs1;
	u8 res;
	u32 fre_clust = 0, fre_sect = 0, tot_sect = 0;
	//�õ�������Ϣ�����д�����
	res = f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
	if (res == 0)
	{
		tot_sect = (fs1->n_fatent - 2) * fs1->csize;//�õ���������
		fre_sect = fre_clust * fs1->csize;			//�õ�����������
#if _MAX_SS!=512
		tot_sect *= fs1->ssize / 512;
		fre_sect *= fs1->ssize / 512;
#endif
		// if (tot_sect < 20480) //������С��10M
		// {
		// 	/* Print free space in unit of KB (assuming 512 bytes/sector) */
		// 	printf("\r\n����������:%d KB\r\n"
		// 	       "���ÿռ�:%d KB\r\n",
		// 	       tot_sect >> 1, fre_sect >> 1);
		// } else
		// {
		// 	/* Print free space in unit of KB (assuming 512 bytes/sector) */
		// 	printf("\r\n����������:%d MB\r\n"
		// 	       "���ÿռ�:%d MB\r\n",
		// 	       tot_sect >> 11, fre_sect >> 11);
		// }
	}
	return fre_sect;
}
//�ļ���дָ��ƫ��
//offset:����׵�ַ��ƫ����
//����ֵ:ִ�н��.
u8 mf_lseek(u32 offset)
{
	return f_lseek(file, offset);
}
//��ȡ�ļ���ǰ��дָ���λ��.
//����ֵ:λ��
u32 mf_tell(void)
{
	return f_tell(file);
}
//��ȡ�ļ���С
//����ֵ:�ļ���С
u32 mf_size(void)
{
	return f_size(file);
}
//����Ŀ¼
//pname:Ŀ¼·��+����
//����ֵ:ִ�н��
u8 mf_mkdir(u8*pname)
{
	return f_mkdir((const TCHAR *)pname);
}
//��ʽ��
//path:����·��������"0:"��"1:"
//mode:ģʽ
//au:�ش�С
//����ֵ:ִ�н��
u8 mf_fmkfs(u8* path, u8 mode, u16 au)
{
	return f_mkfs((const TCHAR*)path, mode, au); //��ʽ��,drv:�̷�;mode:ģʽ;au:�ش�С
}
//ɾ���ļ�/Ŀ¼
//pname:�ļ�/Ŀ¼·��+����
//����ֵ:ִ�н��
u8 mf_unlink(u8 *pname)
{
	return  f_unlink((const TCHAR *)pname);
}

//�޸��ļ�/Ŀ¼����(���Ŀ¼��ͬ,�������ƶ��ļ�Ŷ!)
//oldname:֮ǰ������
//newname:������
//����ֵ:ִ�н��
u8 mf_rename(u8 *oldname, u8* newname)
{
	return  f_rename((const TCHAR *)oldname, (const TCHAR *)newname);
}
//��ȡ�̷����������֣�
//path:����·��������"0:"��"1:"
void mf_getlabel(u8 *path)
{
	u8 buf[20];
	u32 sn = 0;
	u8 res;
	res = f_getlabel ((const TCHAR *)path, (TCHAR *)buf, (DWORD*)&sn);
	if (res == FR_OK)
	{
		printf("\r\n����%s ���̷�Ϊ:%s\r\n", path, buf);
		printf("����%s �����к�:%X\r\n\r\n", path, sn);
	} else printf("\r\n��ȡʧ�ܣ�������:%X\r\n", res);
}
//�����̷����������֣����11���ַ�������֧�����ֺʹ�д��ĸ����Լ����ֵ�
//path:���̺�+���֣�����"0:ALIENTEK"��"1:OPENEDV"
void mf_setlabel(u8 *path)
{
	u8 res;
	res = f_setlabel ((const TCHAR *)path);
	if (res == FR_OK)
	{
		printf("\r\n�����̷����óɹ�:%s\r\n", path);
	} else printf("\r\n�����̷�����ʧ�ܣ�������:%X\r\n", res);
}

//���ļ������ȡһ���ַ���
//size:Ҫ��ȡ�ĳ���
void mf_gets(u16 size)
{
	TCHAR* rbuf;
	rbuf = f_gets((TCHAR*)fatbuf, size, file);
	if (*rbuf == 0)return  ; //û�����ݶ���
	else
	{
		printf("\r\nThe String Readed Is:%s\r\n", rbuf);
	}
}
//��Ҫ_USE_STRFUNC>=1
//дһ���ַ����ļ�
//c:Ҫд����ַ�
//����ֵ:ִ�н��
u8 mf_putc(u8 c)
{
	return f_putc((TCHAR)c, file);
}
//д�ַ������ļ�
//c:Ҫд����ַ���
//����ֵ:д����ַ�������
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
//�� �� �� : f_deldir
//�������� : �Ƴ�һ���ļ��У������䱾��������ļ��У����ļ�
//��    �� : const TCHAR *path---ָ��Ҫ�Ƴ��Ŀս�β�ַ��������ָ��
//��    �� : ��
//�� �� ֵ : FR_OK(0)��           �����ɹ�
//           FR_NO_FILE��         �޷��ҵ��ļ���Ŀ¼
//           FR_NO_PATH��         �޷��ҵ�·��
//           FR_INVALID_NAME��    �ļ����Ƿ�
//           FR_INVALID_DRIVE��   �������ŷǷ�
//           FR_DENIED��          ������������ԭ�򱻾ܾ���
//               ��������Ϊֻ����
//               Ŀ¼�·ǿգ�
//               ��ǰĿ¼��
//           FR_NOT_READY��       �����������޷�������������������û��ý�������ԭ��
//           FR_WRITE_PROTECTED�� ý��д����
//           FR_DISK_ERR��        ����ʧ�����ڴ������е�һ������
//           FR_INT_ERR��         ����ʧ������һ������� FAT �ṹ���ڲ�����
//           FR_NOT_ENABLED��     �߼�������û�й�����
//           FR_NO_FILESYSTEM��   ��������û�кϷ��� FAT ��
//           FR_LOCKED��          �������������ļ�������ƣ�_FS_SHARE��
//��    ע : f_deldir ���������Ƴ�һ���ļ��м������ļ��С����ļ����������Ƴ��Ѿ��򿪵Ķ���
//====================================================================================================

FRESULT f_deldir(const TCHAR *path)
{
	FRESULT res;
	DIR   dir;     /* �ļ��ж��� */ //36  bytes
	FILINFO fno;   /* �ļ����� */   //32  bytes
	TCHAR file[_MAX_LFN + 1] = {0};
// #if _USE_LFN
//     TCHAR lname[_MAX_LFN + 1] = {0};
// #endif
// #if _USE_LFN
//     fno.fsize = _MAX_LFN;
//     fno.fname = lname;    //���븳��ֵ
// #endif
	//���ļ���
	res = f_opendir(&dir, path);
	//������ȡ�ļ�������
	while ((res == FR_OK) && (FR_OK == f_readdir(&dir, &fno)))
	{
		//����"."��".."�ļ��У�����
		if (0 == strlen(fno.fname))          break;     //���������ļ���Ϊ��
		if (0 == strcmp(fno.fname, "."))     continue;  //���������ļ���Ϊ��ǰ�ļ���
		if (0 == strcmp(fno.fname, ".."))    continue;  //���������ļ���Ϊ��һ���ļ���
		memset(file, 0, sizeof(file));

		sprintf((char*)file, "%s/%s", path, fno.fname);

		if (fno.fattrib & AM_DIR)
		{	//�����ļ��У��ݹ�ɾ��
			res = f_deldir(file);
		}
		else
		{	//�����ļ���ֱ��ɾ��
			res = f_unlink(file);
		}
	}
	//ɾ������
	if (res == FR_OK)    res = f_unlink(path);
	return res;
}

void rm_not_log(const TCHAR *path)
{
	FRESULT res;
	res = f_opendir(&dir, (const TCHAR*)path); //��һ��Ŀ¼

	if (res == FR_OK)
	{
		printf("\r\n");
		while (1)
		{
			res = f_readdir(&dir, &fileinfo);                   //��ȡĿ¼�µ�һ���ļ�
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //������/��ĩβ��,�˳�
			//if (fileinfo.fname[0] == '.') continue;             //�����ϼ�Ŀ¼
			printf("%s/", path);//��ӡ·��
			printf("%s\r\n", fileinfo.fname); //��ӡ�ļ���
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
	res = f_opendir(&dir, (const TCHAR*)path); //��һ��Ŀ¼

	if (res == FR_OK)
	{
		printf("\r\n");
		while (1)
		{
			res = f_readdir(&dir, &fileinfo);                   //��ȡĿ¼�µ�һ���ļ�
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //������/��ĩβ��,�˳�
			//if (fileinfo.fname[0] == '.') continue;             //�����ϼ�Ŀ¼
			printf("%s/", path);//��ӡ·��
			printf("%s\r\n", fileinfo.fname); //��ӡ�ļ���
			if (f_typetell((u8 *)fileinfo.fname) != T_BIN)
			{
				f_unlink(fileinfo.fname);
			}
		}
	}
}



// //ɨ��flash�е��ļ�����������7����Ч�ļ�

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





