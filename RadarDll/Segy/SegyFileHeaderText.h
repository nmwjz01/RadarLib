#pragma once
#include <cstring>

//EBCDIC
//3200 byte
class SegyFileHeaderText
{
public:
	SegyFileHeaderText()
	{
		memset( szC01, 0x20, 80 );
		memset( szC02, 0x20, 80 );    //lines:Trace数, lines:通道数, Sanples:采样点数
		memset( szC03, 0x20, 80 );
		memset( szC04, 0x20, 80 );
		memset( szC05, 0x20, 80 );
		memset( szC06, 0x20, 80 );
		memset( szC07, 0x20, 80 );
		memset( szC08, 0x20, 80 );
		memset( szC09, 0x20, 80 );
		memset( szC10, 0x20, 80 );
		memset( szC11, 0x20, 80 );
		memset( szC12, 0x20, 80 );
		memset( szC13, 0x20, 80 );
		memset( szC14, 0x20, 80 );
		memset( szC15, 0x20, 80 );
		memset( szC16, 0x20, 80 );
		memset( szC17, 0x20, 80 );
		memset( szC18, 0x20, 80 );
		memset( szC19, 0x20, 80 );
		memset( szC20, 0x20, 80 );
		memset( szC21, 0x20, 80 );
		memset( szC22, 0x20, 80 );
		memset( szC23, 0x20, 80 );
		memset( szC24, 0x20, 80 );
		memset( szC25, 0x20, 80 );
		memset( szC26, 0x20, 80 );
		memset( szC27, 0x20, 80 );
		memset( szC28, 0x20, 80 );
		memset( szC29, 0x20, 80 );
		memset( szC30, 0x20, 80 );
		memset( szC31, 0x20, 80 );
		memset( szC32, 0x20, 80 );
		memset( szC33, 0x20, 80 );
		memset( szC34, 0x20, 80 );
		memset( szC35, 0x20, 80 );
		memset( szC36, 0x20, 80 );
		memset( szC37, 0x20, 80 );
		memset( szC38, 0x20, 80 );
		memset( szC39, 0x20, 80 );
		memset( szC40, 0x20, 80 );
	};

	void setC01( char *szC ){ strncpy(szC01, szC, 79); szC01[79]=0x20; };
	void setC02( char *szC ){ strncpy(szC02, szC, 79); szC02[79]=0x20; };
	void setC03( char *szC ){ strncpy(szC03, szC, 79); szC03[79]=0x20; };
	void setC04( char *szC ){ strncpy(szC04, szC, 79); szC04[79]=0x20; };
	void setC05( char *szC ){ strncpy(szC05, szC, 79); szC05[79]=0x20; };
	void setC06( char *szC ){ strncpy(szC06, szC, 79); szC06[79]=0x20; };
	void setC07( char *szC ){ strncpy(szC07, szC, 79); szC07[79]=0x20; };
	void setC08( char *szC ){ strncpy(szC08, szC, 79); szC08[79]=0x20; };
	void setC09( char *szC ){ strncpy(szC09, szC, 79); szC09[79]=0x20; };
	void setC10( char *szC ){ strncpy(szC10, szC, 79); szC10[79]=0x20; };
	void setC11( char *szC ){ strncpy(szC11, szC, 79); szC11[79]=0x20; };
	void setC12( char *szC ){ strncpy(szC12, szC, 79); szC12[79]=0x20; };
	void setC13( char *szC ){ strncpy(szC13, szC, 79); szC13[79]=0x20; };
	void setC14( char *szC ){ strncpy(szC14, szC, 79); szC14[79]=0x20; };
	void setC15( char *szC ){ strncpy(szC15, szC, 79); szC15[79]=0x20; };
	void setC16( char *szC ){ strncpy(szC16, szC, 79); szC16[79]=0x20; };
	void setC17( char *szC ){ strncpy(szC17, szC, 79); szC17[79]=0x20; };
	void setC18( char *szC ){ strncpy(szC18, szC, 79); szC18[79]=0x20; };
	void setC19( char *szC ){ strncpy(szC19, szC, 79); szC19[79]=0x20; };
	void setC20( char *szC ){ strncpy(szC20, szC, 79); szC20[79]=0x20; };
	void setC21( char *szC ){ strncpy(szC21, szC, 79); szC21[79]=0x20; };
	void setC22( char *szC ){ strncpy(szC22, szC, 79); szC22[79]=0x20; };
	void setC23( char *szC ){ strncpy(szC23, szC, 79); szC23[79]=0x20; };
	void setC24( char *szC ){ strncpy(szC24, szC, 79); szC24[79]=0x20; };
	void setC25( char *szC ){ strncpy(szC25, szC, 79); szC25[79]=0x20; };
	void setC26( char *szC ){ strncpy(szC26, szC, 79); szC26[79]=0x20; };
	void setC27( char *szC ){ strncpy(szC27, szC, 79); szC27[79]=0x20; };
	void setC28( char *szC ){ strncpy(szC28, szC, 79); szC28[79]=0x20; };
	void setC29( char *szC ){ strncpy(szC29, szC, 79); szC29[79]=0x20; };
	void setC30( char *szC ){ strncpy(szC30, szC, 79); szC30[79]=0x20; };
	void setC31( char *szC ){ strncpy(szC31, szC, 79); szC31[79]=0x20; };
	void setC32( char *szC ){ strncpy(szC32, szC, 79); szC32[79]=0x20; };
	void setC33( char *szC ){ strncpy(szC33, szC, 79); szC33[79]=0x20; };
	void setC34( char *szC ){ strncpy(szC34, szC, 79); szC34[79]=0x20; };
	void setC35( char *szC ){ strncpy(szC35, szC, 79); szC35[79]=0x20; };
	void setC36( char *szC ){ strncpy(szC36, szC, 79); szC36[79]=0x20; };
	void setC37( char *szC ){ strncpy(szC37, szC, 79); szC37[79]=0x20; };
	void setC38( char *szC ){ strncpy(szC38, szC, 79); szC38[79]=0x20; };
	void setC39( char *szC ){ strncpy(szC39, szC, 79); szC39[79]=0x20; };
	void setC40( char *szC ){ strncpy(szC40, szC, 79); szC40[79]=0x20; };

