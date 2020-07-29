//ftProInterface2013Library.h
//see https://docs.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-dynamic-link-library-cpp?view=vs-2019
#pragma once
#ifdef _LIB
#define  FtPro_API 
#else
#ifdef FTTXTLIB_EXPORTS
#define FtPro_API __declspec(dllexport)
#else
#define  FtPro_API __declspec(dllimport)
#endif
#endif

extern "C" {
#include "common.h"

}


#include "ftProInterface2013TransferAreaCom.h"
#include "ftProInterface2013TransferAreaComEx2.h"
#include "ftProInterface2013JpegDecode.h"
