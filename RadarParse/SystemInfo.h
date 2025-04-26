#pragma once


#include <windows.h>
#include <tchar.h>
#include <nb30.h>

#include <winioctl.h>
#include <intrin.h>
#include <stdio.h>

#pragma comment(lib, "netapi32.lib" )

typedef struct _ASTAT_
{
	ADAPTER_STATUS adapt;
	NAME_BUFFER NameBuff[30];
} ASTAT;

class SystemInfo
{
private:
	// 输入参数:lana_num为网卡编号,一般地,从0开始,但在Windows 2000中并不一定是连续分配的
	static void  getmac_one(char *pID, int lana_num)
	{
		NCB ncb;
		UCHAR uRetCode;
		ASTAT Adapter;  // 定义一个存放返回网卡信息的变量

		memset(&ncb, 0, sizeof(ncb));
		ncb.ncb_command = NCBRESET;
		ncb.ncb_lana_num = lana_num; // 指定网卡号

		// 首先对选定的网卡发送一个NCBRESET命令,以便进行初始化
		uRetCode = Netbios(&ncb);

		memset(&ncb, 0, sizeof(ncb));
		ncb.ncb_command = NCBASTAT;
		ncb.ncb_lana_num = lana_num; // 指定网卡号
		strcpy((char *)ncb.ncb_callname, "*");
		ncb.ncb_buffer = (unsigned char *)&Adapter; // 指定返回的信息存放的变量
		ncb.ncb_length = sizeof(Adapter);

		// 接着,可以发送NCBASTAT命令以获取网卡的信息
		uRetCode = Netbios(&ncb);

		if (uRetCode == 0)
		{
			// 把网卡MAC地址格式化成常用的16进制形式,如0010-A4E4-5802
			sprintf(pID, "%02X%02X-%02X%02X-%02X%02X,",
				Adapter.adapt.adapter_address[0],
				Adapter.adapt.adapter_address[1],
				Adapter.adapt.adapter_address[2],
				Adapter.adapt.adapter_address[3],
				Adapter.adapt.adapter_address[4],
				Adapter.adapt.adapter_address[5]);
			printf(pID);
		}
	};

