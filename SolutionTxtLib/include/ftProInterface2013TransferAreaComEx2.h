///////////////////////////////////////////////////////////////////////////////
//
// File:    ftProInterface2013TransferAreaComEx2.h
//
// Project: ftPro - fischertechnik Control Graphical Programming System
//
// Module:  Extensie to ftProInterface2013TransferAreaCom.h  and ftProInterface2013TransferAreaComEx1.h
//
// Author:  C.van Leeuwen, Copyright (C) 2020
//
///////////////////////////////////////////////////////////////////////////////
//
// TO DO:
//
///////////////////////////////////////////////////////////////////////////////
// Changes: 2020 - 08 - 25 C.van Leeuwen, Copyright (C) 2020
//          Add callbacks
// Changes: 2020 - 06 - 28 C.van Leeuwen, Copyright (C) 2020
//          Add TA communication Thread in class ftIF2013TransferAreaComHandlerEx2
//          int ftxStartTransferArea();
//          int ftxStopTransferArea();
//          bool TransferAreaIsRunning();
// Changes: 2020 - 06 - 24 C.van Leeuwen, Copyright (C) 2020
//          Choise for Simple or Compressed mode is now a setting
//          void SetTransferMode(bool Compressed);
//          Add class ftIF2013TransferAreaComHandlerEx2
//Changes: 2020 - 06 - 18 C.van Leeuwen, Copyright (C) 2020
//          It is now working for firmware 4.6.6. and 4.7.0 pre-release.
//          Reduce the size of the TA: max=2 TXT master + TXT slave01,
//          Add check in destructor to avoid double EndTransver
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
#include <bitset>

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

				/**********************************************************************************************************/
				//bind: https://stackoverflow.com/questions/28055080/how-to-set-a-member-function-as-callback-using-stdbind
				/**********************************************************************************************************/
				/**************************************************************************************************/
				//Start class
				/**************************************************************************************************/

				class FtPro_API ftIF2013TransferAreaComHandlerEx2 : public ftIF2013TransferAreaComHandlerEx1
				{
				protected:

					/* Thread which take care of the communication between the TXT and the remote TA
					*/
					void thread_TAcommunication(std::future<void> futureObj);
					/* part of the communication thread
					*/
					std::promise<void>  exitSignal;// Create a std::promise object
					std::future<void>  futureObj;// = exitSignal.get_future();
					std::thread   thread1 = thread();
					ftIF2013TransferAreaComHandlerEx2(FISH_X1_TRANSFER* transferarea, int nAreas = 1, const char* name = "192.168.7.2", const char* port = "65000", const char* logMapName = ".\\", LogLevel logLevel = LogLevel::LvLOGERR);



				protected:
					//FtErrors IsHandleValid();
					//FtErrors   errCode = FtErrors::FTLIB_ERR_SUCCESS;




					FtErrors SetDuty(FTX1_OUTPUT* pOut, Motor motorId, int duty, Direction direction);


					//old functions, still internal in use.
					FtErrors   StartFishX1MotorExCmd(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection, Motor sIdx, Direction sDirection, int pulses);
					FtErrors   StartFishX1MotorExCmd4(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection
						, Motor s1Idx, Direction s1Direction, Motor s2Idx, Direction s2Direction, Motor s3Idx, Direction s3Direction
						, int pulses);//[2013-06-05 CvL]
						/**/

					/// <summary>
					/// Do a transfer (uncompressed MASTER ONLY mode
					/// This function is mostly to illustrate the use of the simple uncompressed transfer mode e.g. for use in other languages.
					/// It is recommended to use the compressed transfer mode.
					/// Note: transfers are automatically timed by the interface to once per 10ms
					/// The interface sends the response 10ms after it send the previous response
					/// </summary>
					/// <returns>successful</returns>
					virtual bool DoTransferSimple();

					/// <summary>
					/// Do an I/O transfer with compressed data transmission.
					/// This mode is always faster and more reliable than the simple mode.
					/// Note: transfers are automatically timed by the interface to once per 10ms
					/// The interface sends the response 10 to +/- 30ms after it send the previous response.<br/>
					/// Only in this mode are the callbacks optional active.
					/// </summary>
					/// <returns>successful</returns>

					virtual bool DoTransferCompressed();

				public:
					/*********************************************************************************/
					/* Callback's definitions (signatures)*/
					 /*********************************************************************************/
					/// <summary>
					/// T.N.8.10 The Digital Universal Input [shmId,  id]  has changed into [state]
					/// </summary>
					typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object, ShmIfId_TXT shmId, Input id, bool state)> tCbUniInputDigChanged;
					/// <summary>
					/// T.N.8.12 The Motor  [shmId,  id] (in enhance mode)  has reached its [position].
					/// </summary>
					/// <param name="object">pointer to the caller instance</param>
					/// <param name="messageId">Id of the start MotorEx, .</param>
					typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object, ShmIfId_TXT shmId, Motor id, uint16_t position, uint16_t messageId)> tCbMotorReached;
					/// <summary>
					/// T.N.8.11A Callback for the reset of C counter  [shmId,  id] has been finshed.
					/// </summary>
					///<remarks>only after calling StartCounterReset (a user reset), not after a reset by the system.<remarks>
					/// <param name="object">pointer to the caller instance</param>
					/// <param name="messageId">Id of the reset ready confirmation, >1 user reset, 1 init or motor enhance start.</param>
					typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object, ShmIfId_TXT shmId, Counter id, uint16_t messageId)> tCbCntResetReady;
					/// <summary>
					///  T.N.8.11B Callback for the C counter  [shmId,  id] has changed, last known [count]
					/// </summary>
					/// <param name="object">pointer to the caller instance</param>
					typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object, ShmIfId_TXT shmId, Counter id, uint16_t count)> tCbCount;
					/// <summary>
					///  T.N.8.20 Callback for the TA communication, direct after the send and received inputs (TaCompleted)
					/// </summary>
					/// <param name="object">pointer to the caller instance</param>
					typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object)> tCbTaCompleted;
					/// <summary>
					///  T.N.8.21 Callback for the fischertechnik IR-controller (Joystick) 
					/// </summary>
					/// <param name="object">pointer to the caller instance</param>
					/// <param name="shmId">Which TXT controller, master or slave</param>
					/// <param name="id">which IR-device</param>
					/// <param name="group">which IR-devicejoystick, Left or Right</param>
					/// <param name="AxisX">X-as value of the joystick [-15..0..15]</param>
					/// <param name="AxisY">Y-as value of the joystick  [-15..0..15]</param>
					typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object, ShmIfId_TXT shmId, IrDev id, IrDevGroup group, int16_t AxisX, int16_t AxisY)> tCbTaJoyStick;
					/*********************************************************************************/
					/* constructor and destructor
					 /*********************************************************************************/
					 /// <summary>
					 /// 3.0A constructor
					 /// </summary>
					 /// <param name="nAreas"></param>
					 /// <param name="name"></param>
					 /// <param name="port"></param>
					 /// <param name="logMapName"></param>
					 /// <param name="logLevel"></param>
					 /// <returns></returns>
					ftIF2013TransferAreaComHandlerEx2(int nAreas = 1, const char* name = "192.168.7.2", const char* port = "65000", const char* logMapName = ".\\", LogLevel logLevel = LogLevel::LvLOGERR);

					/// <summary>
						/// 3.0B de
						/// </summary>
					~ftIF2013TransferAreaComHandlerEx2();
					/*********************************************************************************/
					/* Communication thread */
					 /*********************************************************************************/

					 /// <summary>
					 /// 3.4 Get the pointer to the Transfer area
					 /// </summary>
					 /// <returns>the pointer</returns>
					volatile FISH_X1_TRANSFER* GetTransferAreasArrayAddr();

					/// <summary>
					/// 3.1 start the communication thread and configere the Motor/Outputs, Inputs and Counters.<br/>
					/// The cycle time is betweem the 10 and +/- 40 msec.
					/// </summary>
					/// <remarks> The configurations for the TXT needs to be set before this start!  </remarks>
					/// <returns>success or error</returns>
					FtErrors ftxStartTransferArea();

					/// <summary>
					/// 3.2 stop the communication thread
					/// </summary>
					/// <returns> </returns>
					FtErrors ftxStopTransferArea();

					/// <summary>
					/// 3.3 Is the communication thread still running?
					/// </summary>
					/// <returns>Is running</returns>
					bool   ftxIsTransferActiv();

					/*********************************************************************************/
					/* Set configuration */
					/*********************************************************************************/

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
					/// N.8.10 (3.10A) Configurate the Universal Input, digital with callback
					/// </summary>
					/// <param name="shmId">master or master+slave </param>
					/// <param name="idxIO"></param>
					/// <param name="mode">only for digital R or V</param>
					/// <param name="tCbUniInputDigChanged"></param>
					/// <param name="callback">optional callback for digital input has been changed mode</param>
					/// <returns>error if not R or V</returns>
					/// <see>https://stackoverflow.com/questions/14189440/c-callback-using-class-member </see>
					/// 
					FtErrors SetFtUniConfig(ShmIfId_TXT shmId, Input idxIO, InputMode mode, tCbUniInputDigChanged callback = nullptr);
					/// <summary>
					/// N.8.11 (3.11)  Configurate the Counter Input
					/// </summary>
					/// <param name="shmId">master or slave</param>
					/// <param name="iCnt">id counter  </param>
					/// <param name="mode"> 1= rising edge, 0 falling edge</param>
					/// <param name="callbackCount">optional callback for counter value has been changed mode</param>
					/// <param name="callbackCnt">optional callback for reset has been finished.</param>
					/// <returns>error in case of parameter errors</returns>
					FtErrors SetFtCntConfig(ShmIfId_TXT shmId, Counter iCnt,int mode,tCbCount  callbackCount = nullptr,	tCbCntResetReady  callbackCnt = nullptr);
					/// <summary>
					/// N.8.12 (3.12) Configurate the Motor/ Output mode
					/// </summary>
					/// <param name="shmId"></param>
					/// <param name="idxMotor"></param>
					/// <param name="status">true means Motor (full bridge), false means Output mode (half bridge)</param>
					/// <param name="callback">optional callback for enhance mode, motor has reached its position</param>
					/// <returns></returns>
					FtErrors SetFtMotorConfig(ShmIfId_TXT shmId, Motor idxMotor, bool status, tCbMotorReached  callback = nullptr);

					/// <summary>
					/// N.3.20 Set Callback for Transfer Area Cycle Complete (direct after a send/received
					/// </summary>
					/// <param name="callback">optional callback for TA transfer tick</param>
					/// <returns></returns>
					FtErrors SetFtTaCompleted(tCbTaCompleted  callback = nullptr);

					/// <summary>
					/// N.8.21 Set Callbacks (events) for the IT-device (Joystick controller)
					/// </summary>
					/// <param name="shmId">master or master+slave</param>
					/// <param name="id">IR-controller id</param>
					/// <param name="JsX">callback for the X axis or nullptr</param>
					/// <param name="JsY">callback for the Y axis or nullptr</param> 
					/// <returns>success or error</returns>
					FtErrors SetFtCbJoyStick(ShmIfId_TXT shmId, IrDev id, tCbTaJoyStick callbackJsX = nullptr, tCbTaJoyStick  callbackJsY = nullptr);
	

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
					/// <param name="mIdx">index master motor</param>
					/// <param name="duty">[0..512]</param>
					/// <param name="sIdx">0..3 index slave motor 1 or:  Motor::NoMotor  = not in use</param>
					/// <param name="mDirect">direction of the master motor</param>	
					/// <param name="sDirect">direction of the slave 1 motor</param>
					/// <param name="pulses"></param>
					/// <returns></returns>	
					FtErrors StartMotorExCmd(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection, Motor sIdx, Direction sDirection, int pulses);

					/// <summary>
					/// (3.13B)Enhance motor control, Master  Motor in enhance mode.<br/>
					/// Generates before the start of the motor(s) also a reset for the addressed Counter(s).
					/// </summary>
					/// <param name="shmId">Enhanced motor control, Master only in enhance mode. Distance only.</param>
					/// <param name="mIdx">index master motor</param>
					/// <param name="duty">[0..512]</param>
					/// <param name="mDirect">direction of the master motor</param>
					/// <param name="pulses">number of pulses before stop, 1 rotation =xx pulses.</param>
					/// <returns>Ft-error reference.</returns>	
					FtErrors StartMotorExCmd(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection, int pulses);

					///<summary>(3.13A) Enhance motor control, Master and 0..3  slave. Motor in enhance mode.<br/>
					/// Generates before the start of the motor(s) also a reset for the addressed Counter(s).
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
					///<returns> Ft-error reference.</returns>
					FtErrors StartMotorExCmd(ShmIfId_TXT shmId, Motor mIdx, int duty, Direction mDirection
						, Motor s1Idx, Direction s1Direction, Motor s2Idx, Direction s2Direction, Motor s3Idx, Direction s3Direction
						, int pulses);


					/// <summary>
					/// N.2.2 Enhanced motor mode, has the motor reached his position?
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="mIdx"></param>
					/// <returns></returns>
					bool IsMotorExReady(ShmIfId_TXT shmId, Motor mIdx);
					/// <summary>
					/// 3.14 Stop Motor output mIdx, set the distance on 0 and remove the master slave relation (reset the enhance mode)
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <param name="mIdx"></param>
					/// <returns></returns>
					FtErrors StopMotorExCmd(ShmIfId_TXT shmId, Motor mIdx);

					/// <summary>
					/// 3.15 Stops al the motors, set the distances on 0 and remove the master slave relations (reset the enhanced mode)
					/// </summary>
					/// <param name="shmId">Master or slave TXT controller</param>
					/// <returns></returns>
					FtErrors StopAllMotorExCmd(ShmIfId_TXT shmId);

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
					/// <returns>error</returns>
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
					/// <summary>
					/// The number of active slaves.
					/// </summary>
					/// <returns></returns>
					int  IsSlaveActive();
				protected:

					tCbUniInputDigChanged  ArCbUniInputDigChanged[SHM_IF_CNT][IZ_UNI_INPUT] = { {nullptr} };//new 2020-08-15
					tCbMotorReached  ArCbMotorReached[SHM_IF_CNT][IZ_MOTOR] = { {nullptr} };//new 2020-08-15
					tCbCntResetReady  ArCbCntResetReady[SHM_IF_CNT][IZ_COUNTER] = { {nullptr} };//new 2020-08-15
					tCbCount  ArCbCount[SHM_IF_CNT][IZ_COUNTER] = { {nullptr} };//new 2020-08-15
					tCbTaCompleted CbTaCompleted = nullptr;//new 2020-08-22
					tCbTaJoyStick ArCbTaJoyStickL[SHM_IF_CNT][IZ_IR_RECEIVER + 1] = { {nullptr} };//new 2020-08-26
					tCbTaJoyStick ArCbTaJoyStickR[SHM_IF_CNT][IZ_IR_RECEIVER + 1] = { {nullptr} };//new 2020-08-26

				};
			} //api
		}//end namespace
	}//end namespace
}//end namespace
#endif // ftProInterface2013TransferAreaCom_H