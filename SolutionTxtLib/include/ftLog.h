//=============================================================================
//
//	Headerfile	|	ftLog.h
//
//	Created		|	08.04.2008, by H.-Peter Classen
//
//	Last Change	|  2020-07-20 {CvL] addapted to the FtTxtLib
//-----------------------------------------------------------------------------
#ifndef __FTLOG_H__
#define __FTLOG_H__


//  defines for Send and Receive
#define SE  0
#define EM  1

extern "C" {
#include "common.h"

}


 class ftLog{
//  function declaration
//-----------------------------------------------------------------------------
 private:

	 static void OpenLogFile(const char* path);
	 static void LogPrint(LogState state, const char* logstr);
	 static void  DataLogging(BYTE* buff, LONGLONG offs, LONGLONG len);

static void    LogStatistic(void);



 public:
/// <summary>
/// Activate the logging to a file
/// </summary>
/// <param name="logMapName">the patch for the log files</param>
/// <param name="logLevel">The format of the login</param>
static void    StartLogging(const char* logMapName=".\\", LogLevel logLevel= LogLevel::LvLOGERR);
/// <summary>
/// Close the logfile
/// </summary>

static void    CloseLogFile();

/// <summary>
/// Logging dependent by the logging level, LogState=LOG
/// </summary>
/// <param name="level"> level and higher will not be printed</param>
/// <param name="format">text string</param>
/// <param name="">list of vaiables</param>
static void    LogLev(LogLevel level, const char* format, ...);
/// <summary>
/// Logging with different formats
/// </summary>
/// <param name="state"></param>
/// <param name="format"></param>
/// <param name=""></param>
static void    Log(LogState state, const char* format, ...);
static void    LogTxData(BYTE *, int, int);
//void    LogRxData(BYTE *, int, int);
static void    LogRxData(BYTE *, LONGLONG, LONGLONG);
static void    LogBuff(BYTE *, int, int);


};



#endif
