#pragma once


#define ERROR_CODE_OTHER     -1   //系统内部其他错误
#define ERROR_CODE_SUCCESS    0

#define ERROR_PARAM             1    //参数不正确
#define ERROR_CODE_NOSWATH      2    //没有找到目标测线
#define ERROR_CODE_NOCHANNEL    3    //没有找到目标通道
#define ERROR_CODE_PATH         4    //没有目錄 或 路径错误

#define ERROR_DT_XML            5    //目录中没有DT数据的文件Stream X1.xml
#define ERROR_DT_NOCHANNEL      6    //DT数据没有通道信息
#define ERROR_DT_ERRCHANNEL     7    //DT数据没通道信息錯誤
#define ERROR_FILE_READ         8    //读取文件错误，文件格式错误

#define ERROR_NOFILE_MALA       9    //没有Mala的输入文件
#define ERROR_NOFILE_3DRADAR   10    //没有3DRadar的输入文件
#define ERROR_NOFILE_SEGY      11    //没有SEGY的输入文件


#define ERROR_FILE_TRANSFORM  100    //转化文件失败
#define ERROR_CODE_SYS     999    //系统内部错误

#define PATH_MAX_LENGTH 512

