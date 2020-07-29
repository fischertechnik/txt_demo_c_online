///////////////////////////////////////////////////////////////////////////////
//
// File:    ftProInterface2013TransferAreaCom.h
//
// Project: ftPro - fischertechnik Control Graphical Programming System
//
// Module:  Transfer area based communication protocol with interface 2013
//
// Author:  Michael Sögtrop
//
///////////////////////////////////////////////////////////////////////////////
//
// Urheberrecht im Sinne des Urheberrechtsgesetzes bei
//
// Michael Sögtrop
// Germany
//
// Copyright (C) 2015
//
///////////////////////////////////////////////////////////////////////////////
//
// TO DO:
//
///////////////////////////////////////////////////////////////////////////////
// 
//Changes: 2020 - 06 - 18 C.van Leeuwen, Copyright (C) 2020
//          It is now working for firmware 4.6.6. and 4.7.0 pre-release.
//          Reduce the size of the TA: max=2 TXT master + TXT slave01,
//          Add check in destructor to avoid double EndTransver
// Changes: 2020 - 06 - 24 C.van Leeuwen, Copyright (C) 2020
//          Choise for Simple or Compressed mode is now a setting
//          void SetTransferMode(bool Compressed);
//          Add class ftIF2013TransferAreaComHandlerEx2
// Changes: 2020 - 06 - 28 C.van Leeuwen, Copyright (C) 2020
//          Add TA communication Thread in class ftIF2013TransferAreaComHandlerEx2
//          int ftxStartTransferArea();
//          int ftxStopTransferArea();
//          bool TaComThreadIsRunning();
///////////////////////////////////////////////////////////////////////////////
// Usage details for module ftProInterface2013TransferAreaCom
//
// ===== Thread safety =====
// 
// The transfer class is generally not thread safe. In multi threaded applications
// It is recommended to run the data transfer in a separate thread.
// See TaComThread in class ftIF2013TransferAreaComHandlerEx2
//
// There is one exception: GetCameraFrameJpeg uses a separate socket and can be
// called from a separate thread. So if the camera is used, it makes sense to
// decouple the camera and I/O transfer by using separate threads. If I/O and
// camera transfers are done from one thread, the I/O will be slow.
//
// ATTENTION: Start/StopCamera must be called from the main thread because they
// start the camera server by communicating over the main socket.
//
// ===== JPEG images don't have EOI and other camera bugs =====
//
// The ft camera has a bug which has the effect that returned JPEG frames don't have
// an EOI marker. An example function for decoding such frames using libjpeg is in
// ftProInterface2012DecodeJpeg.
// The camera also has a few other bugs, e.g. it seems to return 1280x720 JPEG frames
// If 1280x960 is requested, although it lists 1280x960 as supported.
//
///////////////////////////////////////////////////////////////////////////////


#include <winsock2.h>
#include <future>

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
#include "FtShmemTxt.h"

}
using namespace std;
using namespace fischertechnik::txt::ids;

// Double inclusion protection 
#if(!defined(ftProInterface2013TransferAreaComEx2_H))
#define ftProInterface2013TransferAreaComEx2_H

//******************************************************************************
//*
//* Class for handling transfer area based communication over a TCP/IP
//* connection to a 2013 interface
//*
//******************************************************************************
namespace fischertechnik {
	namespace txt {
		/// <summary>
		/// TXT remote
		/// </summary>
		namespace remote {
			/// <summary>
			/// TXT remote API's
			/// </summary>
			namespace api {
				typedef struct _mask_cb {
					// _mask_cb():uniMasks= { 0 } {  };
					BYTE      uniMasks[SHM_IF_CNT] = { 0 };
					BYTE      cnt_inMasks[SHM_IF_CNT] = { 0 };
					BYTE      counterMasks[SHM_IF_CNT] = { 0 };
				} TMASK_CB;// [2013-06-01 cvl]
				//  TransferArea of ftMscLib (Master IF, 8 Slave IF) 

				typedef struct _if08_transfer {

					FISH_X1_TRANSFER	ftxTransferArea[IF_TXT_MAX];

				} IF08_TRANSFER;

