/*
 * Fun:����Matlab���㷨���ͼ�����ݽ��д���
 */

#include "framework.h"
#include <io.h>
//#include <atlimage.h>

#include "RadarMath.h"
#include "Utils\\RadarConst.h"

//����Ŀ¼
extern char szProjectPathC[];

/*
Fun:RadarMath���ʼ��
*/
extern "C" __declspec(dllexport) bool RadarMathInit()
{
	//AfxMessageBox(_T("init RadarMath Start !!!"));

	bool bInit = RadarMathInitialize();
	if (!bInit)
	{
		//AfxMessageBox(_T("init RadarMath NOK !!!"));
		return false;
	}
	else
	{
		//AfxMessageBox(_T("init RadarMath OK !!!"));
		return true;
	}
}

/*
Fun:RadarMath���ʼ��
*/
extern "C" __declspec(dllexport) bool RadarMathInitEx(const char * projectPath)
{
	strcpy(szProjectPathC, projectPath);

	bool bInit = 0;

	bInit = RadarMathInitialize();
	if (!bInit)
	{
		//AfxMessageBox(_T("init RadarMath NOK !!!"));
		return false;
	}
	else
	{
		//AfxMessageBox(_T("init RadarMath OK !!!"));
		return true;
	}
}

/*
Fun:RadarMath��ȥ��ʼ��
*/
extern "C" __declspec(dllexport) void RadarMathUninit()
{
	RadarMathTerminate();
}

/*
Fun:����ָ������ͨ����ֱ�ﲨͼ��-----��������ײ����Matlab�������㣬�ú��������ϳ������ΪSigPositionPicByMatGPR
Param:swathName      ָ���Ĳ�����
	  channelID      ͨ����
	  sigPositionPic Ŀ��ֱ�ﲨͼ���ļ�
*/
extern "C" __declspec(dllexport) int SigPositionPic(const char * swathName, const int channelID, const char * sigPositionPic)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	length = (int)wcslen((wchar_t*)sigPositionPic);
	if (length <= 0)
		return ERROR_PARAM;

	char szSigPositionPic[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)sigPositionPic, length, szSigPositionPic, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//char fileNameData[512]   = { "RadarData_001_A01.iprb" };
	//char fileNameHeader[512] = { "RadarData_001_A01.rad" };
	//char path[512] = { "C:\\3D������Ϣϵͳ\\�����Ź���\\2\\0928test_2\\RadarData\\" };

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//AfxMessageBox(fileNameData);
	//AfxMessageBox(fileNameHeader);
	//AfxMessageBox(path);

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	//MessageBox(NULL, szOut, szOut, 0);

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray STR_FILE_PIC(szSigPositionPic);

	//����ֱ�ﲨͼ��
	ISigPositionPic(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, STR_FILE_PIC);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:��ȡָ������ͨ����ֱ�ﲨλ��----�ײ����Matlabʵ��
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
*/
extern "C" __declspec(dllexport) int SigPositionNum(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//char fileNameData[512]   = { "RadarData_001_A01.iprb" };
	//char fileNameHeader[512] = { "RadarData_001_A01.rad" };
	//char path[512] = { "C:\\3D������Ϣϵͳ\\�����Ź���\\2\\0928test_2\\RadarData\\" };

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray Result;
	int nargout = 1;
	//��ȡֱ�ﲨλ��
	ISigPositionNum(nargout, Result, STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH);

	return Result.Get(1, 1);
}

/*
Fun:��ȡָ������ͨ����ֱ�ﲨλ��
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  direct    ֱ�ﲨ���� 1���ұ�ֱ�ﲨ�Ĳ��壻 -1�����ֱ�ﲨ�Ĳ��壻 0������ֱ�ﲨ�Ĳ���
*/
extern "C" __declspec(dllexport) int SigPositionNumEx(const char * swathName, const int channelID, int direct)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	//char fileNameData[512]   = { "RadarData_001_A01.iprb" };
	//char fileNameHeader[512] = { "RadarData_001_A01.rad" };
	//char path[512] = { "C:\\3D������Ϣϵͳ\\�����Ź���\\2\\0928test_2\\RadarData\\" };

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_DIRECT(direct);
	mwArray Result;
	int nargout = 1;
	//����ֱ�ﲨͼ��
	ISigPositionNumEx(nargout, Result, STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_DIRECT);

	return Result.Get(1, 1);

}

