///////////////////////////////////////////////////////////////////////////////
//
// File:    MyCallbacks.h
// version 1.1.1.3
// Project: TestProjCallback02- test and example for the sensor callback's, in a class
//
// 2020-08-15 (c)  C van Leeuwen[TesCaWeb.nl] 
//              |  June-Aug 2020  bij [CvL] 
///////////////////////////////////////////////////////////////////////////////
// The callbacks as methods (the use of std::bind, std::::placeholders)
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <iostream>
#include "conio.h"
#include <chrono>
#include <time.h>
#include <fstream>
#include <sstream>
#include "FtTxtLib.h"

using namespace std;
using namespace std::placeholders;
using namespace fischertechnik::txt::ids;
using namespace fischertechnik::txt::remote::api;
extern bool stopWhile  ;

/// <summary>
/// The callbacks but now in a class
/// </summary>

class MyCallBacks {
public:
	/// <summary>
	/// Example callback: print a text
	/// </summary>
	/// <param name="o">calling object</param>
	/// <param name="shmId">master or slave</param>
	/// <param name="id"></param>
	/// <param name="state"></param>
	void MyUniInCallback(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Input id, bool state) {
		std::cout << " MyUniInCallback [" << shmId << ":" << (uint16_t)id << " ] state ="<<(state? "true":"false") << endl;
		if (  (id==Input::I1) && state)  stopWhile = true;
	}
	

	/// <summary>
	/// Example callback: print a text
	/// </summary>
	/// <param name="o">calling object</param>
	/// <param name="shmId">master or slave</param>
	/// <param name="id"></param>
	/// <param name="position"></param>
	/// <param name="messageId"></param>
	void MyMotorReached(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Motor id, uint16_t position, uint16_t messageId) {
		cout << "MyMotorReachedCallback [" << shmId << ":" << (uint16_t)id << " => " << position << "] id=" << messageId << endl;
		
	}
	/// <summary>
	/// Example callback: print a text
	/// </summary>
	/// <param name="o">calling object</param>
	/// <param name="shmId">master or slave</param>
	/// <param name="id"></param>
	/// <param name="count"></param>
	void MyCount(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Counter id, uint16_t count) {
		cout << "MyCount [" << shmId << ":" << (uint16_t)id << " => " << count << "]" << endl;
	}

	/// <summary>
	/// Example callback: print a text
	/// </summary>
	/// <param name="o">calling object</param>
	/// <param name="shmId">master or slave</param>
	/// <param name="id"></param>
	/// <param name="messageId"></param>
	void MyCntResetReady(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Counter id, uint16_t messageId) {
		cout << " MyCntResetReady [" << shmId << ":" << (uint16_t)id << " ] id=" << messageId << endl;
	}
	/// <summary>
	/// Example callback for use with the IR-devices. Joystick value has been changed.
	///         Using only the X-axis of the left and right joystick
	/// </summary>
	/// <param name="o">calling object</param>
	/// <param name="shmId">master or slave</param>
	/// <param name="id">IR device id</param>
	/// <param name="gr">Left or right joystick</param>
	/// <param name="axisX">[-15..00..15]</param>
	/// <param name="axisY">[-15..00..15]</param>
	void MyJoystick(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, IrDev id, IrDevGroup gr, int16_t axisX, int16_t axisY) {
		//cout << " MyJoystick [" << shmId << ":" << (uint16_t)id << " ] id:gr=" << (uint16_t)id << ":" << (uint16_t)gr << " X:Y [" << axisX << ":" << axisY << "]" << endl;
		if (id == IrDev::IR_OFF_ON) {
			switch (gr) {
			case  IrDevGroup::JoystickRight:
				o->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, Motor::M1, (512 / 15) * axisX);
				break;
			case  IrDevGroup::JoystickLeft:
				o->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, Motor::M2, (512 / 15) * axisX);
				break;
			default:cout << " MyJoystick , this device is not in use[" << endl;
				break;
			}
		}

	}
	/// <summary>
	/// 
	/// </summary>
	/// <param name="o"></param>
	void MyTick(ftIF2013TransferAreaComHandlerEx2* o) {

		cout << "tick =" << o->GetTransferAreasArrayAddr()->IFTimer.Timer10ms << endl;
	}
};

