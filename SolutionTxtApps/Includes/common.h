//=============================================================================
//              |
// Headerfile   |   common.h
//              |   Header file describing data types
//              |
// Created      |   04.11.2008, by H.-Peter Classen
//              |
// Last Change  |   
//-----------------------------------------------------------------------------

#ifndef __COMMON_H__
// Protect against multiple file inclusion
#define __COMMON_H__

//  Logstatus
enum LogState {
	LOGERR=0, ///timing+ ERROR+ full information + new line
	WARNING=1,///WARNING+log string+ new line
	LOG = 2,///timing+log string + new line
	INFO=3,/// INFO -only log string no timing + new line
	LOGPRINT=4,/// log string + new line
	LOGFREE=5 /// only log string 
};

/// <summary>
/// 
/// </summary>
enum LogLevel {
	LvLOGERR = 0, ///Print only Errors
	LvWARNING = 1,///Print only Errors and warnings
	LvLOG = 2,///Print only Errors,  warnings and log
	LvINFO = 3,///Print only Errors,  warnings, log and info
	LvLOGPRINT = 4,
	LvLOGFREE = 5/// print everything

};
//  declaration of data types using by RoboPro (fischertechnik)

typedef char                CCHAR8;
typedef signed char         CHAR8;
typedef unsigned char       UCHAR8;

typedef unsigned char       BOOL8;      //  Boolean variable (should be TRUE or FALSE)
typedef unsigned short      BOOL16;     //  Boolean variable (should be TRUE or FALSE)
typedef unsigned long       BOOL32;     //  Boolean variable (should be TRUE or FALSE)

typedef signed char         INT8;
typedef signed short        INT16;
typedef signed int          INT32;

typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
#ifndef WIN32
typedef unsigned long      UINT32;
#endif

