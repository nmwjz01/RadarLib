/*
 * Fun:数据处理，如图像切割等
 */

#include "framework.h"
#include <io.h>
 //#include <atlimage.h>

#include "Utils\\PalSET.h"
#include "Utils\\Utils.h"
#include "Utils\\RadarConst.h"

#include "Impluse\\ImpluseTrace16.h"
#include "Impluse\\ImpluseTrace32.h"
#include "Impluse\\ImpluseCor.h"
#include "Impluse\\ImpluseTime.h"
#include "Impluse\\ImpluseChannelHeader.h"
#include "Impluse\\ImpluseChannelBlob.h"
#include "Impluse\\ImpluseChannel.h"
#include "Impluse\\ImpluseSwath.h"

//#include "FSize.h"

#include "IDS\\IDSChannel.h"
#include "IDS\\IDSChannelBlob.h"
#include "IDS\\IDSChannelHeader.h"
#include "IDS\\IDSSwath.h"
#include "IDS\\IDSSwathFragment.h"
#include "IDS\\IDSTrace16.h"
#include "IDS\\IDSTrace32.h"

#include "Mala\\MalaChannel.h"
#include "Mala\\MalaChannelBlob.h"
#include "Mala\\MalaChannelHeader.h"
#include "Mala\\MalaSwath.h"
#include "Mala\\MalaTime.h"
#include "Mala\\MalaTrace16.h"
#include "Mala\\MalaTrace32.h"

#include "Transform\\TransformSegy.h"
#include "Transform\\Transform3DRadar.h"
#include "Transform\\TransformMala.h"
#include "Transform\\TransformIDS.h"
#include "Transform\\TransformDT.h"

#include "Project.h"
#include "RadarDll.h"


//全局对象
//extern CRadarDllApp theApp;

//extern char szProjectPathC[PATH_MAX_LENGTH] = { 0 };    //工程目录
//extern Project gProjectC;      //全局的工程对象



