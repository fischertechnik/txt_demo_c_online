///////////////////////////////////////////////////////////////////////////////
//
// File:    Main.cpp
// version 1.1.1.3
// Project: TestProjCallback02- test and example for the sensor callback's, in a class
//
// 2020-08-15 (c)  C van Leeuwen[TesCaWeb.nl] 
//              |  June-Aug 2020  bij [CvL] 
//              |             tested with TXT firmware 4.6.0/4.7.0 pre-release
//              |             Solution update to MS-Visual Studio 2019 dialect C17++, Windows 10
///////////////////////////////////////////////////////////////////////////////
// Configuration: Encoder motor on M1/C1 and M2/C2
// Switch on I1, Ultra sonic on I6
// This sample program does the following:
// - Open connection to TXT interface with IP ????? (NOTE the IP adress is static and only valid for a LAN,
//	not usable on WLAN)
// - Start transfer
// -  Start the motors with distance and wait until ready.
// - After this the joysticks can be used.
// - pressing Switch I1 stop and exit.
// The callbacks as methods (the use of std::bind, std::::placeholders)

// The typical IP addresses / hostnames are:
// - 192.168.7.2 : online via USB
// - 192.168.8.2 : online via WLAN
// - 192.168.9.2 : online via Bluetooth
// - ???.???.???.??? : home network WLAN 
// - localhost   : download mode
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "conio.h"
#include <chrono>
#include <time.h>
#include <fstream>
#include <sstream>
#include "MyCallBacks.h"
#include "FtTxtLib.h"

using namespace std;
using namespace std::placeholders;
using namespace fischertechnik::txt::ids;
using namespace fischertechnik::txt::remote::api;



/****************************************************************************/
/// <summary>
/// Adjust for your own situation
/// </summary>
const std::string MyIP = "192.168.10.171";
const std::string TaPort = "65000";// standard for TXT
const std::string logMap = "H:\\workspaceVS\\txt_demo_c_online.git\\SolutionTxtApps\\";//map for the FtTxtLib log file
/****************************************************************************/

const  Motor IdMotorA = Motor::M1, IdMotorB = Motor::M2, IdMotorC = Motor::M3;
const  Counter IdCntA = Counter::C1, IdCntB = Counter::C2;
const  Input IdSensorUltra = Input::I6, IdSensorSwitch = Input::I1;


const int speedMax = 512, speedLow = 200;
const  int counterA = 400, counterB = 400;
/****************************************************************************/


/// <summary>
/// Command handler 
/// </summary>
ftIF2013TransferAreaComHandlerEx2* ComHandler;
volatile FISH_X1_TRANSFER* TransArea;
char input = ' ';
bool stopWhile = false;



