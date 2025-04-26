#pragma once
class TransformBase
{
public:
	int CreateCor(const char *pathDst, const char *swathName, int swathIndex);
	int SaveImpluseCor(SwathCor *corInfo, const char *swathName, int swathID, const char *swathPathDst);
	int CreateGps(const char *pathDst, const char *swathName, int swathIndex);
	int CreateMrk(const char *pathDst, const char *swathName, int swathIndex);
	int CreateOrd(const char *pathDst, const char *swathName, int swathIndex);
	int CreateTime(const char *pathDst, const char *swathName, int swathIndex);
	int SaveImpluseTime(SwathTime *timeInfo, const char *swathName, int swathID, const char *swathPathDst);
	int CreateIprh(const char *pathDst, const char *swathName, int swathIndex, int iChannel);
	int SaveIprh();
	int CreateIprb(const char *pathDst, const char *swathName, int swathIndex, int iChannel);
	int SaveIprb();
	int SaveIprb2File(std::map<long, SwathChannel*> * lstData, const char *swathName, int swathID, const char *swathPathDst);

private:

protected:
	SwathChannelBlob   oIprb;
	SwathChannelHeader oIprh;
	char szIprh[512] = { 0 };
	char szIprb[512] = { 0 };
};