/*
enum FtErrors :unsigned int {

	FTLIB_ERR_SUCCESS = 0x00000000L,
	FTLIB_ERR_INVALID_PARAM = 0xE0001018L,
	FTLIB_ERR_FAILED = 0xE0001000L,
	FTLIB_ERR_THREAD_IS_RUNNING = 0xE00012A5L,
	FTLIB_ERR_THREAD_NOT_RUNNING = 0xE00012A6L,
	FTLIB_ERR_UNKNOWN = 0xEFFFFFFFL
};
*/
//constexpr unsigned int FTLIB_ERR_SUCCESS = 0x00000000L;
//constexpr unsigned int  FTLIB_ERR_NO_MEMORY                 =0xE0000100L;
//constexpr unsigned int  FTLIB_ERR_FAILED                    =0xE0001000L;
/*
constexpr unsigned int  FTLIB_ERR_TIMEOUT                   =0xE000100CL;
constexpr unsigned int  FTLIB_ERR_INVALID_PARAM             =0xE0001018L;

constexpr auto  FTLIB_ERR_SOME_DEVICES_ARE_OPEN     =0xE0001101L;
constexpr auto  FTLIB_ERR_DEVICE_IS_OPEN            =0xE0001102L;
constexpr auto  FTLIB_ERR_DEVICE_NOT_OPEN           =0xE0001103L;
constexpr auto  FTLIB_ERR_NO_SUCH_DEVICE_INSTANCE   =0xE0001104L;

constexpr auto  FTLIB_ERR_UNKNOWN_DEVICE_HANDLE     =0xE0001283L;
constexpr auto  FTLIB_ERR_LIB_IS_INITIALIZED        =0xE0001286L;
constexpr auto  FTLIB_ERR_LIB_IS_NOT_INITIALIZED    =0xE0001287L;
constexpr auto  FTLIB_ERR_THREAD_NOT_STARTABLE      =0xE00012A0L;

constexpr auto  FTLIB_ERR_THREAD_SYNCHRONIZED       =0xE00012AFL;

constexpr auto  FTLIB_ERR_TIMEOUT_TA                =0xE00012B0L;
constexpr auto  FTLIB_ERR_CREATE_EVENT              =0xE00012B1L;
constexpr auto  FTLIB_ERR_CREATE_MM_TIMER           =0xE00012B2L;

constexpr auto  FTLIB_ERR_UPLOAD_FILE_NOT_OPEN      =0xE0001400L;
constexpr auto  FTLIB_ERR_UPLOAD_FILE_READ_ERR      =0xE0001401L;
constexpr auto  FTLIB_ERR_UPLOAD_INVALID_FSIZE      =0xE0001402L;
constexpr auto  FTLIB_ERR_UPLOAD_START              =0xE0001403L;
constexpr auto  FTLIB_ERR_UPLOAD_CANCELED           =0xE0001404L;
constexpr auto  FTLIB_ERR_UPLOAD_FAILED             =0xE0001405L;
constexpr auto  FTLIB_ERR_UPLOAD_TIMEOUT            =0xE0001406L;
constexpr auto  FTLIB_ERR_UPLOAD_ACK                =0xE0001407L;
constexpr auto  FTLIB_ERR_UPLOAD_NAK                =0xE0001408L;
constexpr auto  FTLIB_ERR_UPLOAD_DONE               =0xE0001409L;
constexpr auto 	FTLIB_ERR_UPLOAD_FLASHWRITE         =0xE000140AL;
constexpr auto  FTLIB_ERR_REM_CMD_FAILED            =0xE000140BL;
constexpr auto  FTLIB_ERR_REM_CMD_NOT_SUPPORTED     =0xE000140CL;
constexpr auto  FTLIB_ERR_FWUPD_GET_FILES           =0xE000140DL;
constexpr auto  FTLIB_ERR_FWUPD_NO_FILES            =0xE000140EL;

constexpr auto  FTLIB_ERR_ACCESS_DENIED             =0xE0001905L;
constexpr auto  FTLIB_ERR_OPEN_COM                  =0xE0001906L;
constexpr auto  FTLIB_ERR_INIT_COM                  =0xE0001908L;
constexpr auto  FTLIB_ERR_INIT_COM_TIMEOUT          =0xE0001909L;

constexpr auto  FTLIB_ERR_WRONG_HOSTNAME_LEN        =0xE0002000L;

constexpr auto  FTLIB_FWUPD_UPLOAD_START            =0xE0003000L;
constexpr auto  FTLIB_FWUPD_UPLOAD_DONE             =0xE0003001L;
constexpr auto  FTLIB_FWUPD_TIMEOUT                 =0xE0003002L;
constexpr auto  FTLIB_FWUPD_FLUSH_DISK              =0xE0003003L;
constexpr auto  FTLIB_FWUPD_CLEAN_DISK              =0xE0003004L;
constexpr auto  FTLIB_FWUPD_ERR_FILE_READ           =0xE0003005L;
constexpr auto  FTLIB_FWUPD_UPLOAD_FAILED           =0xE0003006L;
constexpr auto  FTLIB_FWUPD_STARTING                =0xE0003007L;
constexpr auto  FTLIB_FWUPD_FINISHED                =0xE0003008L;
constexpr auto  FTLIB_FWUPD_REM_COMMAND             =0xE0003009L;
constexpr auto  FTLIB_FWUPD_REM_TIMEOUT             =0xE000300AL;
constexpr auto  FTLIB_FWUPD_REM_FAILED              =0xE000300BL;
constexpr auto  FTLIB_FWUPD_IZ_STEPS                =0xE000300CL;
constexpr auto  FTLIB_FWUPD_STEP                    =0xE000300DL;

constexpr auto  FTLIB_BT_INVALID_CONIDX             =0xE0004000L;
constexpr auto  FTLIB_BT_CON_NOT_EXISTS             =0xE0004001L;
constexpr auto  FTLIB_BT_CON_ACTIVE                 =0xE0004002L;
constexpr auto  FTLIB_BT_CON_INACTIVE               =0xE0004003L;
constexpr auto  FTLIB_BT_CON_WRONG_ADDR             =0xE0004004L;
constexpr auto  FTLIB_BT_CON_WAIT_BUSY              =0xE0004005L;

constexpr auto  FTLIB_I2C_INVALID_DEV_ADDR          =0xE0005000L;
constexpr auto  FTLIB_I2C_INVALID_FLAGS_ADDRMODE    =0xE0005001L;
constexpr auto  FTLIB_I2C_INVALID_FLAGS_DATAMODE    =0xE0005002L;
constexpr auto  FTLIB_I2C_INVALID_FLAGS_ERRMODE     =0xE0005003L;

constexpr auto  FTLIB_ERR_UNKNOWN                   =0xEFFFFFFFL;
*/
#endif