	char * getC01() { return szC01; };
	char * getC02() { return szC02; };
	char * getC03() { return szC03; };
	char * getC04() { return szC04; };
	char * getC05() { return szC05; };
	char * getC06() { return szC06; };
	char * getC07() { return szC07; };
	char * getC08() { return szC08; };
	char * getC09() { return szC09; };
	char * getC10() { return szC10; };
	char * getC11() { return szC11; };
	char * getC12() { return szC12; };
	char * getC13() { return szC13; };
	char * getC14() { return szC14; };
	char * getC15() { return szC15; };
	char * getC16() { return szC16; };
	char * getC17() { return szC17; };
	char * getC18() { return szC18; };
	char * getC19() { return szC19; };
	char * getC20() { return szC20; };
	char * getC21() { return szC21; };
	char * getC22() { return szC22; };
	char * getC23() { return szC23; };
	char * getC24() { return szC24; };
	char * getC25() { return szC25; };
	char * getC26() { return szC26; };
	char * getC27() { return szC27; };
	char * getC28() { return szC28; };
	char * getC29() { return szC29; };
	char * getC30() { return szC30; };
	char * getC31() { return szC31; };
	char * getC32() { return szC32; };
	char * getC33() { return szC33; };
	char * getC34() { return szC34; };
	char * getC35() { return szC35; };
	char * getC36() { return szC36; };
	char * getC37() { return szC37; };
	char * getC38() { return szC38; };
	char * getC39() { return szC39; };
	char * getC40() { return szC40; };

	char * getBuff(){ return szC01; };
	void   setBuff( char *szBuff ) { strncpy( szC01, szBuff, 3200 ); };

private:
	char szC01[80];
	char szC02[80];
	char szC03[80];
	char szC04[80];
	char szC05[80];
	char szC06[80];
	char szC07[80];
	char szC08[80];
	char szC09[80];
	char szC10[80];
	char szC11[80];
	char szC12[80];
	char szC13[80];
	char szC14[80];
	char szC15[80];
	char szC16[80];
	char szC17[80];
	char szC18[80];
	char szC19[80];
	char szC20[80];
	char szC21[80];
	char szC22[80];
	char szC23[80];
	char szC24[80];
	char szC25[80];
	char szC26[80];
	char szC27[80];
	char szC28[80];
	char szC29[80];
	char szC30[80];
	char szC31[80];
	char szC32[80];
	char szC33[80];
	char szC34[80];
	char szC35[80];
	char szC36[80];
	char szC37[80];
	char szC38[80];
	char szC39[80];
	char szC40[81];
};

