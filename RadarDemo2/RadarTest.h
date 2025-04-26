//
// MATLAB Compiler: 7.0.1 (R2019a)
// Date: Sat Mar 29 00:55:14 2025
// Arguments:
// "-B""macro_default""-W""cpplib:RadarTest,all""-T""link:lib""-d""C:\D盘开发备?
// 輁BAK.开发和学习\AllDevelop\3D系统\开发目录\matGPR\RadarTest\RadarTest\for_te
// sting""-v""C:\D盘开发备份\BAK.开发和学习\AllDevelop\3D系统\开发目录\matGPR\Ra
// darTest\IPlus.m"
//

#ifndef __RadarTest_h
#define __RadarTest_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_RadarTest_C_API 
#define LIB_RadarTest_C_API /* No special import/export declaration */
#endif

/* GENERAL LIBRARY FUNCTIONS -- START */

extern LIB_RadarTest_C_API 
bool MW_CALL_CONV RadarTestInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_RadarTest_C_API 
bool MW_CALL_CONV RadarTestInitialize(void);

extern LIB_RadarTest_C_API 
void MW_CALL_CONV RadarTestTerminate(void);

extern LIB_RadarTest_C_API 
void MW_CALL_CONV RadarTestPrintStackTrace(void);

/* GENERAL LIBRARY FUNCTIONS -- END */

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

extern LIB_RadarTest_C_API 
bool MW_CALL_CONV mlxIPlus(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */

#ifdef __cplusplus
}
#endif


/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__MINGW64__)

#ifdef EXPORTING_RadarTest
#define PUBLIC_RadarTest_CPP_API __declspec(dllexport)
#else
#define PUBLIC_RadarTest_CPP_API __declspec(dllimport)
#endif

#define LIB_RadarTest_CPP_API PUBLIC_RadarTest_CPP_API

#else

#if !defined(LIB_RadarTest_CPP_API)
#if defined(LIB_RadarTest_C_API)
#define LIB_RadarTest_CPP_API LIB_RadarTest_C_API
#else
#define LIB_RadarTest_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_RadarTest_CPP_API void MW_CALL_CONV IPlus(int nargout, mwArray& Result, const mwArray& paramA, const mwArray& paramB);

/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */
#endif

#endif
