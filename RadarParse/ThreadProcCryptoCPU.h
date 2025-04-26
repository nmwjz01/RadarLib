#pragma once

#include <thread>
#include <list>
#include <mutex>

#include "WalletResultItem.h"
#include "WalletResult.h"
#include "WalletTask.h"

#include "../CryptoC/find-wallet-bin.h"
#include "../CryptoC/ecmult_gen_impl.h"
#include "../CryptoC/rmd160.h"
#include "../CryptoC/sha256.h"

//ҵ�������߳�
//���ڼ������֤һ��˽Կ
class ThreadProcCryptoCPU
{
public:
	ThreadProcCryptoCPU()
	{
		//��ʼ����Բ����
		pContext = (secp256k1_ecmult_gen_context*)malloc(sizeof(secp256k1_ecmult_gen_context));
		secp256k1_ecmult_gen_context_build(pContext);

		//���ؽ������
		pWalletResult = new WalletResult();
		//�����������
		pWalletTask = new WalletTask;
	};
	~ThreadProcCryptoCPU()
	{
		free(pContext);

		//�ͷŽ������
		delete pWalletResult;
		//�ͷ��������
		delete pWalletTask;
	};

	//�����߳�
	void start()
	{
		//���ÿ�ʼ��־
		iFlagExit = 1;


		//���������߳�
		oMainWorkThd = std::thread(&ThreadProcCryptoCPU::run, this);
		oMainWorkThd.detach();
	};
	//ֹͣ�߳�
	void stop()
	{
		//�����˳���־
		iFlagExit = 0;
	};

	//����Ǯ������
	//������
	//  1��pWalletDataǮ�����ݣ�
	//  2��iWalletCountǮ������
	void setWalletData(unsigned char *pWalletData, unsigned int iWalletCount)
	{
		this->pWalletData  = pWalletData;
		this->iWalletCount = iWalletCount;

		//����Ǯ������
		wallet_bin_init(pWalletData, iWalletCount);
	};
	//���ҵ���̵߳�����Data���������
	//������
	//  1��JobID��
	//  2������ID��
	//  3����ʼ˽Կ��
	//  4��ÿ�̵߳ļ�������
	void addTaskData(unsigned int iJobID, unsigned int iTaskID, unsigned char *szPrivateKey, unsigned int iCalcCount)
	{
		WalletTaskItem * pTaskItem = new WalletTaskItem();
		pTaskItem->setJobID(iJobID);
		pTaskItem->setTaskID(iTaskID);
		pTaskItem->setPrivateStart(szPrivateKey);
		pTaskItem->setPrivateCount(iCalcCount);

		pWalletTask->pushItem(pTaskItem);
	};

	//�ж��߳��Ƿ�������
	bool isRun()
	{
		return (1 == iFlagExit);
	};

	//��ȡ����б�
	WalletResult * getResult()
	{
		return pWalletResult;
	};
	//��ȡ����
	WalletTask * getTask()
	{
		return pWalletTask;
	};