				/**************************************************************************************************/
				typedef void __stdcall tCBSlaveState(DWORD, DWORD);
				typedef tCBSlaveState* FPCBSlaveState;

				typedef void __stdcall tCBCounterResetted(DWORD, DWORD);
				typedef tCBCounterResetted* FPCBCounterResetted;

				typedef void __stdcall tCBMotoExReached(DWORD, DWORD);
				typedef tCBMotoExReached* FPCBMotorEx;

				typedef void __stdcall tCBCounterResetted2(DWORD, DWORD, void*);//[2013-06-01 cvl]
				typedef tCBCounterResetted2* FPCBCounterResetted2;
				typedef void __stdcall tCBMotoExReached2(DWORD, DWORD, void*);//[2013-06-01 cvl]
				typedef tCBMotoExReached2* FPCBMotorEx2;//[2013-06-01 cvl]
				typedef void __stdcall tCBUniChanged(DWORD, DWORD, INT16, BOOL16, UINT8, BOOL8, void*);//[2013-06-10 cvl]
				typedef tCBUniChanged* FPCBUniChanged;//[2013-06-01 cvl]
				typedef void __stdcall tCBCntInChanged(DWORD, DWORD, BYTE, void*);//[2013-06-01 cvl]
				typedef tCBCntInChanged* FPCBCntInChanged;//[2013-06-01 cvl]
				typedef void __stdcall tCBCounterChanged(DWORD, DWORD, INT16, UINT8, void*);//[2013-06-01 cvl]
				typedef tCBCounterChanged* FPCBCounterChanged;//[2013-06-01 cvl]
				/**************************************************************************************************/

				/**************************************************************************************************/

				class FtPro_API ftIF2013TransferAreaComHandlerEx2 : public ftIF2013TransferAreaComHandlerEx1
				{
				protected:

				private:
					/* Thread which take care of the communication between the TXT and the remote TA
					*/
					void thread_TAcommunication(std::future<void> futureObj);
					/* part of the communication thread
					*/

					std::promise<void>  exitSignal;// Create a std::promise object
					std::future<void>  futureObj;// = exitSignal.get_future();
					std::thread   thread1 = thread();
					ftIF2013TransferAreaComHandlerEx2(FISH_X1_TRANSFER* transferarea, int nAreas = 1, const char* name = "192.168.7.2", const char* port = "65000", const char* logMapName = ".\\", LogLevel logLevel = LogLevel::LvLOGERR);

				public:
					/// <summary>
					/// 3.0A
					/// </summary>
					/// <param name="nAreas"></param>
					/// <param name="name"></param>
					/// <param name="port"></param>
					/// <param name="logMapName"></param>
					/// <param name="logLevel"></param>
					/// <returns></returns>
					ftIF2013TransferAreaComHandlerEx2(int nAreas = 1, const char* name = "192.168.7.2", const char* port = "65000", const char* logMapName = ".\\", LogLevel logLevel = LogLevel::LvLOGERR);

					/// <summary>
						/// 3.0B
						/// </summary>
					~ftIF2013TransferAreaComHandlerEx2();
					/*********************************************************************************/
					/* Communication thread */
					 /*********************************************************************************/

					 /// <summary>
					 /// 3.4
					 /// </summary>
					 /// <returns></returns>
					volatile FISH_X1_TRANSFER* GetTransferAreasArrayAddr();

					/*!
					*  @brief Start the communication thread for the TA with the TXT
					* @return 0=successful, 1=thread is already running
					*/

					/// <summary>
					/// 3.1 start the communication thread and configere the Motor/Outputs, Inputs and Counters.
					/// </summary>
					/// <remarks> The configurations need to be set before!  </remarks>
					/// <returns>success or error</returns>
					FtErrors ftxStartTransferArea();
					/*!
					* @brief Stop the communication thread for the TA with the TXT
					*/

					/// <summary>
					/// 3.2 stop the communication thread
					/// </summary>
					/// <returns> </returns>
					FtErrors ftxStopTransferArea();
					/*!
					 * @return The TaComThread is running.
					* @return 0=successful, 1=thread is already not running
					 */

