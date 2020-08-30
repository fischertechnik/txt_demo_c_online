///////////////////////////////////////////////////////////////////////////////
//
// File:    Main.cpp
// version 1.1.1.2
// Project: TestProjCallback- test and example for the sensor callback's
//                             
//
//
// 2020-08-25 (c) ing.C van Leeuwen Btw. [TesCaWeb.nl]
//              |  June-August 2020  bij [CvL] 
//              |             tested with TXT firmware 4.6.0/4.7.0 pre-release
//              |             Solution update to MS-Visual Studio 2019 dialect C17++, Windows 10
//
///////////////////////////////////////////////////////////////////////////////
//
// This sample program does the following:
// - Open connection to TXT interface with IP 192.168.10.171 (NOTE the IP adress is static and only valid for a LAN,
//	not usable on WLAN)
// 
// - Start transfer, part 1 , part 2, exit
///////////////////////////////////////////////////////////////////////////////

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
using  namespace fischertechnik::txt::remote::api;

/****************************************************************************/
/// <summary>
/// Adjust for your own situation
/// </summary>
const std::string MyIP = "192.168.10.171";//adapt to your situation!
const std::string TaPort = "65000";//don't change
const std::string logMap = "H:\\workspaceVS\\txt_demo_c_online.git\\SolutionTxtApps\\";//map for the internal log file, adapt to your situation!
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

/*************************************************************************************************************************/
/*    example callbacks  */
/*************************************************************************************************************************/

	/// <summary>
	/// 
	/// </summary>
	/// <param name="o"></param>
	/// <param name="shmId"></param>
	/// <param name="id"></param>
	/// <param name="state"></param>
void MyUniInCallback(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Input id, bool state) {
	cout << " MyUniInCallback [" << shmId << ":" << (uint16_t)id << " ] state=" << ( (state)? "true" : "false") << endl;
}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="o"></param>
	/// <param name="shmId"></param>
	/// <param name="id"></param>
	/// <param name="position"></param>
	/// <param name="messageId"></param>
void MyMotorReached(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Motor id, uint16_t position, uint16_t messageId) {
	cout << "MyMotorReachedCallback [" << shmId << ":" << (uint16_t)id << " => " << position << "] id=" << messageId << endl;
}

/// <summary>
/// 
/// </summary>
/// <param name="o"></param>
/// <param name="shmId"></param>
/// <param name="id"></param>
/// <param name="count"></param>
void MyCount(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Counter id, uint16_t count) {
	cout << "MyCount [" << shmId << ":" << (uint16_t)id << " => " << count << "]" << endl;
}

/// <summary>
/// 
/// </summary>
/// <param name="o"></param>
/// <param name="shmId"></param>
/// <param name="id"></param>
/// <param name="messageId"></param>
void MyCntResetReady(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Counter id, uint16_t messageId) {
	cout << " MyCntResetReady [" << shmId << ":" << (uint16_t)id << " ] id=" << messageId << endl;
}

//==============================================================================================================

int main()
{
	ComHandler = new ftIF2013TransferAreaComHandlerEx2(IF_TXT_MAX, MyIP.c_str(), TaPort.c_str(), logMap.c_str(), LogLevel::LvLOGERR);
	if (ComHandler == nullptr) {
		cerr << "Main: ComHandler nullptr" << endl;
		return -3;
	}

	TransArea = ComHandler->GetTransferAreasArrayAddr();

	ComHandler->SetTransferMode(true);// In simple mode callbacks are not implemented
	/****************************************************************************/
	/* setup the communication thread with the TXT */
	/* Setup the configuration first*/
	/* Set some callbacks */
	/****************************************************************************/
	//Switch on I1 to end the program
	ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, Input::I2, InputMode::MODE_R, true);


	//ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, IdSensorSwitch, InputMode::MODE_R, &MyUniInCallback);//
	ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, IdSensorSwitch, 	InputMode::MODE_R, 
		std::bind(&MyUniInCallback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

	ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, IdSensorUltra, InputMode::MODE_ULTRASONIC,false);

	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorA, true, &MyMotorReached);//As full bridge (M functionality) (default)
	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorB, true, &MyMotorReached); //As full bridge (M functionality)(default)
	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorC, false);//As two half  bridges (O functionality)
	//Two different ways of using a function or static member methode.
	//1 with std::bind and std::placeholders
	ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntA, false 
		,bind(&MyCount, _1, _2, _3, _4)
		,bind(&MyCntResetReady, _1, _2, _3, _4)
		);
	//2 direct use
	ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntB, true
		, &MyCount
		, &MyCntResetReady
		);


	/*********************************************************************************/
	//start the communication thread, includes update configuration.
	/*********************************************************************************/
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
	// ready to start the main loop
	/*********************************************************************************/
	int iLoop = 0;
	printf("Two encoder motors [on M1/C1 and M2/C2] \n");
	int16_t r = 0, l = 0;
	int16_t factor = (512) / 15;//transformation from Joystick (-15..0..15) to motor (-512..0..512)
	/*********************************************************************************/
    //part 1: 2 motors in enhance mode start, wait unitil ready and reset enhance mode.
	/*********************************************************************************/
	ComHandler->StartMotorExCmd(ShmIfId_TXT::LOCAL_IO, Motor::M1,400,Direction::CW, Motor::M2, Direction::CW,200);

	while (!(ComHandler->IsMotorExReady(ShmIfId_TXT::LOCAL_IO, Motor::M1) && ComHandler->IsMotorExReady(ShmIfId_TXT::LOCAL_IO, Motor::M2))) {

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	//Todo: If not reset enhance mode with StopMotorExCmd, part 2 will not work, Try this yourself
	ComHandler->StopMotorExCmd(ShmIfId_TXT::LOCAL_IO, Motor::M1);//needed
	ComHandler->StopMotorExCmd(ShmIfId_TXT::LOCAL_IO, Motor::M2);//needed
	/*********************************************************************************/
	//part 2
	/*********************************************************************************/
	while (!stopWhile && (iLoop < 500))
	{
		try {
			//read the Joystick and set the motor duty.
			r = ComHandler->GetJoystickFull(ShmIfId_TXT::LOCAL_IO, IrDev::IR_ON_OFF, IrAxisF::JoyLeftX);
			l = ComHandler->GetJoystickFull(ShmIfId_TXT::LOCAL_IO, IrDev::IR_ON_OFF, IrAxisF::JoyRightX);
			cout << "Joystick l:r= [ " <<l<<":"<< r << " ]\n";
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