int main()
{
	MyCallBacks* mc = new MyCallBacks();
	ComHandler = new ftIF2013TransferAreaComHandlerEx2(IF_TXT_MAX, MyIP.c_str(), TaPort.c_str(), logMap.c_str(), LogLevel::LvLOGERR);
	if (ComHandler == nullptr) {
		cerr << "Main: ComHandler nullptr" << endl;
		return -3;
	}

	TransArea = ComHandler->GetTransferAreasArrayAddr();

	ComHandler->SetTransferMode(true);//compressed mode, simple mode has no callbacks
	/****************************************************************************/
	/* setup the communication thread with the TXT */
	/* Setup the configuration first*/
	/****************************************************************************/
	//Switch on I1 to end the program

	ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, IdSensorSwitch, InputMode::MODE_R,
		bind(&MyCallBacks::MyUniInCallback, mc, _1, _2, _3, _4));

	ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, Input::I2, InputMode::MODE_R, true);

	ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, IdSensorUltra, InputMode::MODE_ULTRASONIC, false);

	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorA, true,
		bind(&MyCallBacks::MyMotorReached, mc, _1, _2, _3, _4, _5));//As full bridge (M functionality) (default)

	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorB, true,
		bind(&MyCallBacks::MyMotorReached, mc, _1, _2, _3, _4, _5));


	ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntA, true
		,std::bind(&MyCallBacks::MyCount, mc,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		, nullptr//,std::bind(&MyCallBacks::MyCntResetReady, mc, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
	);
	ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntB, true
		,std::bind(&MyCallBacks::MyCount, mc, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		, nullptr//,std::bind(&MyCallBacks::MyCntResetReady, mc,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
	);
	ComHandler->SetFtTaCompleted(std::bind(&MyCallBacks::MyTick, mc, std::placeholders::_1));

	/*********************************************************************************/
	//start the communication thread, includes update configuration.
	/****************************************************************************/
	ComHandler->SetTransferMode(true);
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
	// part 1:
	/****************************************************************************/
	printf("Two encoder motors [on M1/C1 and M2/C2] \n");

	ComHandler->StartMotorExCmd(ShmIfId_TXT::LOCAL_IO, Motor::M1, 360, Direction::CW, Motor::M2, Direction::CW, 100);
	while (!(ComHandler->IsMotorExReady(ShmIfId_TXT::LOCAL_IO, Motor::M1) && ComHandler->IsMotorExReady(ShmIfId_TXT::LOCAL_IO, Motor::M2))) {

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	ComHandler->StartCounterReset(ShmIfId_TXT::LOCAL_IO, Counter::C1);
	ComHandler->StartCounterReset(ShmIfId_TXT::LOCAL_IO, Counter::C2);
	while (!(ComHandler->IsCntResetReady(ShmIfId_TXT::LOCAL_IO, Counter::C1) && ComHandler->IsCntResetReady(ShmIfId_TXT::LOCAL_IO, Counter::C2))) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	ComHandler->StartCounterReset(ShmIfId_TXT::LOCAL_IO, Counter::C1);
	ComHandler->StartCounterReset(ShmIfId_TXT::LOCAL_IO, Counter::C2);
	while (!(ComHandler->IsCntResetReady(ShmIfId_TXT::LOCAL_IO, Counter::C1) && ComHandler->IsCntResetReady(ShmIfId_TXT::LOCAL_IO, Counter::C2))) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}



	ComHandler->StartMotorExCmd(ShmIfId_TXT::LOCAL_IO, Motor::M1, 410, Direction::CW, Motor::M2, Direction::CW, 100);

	while (!(ComHandler->IsMotorExReady(ShmIfId_TXT::LOCAL_IO, Motor::M1) && ComHandler->IsMotorExReady(ShmIfId_TXT::LOCAL_IO, Motor::M2))) {

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	ComHandler->StopMotorExCmd(ShmIfId_TXT::LOCAL_IO, Motor::M1);
	ComHandler->StopMotorExCmd(ShmIfId_TXT::LOCAL_IO, Motor::M2);
	//Set a print for the timer tick
	ComHandler->SetFtTaCompleted(std::bind(&MyCallBacks::MyTick, mc, std::placeholders::_1));
	//==============================================================================================================
	//part 2: motor in a loop with also the use of the joysticks left and right, see callback 
	//===============================================================================================================
	ComHandler->SetFtCbJoyStick(ShmIfId_TXT::LOCAL_IO, IrDev::IR_OFF_ON,
		std::bind(&MyCallBacks::MyJoystick, mc, _1, _2, _3, _4, _5, 6), std::bind(&MyCallBacks::MyJoystick, mc, _1, _2, _3, _4, _5, _6));

	while (!stopWhile )//stopWhile will be set in the Uni callback
	{
		try {

			
		}
		catch (exception& ex) {
			cout << ex.what() << '\n';
			stopWhile = true;
		}//end try

		std::this_thread::sleep_for(std::chrono::milliseconds(20));

	} //end while
	/*********************************************************************************/
	cout << endl << "Main: normal end, " << endl;
	/*********************************************************************************/
	std::cout << "Asking Thread to Stop" << std::endl;
	ComHandler->ftxStopTransferArea();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	std::cout << "End" << std::endl;
	if (ComHandler != nullptr) delete ComHandler;	// Delete transfer area and communication area

	std::system("pause");
	return 0;
}