					 /// <summary>
					 /// 3.3 Is the communication thread still running?
					 /// </summary>
					 /// <returns>Is running</returns>
					bool   ftxIsTransferActiv();
					/*********************************************************************************/
					/* Bstrac control functions from older interfaces */
					 /*********************************************************************************/
				private:
					FtErrors IsHandleValid();
					TMASK_CB m_mask_cb;
					FtErrors   errCode = FtErrors::FTLIB_ERR_SUCCESS;
					FPCBCounterResetted CBCounterResetted = nullptr;
					FPCBMotorEx         CBMotorExReached = nullptr;
					FPCBCounterResetted2 CBCounterResetted2 = nullptr;//[2013-06-13 CvL]
					FPCBMotorEx2         CBMotorExReached2 = nullptr;//[2013-06-13 CvL]
					FPCBUniChanged         CBUniChanged = nullptr;//[2013-06-13 CvL]
					FPCBCntInChanged         CBCntInChanged = nullptr;//[2013-06-13 CvL]
					FPCBCounterChanged         CBCounterChanged = nullptr;//[2013-06-13 CvL]
				  //  void* CBMultiIFState2Context = nullptr;
					FtErrors SetDuty(FTX1_OUTPUT* pOut, Motor motorId, int duty, Direction direction);
					void* CBCounterResetted2Context = nullptr;
					void* CBMotorExReached2Context = nullptr;

					FtErrors   StartFishX1MotorExCmd(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection, Motor sIdx, Direction sDirection, int pulses);
					FtErrors   StartFishX1MotorExCmd4(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection
						, Motor s1Idx, Direction s1Direction, Motor s2Idx, Direction s2Direction, Motor s3Idx, Direction s3Direction
						, int pulses);//[2013-06-05 CvL]

				public:
					/*********************************************************************************/
					/* Set configuration */
					/*********************************************************************************/
					/// <summary>
					/// 3.11 Configurate the Counter Input
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="iCnt"></param>
					/// <param name="mode"> 1= rising edge, 0 falling edge</param>
					/// <returns></returns>
					FtErrors SetFtCntConfig(ShmIfId_TXT shmId, Counter iCnt, int mode);
					/// <summary>
					/// 3.10 Configurate the Universal Input
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="idxIO"></param>
					/// <param name="mode"></param>
					/// <param name="digital"></param>
					/// <returns></returns>
					FtErrors SetFtUniConfig(ShmIfId_TXT shmId, Input idxIO, InputMode mode, bool digital);
					/// <summary>
					/// 3.12 Configurate the Motor/ Output mode
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="idxMotor"></param>
					/// <param name="status">true means Motor (full bridge), false means Output mode (half bridge)</param>
					/// <returns></returns>
					FtErrors SetFtMotorConfig(ShmIfId_TXT shmId, Motor idxMotor, bool status);
					/*********************************************************************************/
					/* Motor/Output  functions */
					/*********************************************************************************/
					/// <summary>
					/// 3.8A Activates a Motor output
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="motorId"></param>
					/// <param name="duty">0..512</param>
					/// <param name="direction">1=CW, 0 halt, -1=CCW</param>
					/// <returns>Error if the Motor is cofigurated as 2 single Outputs</returns>
					FtErrors SetOutMotorValues(ShmIfId_TXT shmId, Motor motorId, int duty, Direction direction);

					/// <summary>
					/// 3.8B Activates a Motor output, with combine direction and duty.
					/// <seealso>SetOutMotorValues(ShmIfId_TXT shmId, Motor motorId, int duty, Direction direction) </seealso>
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="motorId"></param>
					/// <param name="power">   -512..0..512  (- =CCW and +=CW)</param>
					/// <returns>Error if the Motor is cofigurated as 2 single Outputs</returns>
					FtErrors SetOutMotorValues(ShmIfId_TXT shmId, Motor motorId, int power);