	//�����̵߳�Handle
	std::thread::native_handle_type getHandle()
	{
		return oMainWorkThd.native_handle();
	};

private:
	//������ͷ�Ĺ�Կ
	unsigned int genPublicKeyStart(unsigned char *pPrivateKey, unsigned char *pPublicKey, unsigned int iThreadCount, unsigned int iCalcCount, unsigned char *pPrivateStart)
	{
		//ʹ��˽Կ�����㹫ԿΪÿ��GPU�̼߳��㹫Կ
		for (unsigned int i = 0; i < iThreadCount; i++)
		{
			//Ŀ��˽Կ
			ProcPrivateAddInt(pPrivateKey, pPrivateStart, iCalcCount * i);

			//Ŀ��˽Կ��Ӧ�Ĺ�Կ
			secp256k1_ecmult_gen(pContext, pPublicKey, pPrivateKey);

			//Ŀ��˽Կ�͹�Կ�Ĵ洢λ��
			pPrivateKey = pPrivateKey + 32;
			pPublicKey  = pPublicKey  + 66;
		}
		return 0;
	};
	//���������Ĺ�Կ
	unsigned int genPublicKeyOther(unsigned char *pPrivateKey, unsigned char *pPublicKey, unsigned int iThreadCount, unsigned int iCalcCount, unsigned char * pOutput, unsigned int &iResultCount, unsigned int iJob, unsigned int iTask, unsigned int *iCurrent)
	{
		unsigned char * szPublicKey  = pPublicKey;
		//===================================//
		uint32_t iPublic0 = ((szPublicKey[31] << 24) & 0xFF000000) | ((szPublicKey[30] << 16) & 0x00FF0000) | ((szPublicKey[29] << 8) & 0x0000FF00) | (szPublicKey[28] & 0xFF);
		uint32_t iPublic1 = ((szPublicKey[27] << 24) & 0xFF000000) | ((szPublicKey[26] << 16) & 0x00FF0000) | ((szPublicKey[25] << 8) & 0x0000FF00) | (szPublicKey[24] & 0xFF);
		uint32_t iPublic2 = ((szPublicKey[23] << 24) & 0xFF000000) | ((szPublicKey[22] << 16) & 0x00FF0000) | ((szPublicKey[21] << 8) & 0x0000FF00) | (szPublicKey[20] & 0xFF);
		uint32_t iPublic3 = ((szPublicKey[19] << 24) & 0xFF000000) | ((szPublicKey[18] << 16) & 0x00FF0000) | ((szPublicKey[17] << 8) & 0x0000FF00) | (szPublicKey[16] & 0xFF);
		uint32_t iPublic4 = ((szPublicKey[15] << 24) & 0xFF000000) | ((szPublicKey[14] << 16) & 0x00FF0000) | ((szPublicKey[13] << 8) & 0x0000FF00) | (szPublicKey[12] & 0xFF);
		uint32_t iPublic5 = ((szPublicKey[11] << 24) & 0xFF000000) | ((szPublicKey[10] << 16) & 0x00FF0000) | ((szPublicKey[ 9] << 8) & 0x0000FF00) | (szPublicKey[ 8] & 0xFF);
		uint32_t iPublic6 = ((szPublicKey[ 7] << 24) & 0xFF000000) | ((szPublicKey[ 6] << 16) & 0x00FF0000) | ((szPublicKey[ 5] << 8) & 0x0000FF00) | (szPublicKey[ 4] & 0xFF);
		uint32_t iPublic7 = ((szPublicKey[ 3] << 24) & 0xFF000000) | ((szPublicKey[ 2] << 16) & 0x00FF0000) | ((szPublicKey[ 1] << 8) & 0x0000FF00) | (szPublicKey[ 0] & 0xFF);
		uint32_t iPublic8 = ((szPublicKey[63] << 24) & 0xFF000000) | ((szPublicKey[62] << 16) & 0x00FF0000) | ((szPublicKey[61] << 8) & 0x0000FF00) | (szPublicKey[60] & 0xFF);
		uint32_t iPublic9 = ((szPublicKey[59] << 24) & 0xFF000000) | ((szPublicKey[58] << 16) & 0x00FF0000) | ((szPublicKey[57] << 8) & 0x0000FF00) | (szPublicKey[56] & 0xFF);
		uint32_t iPublica = ((szPublicKey[55] << 24) & 0xFF000000) | ((szPublicKey[54] << 16) & 0x00FF0000) | ((szPublicKey[53] << 8) & 0x0000FF00) | (szPublicKey[52] & 0xFF);
		uint32_t iPublicb = ((szPublicKey[51] << 24) & 0xFF000000) | ((szPublicKey[50] << 16) & 0x00FF0000) | ((szPublicKey[49] << 8) & 0x0000FF00) | (szPublicKey[48] & 0xFF);
		uint32_t iPublicc = ((szPublicKey[47] << 24) & 0xFF000000) | ((szPublicKey[46] << 16) & 0x00FF0000) | ((szPublicKey[45] << 8) & 0x0000FF00) | (szPublicKey[44] & 0xFF);
		uint32_t iPublicd = ((szPublicKey[43] << 24) & 0xFF000000) | ((szPublicKey[42] << 16) & 0x00FF0000) | ((szPublicKey[41] << 8) & 0x0000FF00) | (szPublicKey[40] & 0xFF);
		uint32_t iPublice = ((szPublicKey[39] << 24) & 0xFF000000) | ((szPublicKey[38] << 16) & 0x00FF0000) | ((szPublicKey[37] << 8) & 0x0000FF00) | (szPublicKey[36] & 0xFF);
		uint32_t iPublicf = ((szPublicKey[35] << 24) & 0xFF000000) | ((szPublicKey[34] << 16) & 0x00FF0000) | ((szPublicKey[33] << 8) & 0x0000FF00) | (szPublicKey[32] & 0xFF);
		//��Կ
		secp256k1_gej oPublicKey;
		//��ȡ��һ����Կ
		secp256k1_ge secp256k1_Public = SECP256K1_GE_CONST(iPublic0, iPublic1, iPublic2, iPublic3, iPublic4, iPublic5, iPublic6, iPublic7, iPublic8, iPublic9, iPublica, iPublicb, iPublicc, iPublicd, iPublice, iPublicf);
		secp256k1_gej_set_ge(&oPublicKey, &secp256k1_Public);

		//����SHA256���ܵĹ�Կ�ַ���
		unsigned char szPublicForSha256[66] = { 0 };

		//sha256�����
		unsigned char szSHA256[32] = { 0 };
		//RMD160�����
		unsigned char szRMD160[20] = { 0 };

		unsigned int *i = iCurrent;
		//ѭ����ȡÿһ����Կ
		for (; (*i) < iCalcCount; (*i)++)
		{
			if (!iFlagExit)
				break;

			//��Ŀ�깫Կת��Ϊ�ַ���
			secp256k1_ge ge;
			secp256k1_ge_set_gej(&ge, &oPublicKey);
			//��Կ�ַ���
			//secp256k1_ge_to_storage((secp256k1_ge_storage *)(szPublicForSha256 + 1), &ge);
			secp256k1_ge_to_buff( szPublicForSha256 + 1, &ge);
			//��ͷ�̶�Ϊ04����ʶ��ѹ����ʽ
			szPublicForSha256[0] = 0x04;

			memset(szSHA256, 0, 32);
			//���㹫Կ��SHA265
			sha256(szPublicForSha256, 65, szSHA256);

			//����RMD160
			rmd160(szRMD160, szSHA256);

			//���Ҽ������Ƿ�OK
			int bResult = wallet_bin_find(szRMD160, 0, iWalletCount);
			if (0 == bResult)
			//if( ( 0 == ( (*i) % 0x100 ) ) && (iResultCount < 10 ) )//������
			{
				//�ҵ���д��λ��
				unsigned char szDstPrivate[32] = { 0 };
				//��ԭ��˽Կ�Ļ����ϣ����ƫ��ֵ��Ȼ��д��Ŀ���ַ
				ProcPrivateAddInt(szDstPrivate, pPrivateKey, *i);

				memcpy(pOutput, szDstPrivate, 32);
				pOutput = pOutput + 32;

				iResultCount++;
			}

			//ʹ�õ�ǰ��Կ��������һ����Կ
			secp256k1_gej_add_ge(&oPublicKey, &oPublicKey, &secp256k1_ge_const_g);
		}

		return 0;
	}
	//����˽Կ �� 32λ�������
	int ProcPrivateAddInt(unsigned char *pPrivateDest, unsigned char * szPrivateKey, unsigned int iNum)
	{
		unsigned char iFirst = iNum % 0x100; iNum = iNum / 0x100;
		unsigned char iSecond = iNum % 0x100; iNum = iNum / 0x100;
		unsigned char iThird = iNum % 0x100; iNum = iNum / 0x100;
		unsigned char iFourth = iNum % 0x100; iNum = iNum / 0x100;

		//�Ȱ�ȫ�����ݷŵ�Ŀ��ռ�
		memcpy(pPrivateDest, szPrivateKey, 32);

		//ǰ28λԭ������
		int iTmp = szPrivateKey[31] + iFirst; pPrivateDest[31] = szPrivateKey[31] + iFirst;
		if (iTmp >= 0x100)
		{
			iTmp = szPrivateKey[30] + iSecond + 1; pPrivateDest[30] = szPrivateKey[30] + iSecond + 1;
		}
		else
		{
			iTmp = szPrivateKey[30] + iSecond; pPrivateDest[30] = szPrivateKey[30] + iSecond;
		}

		if (iTmp >= 0x100)
		{
			iTmp = szPrivateKey[29] + iThird + 1; pPrivateDest[29] = szPrivateKey[29] + iThird + 1;
		}
		else
		{
			iTmp = szPrivateKey[29] + iThird; pPrivateDest[29] = szPrivateKey[29] + iThird;
		}

		if (iTmp >= 0x100)
		{
			iTmp = szPrivateKey[28] + iFourth + 1; pPrivateDest[28] = szPrivateKey[28] + iFourth + 1;
		}
		else
		{
			iTmp = szPrivateKey[28] + iFourth; pPrivateDest[28] = szPrivateKey[28] + iFourth;
		}

		if (iTmp >= 0x100)
		{
			iTmp = szPrivateKey[27] + 1; szPrivateKey[27] = szPrivateKey[27] + 1;
		}

		if (iTmp >= 0x100)
		{
			iTmp = szPrivateKey[26] + 1; szPrivateKey[26] = szPrivateKey[26] + 1;
		}
		return iTmp;
	};

private:
	//�ͷ�ÿһ���Ѿ���Ϊɾ��������
	static void ReleaseFinish(WalletTask * pWalletTask)
	{
		//�������
		WalletTaskItem * pTaskItem = NULL;
		//��ȡ��һ��Item
		pTaskItem = pWalletTask->getItemBegin();

		//ѭ���ͷ�ÿһ���Ѿ���Ϊɾ��������
		while (true)
		{
			//����Ҳ�����������ȥ
			if (NULL == pTaskItem)
			{
				break;
			}

			//����Ѿ�����Ϊɾ��״̬����ɾ����
			if (3 == pTaskItem->getState())
			{
				//�������嵥ɾ��
				pWalletTask->popItem(pTaskItem);
				//�ͷſռ�
				delete pTaskItem;
				pTaskItem = NULL;
			}

			//��ȡ��һ������
			pTaskItem = pWalletTask->getItemNext();
		}
	}
	//�߳�������
	static void run(ThreadProcCryptoCPU * pThread)
	{
		if (NULL == pThread)
			return;

		//�������
		WalletTask * pWalletTask = pThread->pWalletTask;
		WalletTaskItem * pTaskItem = NULL;

		while (pThread->iFlagExit)
		{
			//���Һ��ͷ��Ѿ�ɾ��������
			ReleaseFinish(pWalletTask);

			//Ѱ��δ���������Item
			pTaskItem = pWalletTask->getItemPending();
			//����Ҳ������͵�һ������
			if (!pTaskItem)
			{
				Sleep(100);
				continue;
			}

			//����Ϊ���ڴ���״̬
			pTaskItem->setState(1);

			//�����񽻸�������������
			pThread->proc_task(pTaskItem);

			//����Ϊ���״̬
			pTaskItem->setState(2);
		}
		//�˳�״̬
		pThread->iFlagExit = 0;
	};
	void proc_task(WalletTaskItem * pTaskItem)
	{
		//��ǰ����λ��
		unsigned int * iCurrent = pTaskItem->getCurrent();

		//��ʼ˽Կ
		unsigned char szPrivateStart[33] = { 0 };
		memcpy(szPrivateStart, pTaskItem->getPrivateStart(), 32);
		//JobID
		unsigned int iJobID = pTaskItem->getJobID();
		//TaskID
		unsigned int iTaskID = pTaskItem->getTaskID();
		//˽Կ����
		unsigned int iPrivateCount = pTaskItem->getPrivateCount();

		//����ÿ���׶η��صĽ��
		unsigned int iResult = 0;

		//������߳�����
		unsigned int iThreadCount = 1;
		//����˽Կ�͹�Կ�Ŀռ� //CPU�̣߳��ڲ�����ֻ��1��
		unsigned char * pPrivateKey = (unsigned char *)malloc(32);
		unsigned char * pPublicKey  = (unsigned char *)malloc(66);

		//���ؽ���Լ����ؽ��������ÿ���߳����128������
		unsigned int iResultCount = 0;
		unsigned char * pOutput = (unsigned char *)malloc(32 * iThreadCount * 128);

		//���㿪ʼ��Կ����
		iResult = genPublicKeyStart(pPrivateKey, pPublicKey, iThreadCount, iPrivateCount, szPrivateStart);

		//�������Ϊ0
		iResultCount = 0;

		//����GPU����ʣ��ļ�������
		iResult = genPublicKeyOther(pPrivateKey, pPublicKey, iThreadCount, iPrivateCount, pOutput, iResultCount, iJobID, iTaskID, iCurrent);

		//��ȡÿ�����ؽ��
		for (unsigned int i = 0; i < iResultCount; i++)
		{
			WalletResultItem *pItem = new WalletResultItem();
			pItem->setPrivateKey(pOutput + i * 32);
			pItem->setJobID(iJobID);
			pItem->setTaskID(iTaskID);
			pWalletResult->pushItem(pItem);
		}

		//�ͷſռ�
		free(pPrivateKey);
		free(pPublicKey);
		free(pOutput);
	};

private:
	//�߳����к��˳���־: 0,�˳���1,����
	unsigned int iFlagExit   = 0;

	//���߳̾��
	std::thread oMainWorkThd;
	//������
	WalletResult * pWalletResult = NULL;
	//��������
	WalletTask * pWalletTask = NULL;

	//Ǯ���ļ�����
	unsigned char *pWalletData = NULL;
	//Ǯ��������Ǯ��������
	unsigned int iWalletCount  = 0;

	//��Բ���߼������
	secp256k1_ecmult_gen_context * pContext = NULL;
};