	// Model Number: 40 ASCII Chars  
	// SerialNumber: 20 ASCII Chars  
	static BOOL GetPhyDriveSerial(LPTSTR pModelNo, LPTSTR pSerialNo)
	{
		//-1是因为 SENDCMDOUTPARAMS 的结尾是 BYTE bBuffer[1];  
		BYTE IdentifyResult[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
		DWORD dwBytesReturned;
		GETVERSIONINPARAMS get_version;
		SENDCMDINPARAMS send_cmd = { 0 };

		HANDLE hFile = CreateFile(_T("\\\\.\\PHYSICALDRIVE0"), GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			strcpy((char *)pSerialNo, "ERROR:CAN'T CreateFile");
			return FALSE;
		}

		//get version  
		DeviceIoControl(hFile, SMART_GET_VERSION, NULL, 0,
			&get_version, sizeof(get_version), &dwBytesReturned, NULL);

		//identify device  
		send_cmd.irDriveRegs.bCommandReg = (get_version.bIDEDeviceMap & 0x10) ? ATAPI_ID_CMD : ID_CMD;
		DeviceIoControl(hFile, SMART_RCV_DRIVE_DATA, &send_cmd, sizeof(SENDCMDINPARAMS) - 1,
			IdentifyResult, sizeof(IdentifyResult), &dwBytesReturned, NULL);
		CloseHandle(hFile);

		//adjust the byte order  
		PUSHORT pWords = (USHORT*)(((SENDCMDOUTPARAMS*)IdentifyResult)->bBuffer);
		ToLittleEndian(pWords, 27, 46, pModelNo);
		ToLittleEndian(pWords, 10, 19, pSerialNo);
		return TRUE;
	}

	static void ToLittleEndian(PUSHORT pWords, int nFirstIndex, int nLastIndex, LPTSTR pBuf)
	{
		int index;
		LPTSTR pDest = pBuf;
		for (index = nFirstIndex; index <= nLastIndex; ++index)
		{
			pDest[0] = pWords[index] >> 8;
			pDest[1] = pWords[index] & 0xFF;
			pDest += 2;
		}
		*pDest = 0;

		//trim space at the endof string; 0x20: _T(' ')  
		--pDest;
		while (*pDest == 0x20)
		{
			*pDest = 0;
			--pDest;
		}
	}

	static void TrimStart(LPTSTR pBuf)
	{
		if (*pBuf != 0x20)
			return;

		LPTSTR pDest = pBuf;
		LPTSTR pSrc = pBuf + 1;
		while (*pSrc == 0x20)
			++pSrc;

		while (*pSrc)
		{
			*pDest = *pSrc;
			++pDest;
			++pSrc;
		}
		*pDest = 0;
	}

	static char* TCHAR2char(const TCHAR* STR)
	{
		//返回字符串的长度
		//int size = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, FALSE);

		//申请一个多字节的字符串变量
		//char* str = new char[sizeof(char) * size];

		//将STR转成str
		//WideCharToMultiByte(CP_ACP, 0, STR, -1, str, size, NULL, FALSE);

		char* str = NULL;
		return str;
	}

public:
	//得到所有网卡的Mac
	static boolean GetMAC(char *pID)
	{
		NCB ncb;
		UCHAR uRetCode;
		LANA_ENUM lana_enum;
		memset(&ncb, 0, sizeof(ncb));
		ncb.ncb_command = NCBENUM;
		ncb.ncb_buffer = (unsigned char *)&lana_enum;
		ncb.ncb_length = sizeof(lana_enum);

		// 向网卡发送NCBENUM命令,以获取当前机器的网卡信息,如有多少个网卡、每张网卡的编号等
		uRetCode = Netbios(&ncb);
		if (0 != uRetCode)
		{
			strcpy(pID, "ERROR:CALL Netbios");
			return false;
		}

		printf("Ethernet Count is : %d\n\n", lana_enum.length);

		// 对每一张网卡,以其网卡编号为输入编号,获取其MAC地址
		for (int i = 0; i < lana_enum.length; ++i)
			getmac_one(pID + strlen(pID), lana_enum.lana[i]);

		return true;
	}
	static boolean GetCPUID(char *pID)
	{
		char pvendor[16];
		INT32 dwBuf[4];

		__cpuid(dwBuf, 0);
		*(INT32*)&pvendor[0] = dwBuf[1];    // ebx: 前四个字符
		*(INT32*)&pvendor[4] = dwBuf[3];    // edx: 中间四个字符
		*(INT32*)&pvendor[8] = dwBuf[2];    // ecx: 最后四个字符
		pvendor[12] = '\0';

		__cpuid(dwBuf, 0x1);
		int family = (dwBuf[0] >> 8) & 0xf;

		char pbrand[64];
		__cpuid(dwBuf, 0x80000000);
		if (dwBuf[0] < 0x80000004)
		{
			strcpy(pID, "ERROR:CAN'T GET");
			return false;
		}

		__cpuid((INT32*)&pbrand[0], 0x80000002);    // 前16个字符
		__cpuid((INT32*)&pbrand[16], 0x80000003);    // 中间16个字符
		__cpuid((INT32*)&pbrand[32], 0x80000004);    // 最后16个字符
		pbrand[48] = '\0';

		printf("get cpuid=");
		printf(pvendor);
		printf("\n");
		printf(pbrand);
		printf("\n");

		__cpuidex(dwBuf, 1, 1);
		char szTmp[33] = { NULL };
		//sprintf_s(szTmp, "%08X%08X", dwBuf[3], dwBuf[0]);
		sprintf_s(szTmp, "%08X%08X%08X%08X", dwBuf[0], dwBuf[1], dwBuf[2], dwBuf[3]);
		//sprintf(pID, "%08X%08X%08X%08X", dwBuf[0], dwBuf[1], dwBuf[2], dwBuf[3]);
		sprintf(pID, "%08X%08X%08X", dwBuf[0], dwBuf[2], dwBuf[3]);

		printf(szTmp);
		printf("\n");

		sprintf_s(szTmp, "%08X", family);
		printf(szTmp);

		return true;
	}
	static boolean GetDiskID(char *pID)
	{
		TCHAR szModelNo[64], szSerialNo[64];
		if (GetPhyDriveSerial(szModelNo, szSerialNo))
		{
			TrimStart(szSerialNo);

			//strcpy(pID, TCHAR2char(szSerialNo));

			return TRUE;
		}
		else
		{
			//strcpy(pID, TCHAR2char(szSerialNo));
			return FALSE;
		}
	}


};