					/// <summary>
					/// N.2.2 Inverse the direction of the motor
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="motorId"></param>
					/// <returns>Error if the Motor is cofigurated as 2 single Outputs</returns>
					FtErrors SetOutMotorInverse(ShmIfId_TXT shmId, Motor motorId);
					/// <summary>
					/// N.2.1 Get the motor power value
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="motorId"></param>
					/// <param name="power">returns a value between =512..0..512</param>
					/// <returns>Error if the Motor is cofigurated as 2 single Outputs</returns>
					FtErrors GetOutMotorValues(ShmIfId_TXT shmId, Motor motorId, int& power);

					/// <summary>
					/// 3.9 Set the duty for outIdx. Needs to be configurated as single mode.
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="outIdx"></param>
					/// <param name="duty"></param>
					/// <returns></returns>
					FtErrors SetOutPwmValues(ShmIfId_TXT shmId, Output outIdx, int duty);



					/// <summary>
					/// (3.13) Enhance motor control, Master and 0..1 slave motor in enhance mode.
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="mIdx"></param>
					/// <param name="duty"></param>
					/// <param name="mDirect">direction of the master motor</param>
					/// <param name="sIdx"0..3 index slave motor 1 or:  Motor::NoMotor  = not in use</param>
					/// <param name="sDirect">direction of the slave 1 motor</param>
					/// <param name="pulses"></param>
					/// <returns></returns>	
					FtErrors StartMotorExCmd(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection, Motor sIdx, Direction sDirection, int pulses);

					/// <summary>
					/// (3.13B)Enhance motor control, Master  Motor in enhance mode.
					/// </summary>
					/// <param name="shmId">Enhanced motor control, Master only in enhance mode. Distance only.</param>
					/// <param name="mIdx"></param>
					/// <param name="duty"></param>
					/// <param name="mDirect">direction of the master motor</param>
					/// <param name="pulses">number of pulses before stop, 1 rotation =xx pulses.</param>
					/// <returns></returns>	
					FtErrors StartMotorExCmd(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection, int pulses);

					///<summary>(3.13A) Enhance motor control, Master and 0..3  slave. Motor in enhance mode..<br/>
					///</summary>
					///<param name='shmId'>which interface</param> 
					///<param name='mIdx'>0..3 index master motor</param> 
					///<param name='duty'>power 0..512 (PWM) </param> 
					///<param name='pulses'>number of pulses before stop, 1 rotation =xx pulses. </param> 
					///<param name='s1Idx'>0..3 index slave motor 1 or:  Motor::NoMotor  = not in use</param> 
					///<param name='s2Idx'>0..3 index slave motor 2 or:  Motor::NoMotor  = not in use</param> 
					///<param name='s3Idx'>0..3 index slave motor 3 or:  Motor::NoMotor  = not in use</param> 
					///<param name='mDirection'>direction of the master motor</param> 
					///<param name='s1Direction'>direction of the slave 1 motor</param> 
					///<param name='s2Direction'>direction of the slave 2 motor</param> 
					///<param name='s3Direction'>direction of the slave 3 motor</param> 
					///<remarks>1 rotation =xx pulses <paramref name=”pulses”/> stop after these pulse with a notification.</remarks>
					///<remarks> Motor::NoMotor = not in use: <paramref name=”s1Idx”/> <paramref name=”s2Idx”/><paramref name=”s3Idx”/>This slave is not in use</remarks>
					///<returns> Ft-error reference.</returns>
					FtErrors StartMotorExCmd(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection
						, Motor s1Idx, Direction s1Direction, Motor s2Idx, Direction s2Direction, Motor s3Idx, Direction s3Direction
						, int pulses);


					/// <summary>
					/// N.2.2 Enhanced motor mode, has the motor reach his position?
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="mIdx"></param>
					/// <returns></returns>
					bool IsMotorExReady(ShmIfId_TXT shmId, Motor mIdx);

					/// <summary>
					/// 3.15 Stops al the motors, set the distances on 0 and remove the master slave relations (reset the enhanced mode)
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <returns></returns>
					FtErrors StopAllMotorExCmd(ShmIfId_TXT shmId);

