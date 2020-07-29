///////////////////////////////////////////////////////////////////////////////
//
// File:    Main.cpp
// version 1.1.3.2
// Project: TestProjIrControl - example of the TA-communication thread and the Fischertechnik IR JoyStick control.
//                              basic use of the motor enchanced control, use of the API
//
//
// 2020-07-25 (c) TesCaWeb ing.C van Leeuwen Btw.
//              |  June-July 2020  bij [CvL] 
//              |             tested with TXT firmware 4.6.0/4.7.0 pre-release
//              |             Solution update to MS-Visual Studio 2019 dialect C17++, Windows 10
//
///////////////////////////////////////////////////////////////////////////////
//
// This sample program does the following:
// - Open connection to TXT interface with IP 192.168.10.171 (NOTE the IP adress is static and only valid for a LAN,
//	not usable on WLAN)
// 
// - Start transfer

// The typical IP addresses / hostnames are:
// - 192.168.7.2 : online via USB
// - 192.168.8.2 : online via WLAN
// - 192.168.9.2 : online via Bluetooth
// - 192.168.178.21 : home network WLAN 
// - localhost   : download mode
///////////////////////////////////////////////////////////////////////////////


#include "FtTxtLib.h"
#include <iostream>
#include "conio.h"
#include <chrono>
#include <time.h>
#include <fstream>
#include <sstream>


using namespace fischertechnik::txt::ids;
using  namespace fischertechnik::txt::remote::api;

using namespace std;
/****************************************************************************/
/// <summary>
/// Adjust for your own situation
/// </summary>
const std::string MyIP = "192.168.10.171";
const std::string TaPort = "65000";
const std::string logMap = "H:\\workspaceVS\\txt_demo_c_online.git\\SolutionTxtApps\\";//map for the internal log file
/****************************************************************************/

const  Motor IdMotorA = Motor::M1, IdMotorB = Motor::M2, IdMotorC = Motor::M3;
const  Counter IdCntA = Counter::C1, IdCntB = Counter::C2;
const  Input IdSensorUltra = Input::I6, IdSensorSwitch = Input::I1;

const int speedMax = 512, speedLow = 200;
const  int counterA = 400, counterB = 400;

void PrintInfo(volatile  FISH_X1_TRANSFER* TransArea);

/// <summary>
/// Command handler 
/// </summary>
ftIF2013TransferAreaComHandlerEx2* ComHandler;
char input = ' ';
bool stopWhile = false;
volatile FISH_X1_TRANSFER* TransArea;

int main()
{
	ComHandler = new ftIF2013TransferAreaComHandlerEx2(IF_TXT_MAX, MyIP.c_str(), TaPort.c_str(), logMap.c_str(), LogLevel::LvLOG);
	if (ComHandler == nullptr) {
		cerr << "Main: ComHandler nullptr" << endl;
		return -3;
	}

	TransArea = ComHandler->GetTransferAreasArrayAddr();

	ComHandler->SetTransferMode(true);
	/****************************************************************************/
	/* setup the communication thread with the TXT */
	/* Setup the configuration first*/
	/****************************************************************************/
	//Switch on I1 to end the program
	ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, IdSensorSwitch, InputMode::MODE_R, true);
	//ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, IdSensorUltra, MODE_ULTRASONIC, false);
	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorA, true);//As full bridge (M functionality) (default)
	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorB, true); //As full bridge (M functionality)(default)
	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorC, false);//As two half  bridges (O functionality)
	ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntA, false);
	ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntB, true);
	/*********************************************************************************/
	//start the communication thread, includes update configuration.
	/****************************************************************************/
	ComHandler->SetTransferMode(false);
	if (!ComHandler->ftxStartTransferArea() == FtErrors::FTLIB_ERR_SUCCESS)
	{
		cout << "Main: Error: ftxStartTransferArear" << endl;
		do
		{
			cerr << '\n' << "Press a key to continue...";
		} while (cin.get() != '\n');
		return false;
	}
	/*********************************************************************************/
	// ready to start the main loop
	/****************************************************************************/
	int iLoop = 0;
	printf("Two encoder motors [on M1/C1 and M2/C2] \n");
	int16_t r = 0, l = 0;
	int16_t factor = 512 / 15;//transformation from Joystick (-15..0..15) to motor (-512..0..512)
	while (!stopWhile && (iLoop < 500))
	{
		try {
			r = ComHandler->GetJoystickFull(ShmIfId_TXT::LOCAL_IO, IrDev::IR_ON_OFF, IrAxisF::JoyLeftX);
			l = ComHandler->GetJoystickFull(ShmIfId_TXT::LOCAL_IO, IrDev::IR_ON_OFF, IrAxisF::JoyRightX);
			cout << "Joystick =" << r << '\n';
			ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, Motor::M1, r * factor);
			ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, Motor::M2, l * factor);
			if (ComHandler->GetInIOValueBinary(ShmIfId_TXT::LOCAL_IO, Input::I1)) stopWhile = true;
		}
		catch (exception& ex) {
			cout << ex.what() << '\n';
			stopWhile = true;
		}//end try

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		if (iLoop % 10 == 0)
		{  //remove comment to see more information
			//ComHandler->PrintIO(ShmIfId_TXT::LOCAL_IO);
			//ComHandler->PrintIR(ShmIfId_TXT::LOCAL_IO);
			//ComHandler->PrintIO(ShmIfId_TXT::REMOTE_IO_1);
			//ComHandler->PrintIR(ShmIfId_TXT::REMOTE_IO_1);
		}
	} //end while
	/*********************************************************************************/
	cout << endl << "Main: normal end, transfers done iLoop=" << iLoop << endl;
	/*********************************************************************************/
	std::cout << "Asking Thread to Stop" << std::endl;
	ComHandler->ftxStopTransferArea();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	std::cout << "End" << std::endl;
	if (ComHandler != nullptr) delete ComHandler;	// Delete transfer area and communication area
	std::cout << "pause 1" << std::endl;
	std::system("pause");
	return 0;
}