/*
Fun:�г�ָ����ֱ�ﲨͼ��
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  iZero     ֱ�ﲨλ��
*/
extern "C" __declspec(dllexport) int SigPositionCut(const char * swathName, const int channelID, int iZero)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_DEEP(iZero);
	//�г�ֱ�ﲨͼ��
	ISigPositionCut(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_DEEP);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:ȥ����������
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  offerset  ��������������ƫ��
*/
extern "C" __declspec(dllexport) int RemoveBackgr(const char * swathName, const int channelID )
//extern "C" __declspec(dllexport) int RemoveBackgr(const char * swathName, const int channelID, const int offset)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };
	//int offerset   = 0;

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_OFFSET(5);

	//ȥ��������
	IRmBackGr(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_OFFSET);
	//IRmBackGr(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:ȥ����������
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  scaleBase ����ȥ��ʱ����ֵԽ���ϲ�����ȥ��������ԽС�����Impluse 450M�״ȡֵ0-3��0��ʾ�������еĴ���̶ȡ�
*/
extern "C" __declspec(dllexport) int RemoveBackgr450M(const char * swathName, const int channelID, int scaleBase)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_SCALE_BASE(scaleBase);

	//ȥ��������
	IRmBackGr450M(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_SCALE_BASE);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:ȥ��ֱ������
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
*/
extern "C" __declspec(dllexport) int RemoveDC(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);

	//ȥֱ������
	IRemoveDC(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:������������Զ�����
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
*/
extern "C" __declspec(dllexport) int GainInvDecay(const char * swathName, const int channelID)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);

	//���������˥�p���Զ�����
	IGainInvDecay(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:������������Զ�����
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  curve     ������ͣ�Ĭ�JΪ1
	  order     ������Խ��ͼ��Խ���������\���ٶ�Խ��(ȡֵ����1-10)
*/
extern "C" __declspec(dllexport) int GainInvDecayEx(const char * swathName, const int channelID, int curve, int order)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);

	mwArray INT_CURVE(curve);
	mwArray INT_ORDER(order);

	//���������˥�p���Զ�����
	IGainInvDecayEx(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_CURVE, INT_ORDER);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:��ͼ���M�Ў�ͨ�˲�(������˹�˲���)
Param:swathName  ָ���Ĳ�����
	  channelID  ͨ����
	  freqStart  ͨ���_ʼ�l��  -- ��С
	  freqEnd    ͨ���Y���l��  -- ���
Return: �ɹ���0�� ʧ�ܣ�������
*/
extern "C" __declspec(dllexport) int FilterButterworth(const char * swathName, const int channelID, int freqStart, int freqEnd)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	//MessageBox(NULL, szOut, szOut, 0);
	//return 0;

	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);

	mwArray INT_FreqStart(freqStart);
	mwArray INT_FreqEnd(freqEnd);


	//char szOut[512] = { 0 };
	//sprintf(szOut, "IPRB:%s; IPRH:%s; PATH:%s; freqStart:%d; freqEnd:%d; ", fileData, fileHeader, path, freqStart, freqEnd );
	//MessageBox(NULL, szOut, szOut, 0);
	//return true;

	//��ͨ�˲�
	IButterworthPassBand(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_FreqStart, INT_FreqEnd);

	return ERROR_CODE_SUCCESS;
}

/*
Fun:��ͼ���M��KL�任
Param:swathName ָ���Ĳ�����
	  channelID ͨ����
	  tr        �任ֵ1---�����ֵԽСЧ��Խ����
Return: �ɹ���0�� ʧ�ܣ�������
*/
extern "C" __declspec(dllexport) int TransferKL(const char * swathName, const int channelID, int tr)
{
	int length = (int)wcslen((wchar_t*)swathName);
	if (length <= 0)
		return ERROR_PARAM;

	char szSwathName[512] = { 0 };
	//���潫Java���ݵ�Unicode�ַ���ת��ΪASCII�ַ���
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(wchar_t*)swathName, length, szSwathName, (length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return ERROR_PARAM;

	char fileNameData[512] = { 0 };
	char fileNameHeader[512] = { 0 };
	char path[512] = { 0 };

	sprintf(fileNameData, "%s_A%02d.iprb", szSwathName, channelID);
	sprintf(fileNameHeader, "%s_A%02d.iprh", szSwathName, channelID);
	sprintf(path, "%s\\RadarData\\", szProjectPathC);

	//char szOut[512] = { 0 };
	//sprintf(szOut, "fileNameData:%s; fileNameHeader:%s; path:%s", fileNameData, fileNameHeader, path);
	//MessageBox(NULL, szOut, szOut, 0);

	//ͼ��KL�任
	//bool bReturn = theApp.TransferKL(fileNameData, fileNameHeader, path, tr);
	//�������
	mwArray STR_FILE_DATA(fileNameData);
	mwArray STR_FILE_HEAD(fileNameHeader);
	mwArray STR_FILE_PATH(path);
	mwArray INT_TR(tr);

	//char szOut[512] = { 0 };
	//sprintf(szOut, "IPRB:%s; IPRH:%s; PATH:%s; freqSample:%d; freqStart:%d; freqEnd:%d; ", fileData, fileHeader, path, freq, freqStart, freqEnd );
	//MessageBox(NULL, szOut, szOut, 0);

	//KL�任
	IKarhunenLoeve(STR_FILE_DATA, STR_FILE_HEAD, STR_FILE_PATH, INT_TR);

	return ERROR_CODE_SUCCESS;
}