					/// <summary>
					/// 3.14 Stop Motor output mIdx and reset the enhance mode
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="mIdx"></param>
					/// <returns></returns>
					FtErrors StopMotorExCmd(ShmIfId_TXT shmId, Motor mIdx);

					/// <summary>
					///  N.2.3 Get the value of the Output outIdx
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="outIdx"></param>
					/// <param name="duty"></param>
					/// <returns></returns>
					FtErrors GetOutPwmValues(ShmIfId_TXT shmId, Output outIdx, int& duty);

					/*********************************************************************************/
					/* Counter  */
					/*********************************************************************************/
					/// <summary>
					/// 3.6A Has the counter [shmId:idx] been resetted?
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="mIdx"></param>
					/// <returns></returns>
					bool IsCntResetReady(ShmIfId_TXT shmId, Counter mIdx);

					/// <summary>
					/// 3.6 Start the reset action of  counter [shmId:idx]
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="idx"></param>
					/// <returns></returns>
					FtErrors StartCounterReset(ShmIfId_TXT shmId, Counter idx);

					/// <summary>
					/// 3.18 General get the state of the counter [shmId:idx]
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="idx"></param>
					/// <param name="count">numeric value</param>
					/// <param name="state">logical value, state</param>
					/// <returns></returns>
					FtErrors GetInCounterValue(ShmIfId_TXT shmId, Counter idx, INT16& count, bool& state);

					/// <summary>
					/// 3.18B Get the numeric value of the counter [shmId:idx]
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="idx"></param>
					/// <returns>counter value</returns>
					UINT16 GetInCntCount(ShmIfId_TXT shmId, Counter idx);

					/// <summary>
					/// 3.18A Get the logical value of the counter [shmId:idx]
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="idx"></param>
					/// <returns>Logical value,state</returns>
					bool GetInCntState(ShmIfId_TXT shmId, Counter idx);

					/*********************************************************************************/
					/* Universal inputs */
					/*********************************************************************************/

					/// <summary>
					/// 3.17 General, get the Input [shmId:idx] result
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="idx"></param>
					/// <param name="ftValue">value</param>
					/// <param name="overrun">value is out of range</param>
					/// <returns></returns>
					FtErrors GetInIOValue(ShmIfId_TXT shmId, Input idx, INT16& ftValue, bool& overrun);

					/// <summary>
					/// N.3.1 returns the state. The [shmId:idx, configuration] needs to be okay,
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="idx"></param>
					/// <returns>state; in case of error see log file</returns>
					bool GetInIOValueBinary(ShmIfId_TXT shmId, Input idx);
					/// <summary>
					/// N.3.2 returns the value. The [shmId:idx, configuration] needs to be okay, 
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="idx"></param>
					/// <returns>value or -1=error and see log file</returns>
					INT16 GetInIOValueAnalogue(ShmIfId_TXT shmId, Input idx);
					/*********************************************************************************/
					/* Ir controller inputs */
					/*********************************************************************************/

					/// <summary>
					/// N.5.1 Get partial data [0..15] from one of the 8 halve axis. 
					/// The full axis have been spilt up in parts. 
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="dev">Which fischertechnik IR-controller</param>
					/// <param name="axis"></param>
					/// <returns></returns>
					UINT16 GetJoystickPart(ShmIfId_TXT shmId, IrDev dev, IrAxisP axis);
					/// <summary>
					/// N.5.2 Get full data  [-15..0..15] from one of the 4 full axis
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="dev">Which fischertechnik IR-controller</param>
					/// <param name="axis"></param>
					/// <returns></returns>
					INT16 GetJoystickFull(ShmIfId_TXT shmId, IrDev dev, IrAxisF axis);

					/// <summary>
					/// N.5.3 Get data from a switch or button
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="dev">Which fischertechnik IR-controller</param>
					/// <param name="button"></param>
					/// <returns></returns>
					bool GetJoystickButSwi(ShmIfId_TXT shmId, IrDev dev, IrButSwi button);
					/*********************************************************************************/
					/* Sound and audio */
					/*********************************************************************************/

					/// <summary>
					/// N.6.1 Activate a sound file with index and repeat it.
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="index"></param>
					/// <param name="repeat"></param>
					/// <returns></returns>
					FtErrors SetSound(ShmIfId_TXT shmId, UINT16 index, UINT16 repeat);

					/// <summary>
					/// N.6.2 Check if the activated sound has been finished
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <returns></returns>
					bool IsSoundReady(ShmIfId_TXT shmId);

					/// <summary>
					/// N.6.3 Get microphone data
					/// </summary>
					/// <remarks> does not work in Simple mode</remarks>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <returns></returns>
					UINT16 GetMicLin(ShmIfId_TXT shmId);

					/// <summary>
					/// N.6.4 Get microphone data compressed(logarithmic)
					/// </summary>
					/// <remarks> does not work in Simple mode</remarks>
					/// <param name="shmId">master or slave controller</param>
					/// <returns></returns>
					UINT16 GetMicLog(ShmIfId_TXT shmId);

				private:
					/*********************************************************************************/
					//2013-06-14 Cvl] set callback functions
					//                The callback for the individual Universal Inputs, Counter Change and Counter Input need
					//                to be activate by setting the mask.
					/*********************************************************************************/
					/*********************************************************************************/
					/* call backs   not implemented yet*/
					/*********************************************************************************/

					void  SetCBUniChanged(FPCBUniChanged fpUniEx
					/* void(__stdcall*)(DWORD, DWORD, INT16, BOOL16, UINT8, BOOL8, void*)*/);//2013-06-14 Cvl]

					void  SetCBMotorExReached(FPCBMotorEx fpMotorEx /* void(__stdcall*)(DWORD, DWORD)*/);

					// C++ friedly version for this callback
					void  SetCBMotorExReached2(FPCBMotorEx2 fpMotorEx  /* void(__stdcall*)(DWORD, DWORD, void*)*/);


					// C++ friedly version for this callback
					void  SetCBRoboExtState2(
						void(__stdcall*)(DWORD, DWORD, void*));//2013-06-14 Cvl]
					/// <summary>
					///  
					/// </summary>
					/// <param name="fpCounterResetted"></param>
					void  SetCBCounterResetted(FPCBCounterResetted fpCounterResetted /*void(__stdcall*)(DWORD, DWORD)*/);
					// C++ friedly version for this callback
					void  SetCBCounterResetted2(
						void(__stdcall*)(DWORD, DWORD, void*));//2013-06-14 Cvl]
					void  SetCBCntInChanged(
						void(__stdcall*)(DWORD, DWORD, BYTE, void*));////2013-06-14 Cvl]

					void  SetCBCounterChanged(
						void(__stdcall*)(DWORD, DWORD, INT16, UINT8, void*));//2013-06-14 Cvl]


					 /*********************************************************************************/
					//                Get-set mask for  sensors.
					//                set and get function to allow that a certain Counter or Universal
					//                will use their callback functions
					//                in case the value of this input change.
					 /*********************************************************************************/

					FtErrors  SetCBMaskUniChanged(ShmIfId_TXT shmId, BYTE mask);//2013-06-14 Cvl]
					FtErrors  SetCBMaskCntInChanged(ShmIfId_TXT shmId, BYTE mask);//2013-06-14 Cvl]
					FtErrors  SetCBMaskCounterChanged(ShmIfId_TXT shmId, BYTE mask);//2013-06-14 Cvl]
				   //..........
					FtErrors  GetCBMaskUniChanged(ShmIfId_TXT shmId, BYTE* mask);//2013-06-14 Cvl]
					FtErrors  GetCBMaskCntInChanged(ShmIfId_TXT shmId, BYTE* mask);//2013-06-14 Cvl]
					FtErrors  GetCBMaskCounterChanged(ShmIfId_TXT shmId, BYTE* mask);//2013-06-14 Cvl]
					/*********************************************************************************/



				};
			} //api
		}//end namespace
	}//end namespace
}//end namespace
#endif // ftProInterface2013TransferAreaCom_H