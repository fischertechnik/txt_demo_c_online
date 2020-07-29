///////////////////////////////////////////////////////////////////////////////
//
// File:    Main.cpp
// version 1.1.3.2
// Project: TestProj01 - example of the TA-communication thread, simple keyboard control.
//                  basic use of the motor enchanced control, use of the API
//
// Module:  TA-communication thread 
//
// 2020-06-15 (c) TesCaWeb ing.C van Leeuwen Btw.
//              |  June-July 2020  bij [CvL] 
//              |             tested with TXT firmware 4.6.0/4.7.0 pre-release
//              |             Solution update to MS-Visual Studio 2019 dialect C14++, Windows 10
//
///////////////////////////////////////////////////////////////////////////////
//
// This sample program does the following:
// - Open connection to TXT interface with IP 192.168.10.171 (NOTE the IP adress is static and only valid for a LAN,
//	not usable on WLAN)
// 
// - Start transfer
// - switch motor A and B of master on/off depending on the a,w,s,d keyboard keys
// - h= stop normal motors, q = stop extended use,
// - x,c examples of how to use the extendede motor control
// 
//
// The typical IP addresses / hostnames are:
// - 192.168.7.2 : online via USB
// - 192.168.8.2 : online via WLAN
// - 192.168.9.2 : online via Bluetooth
// - 192.168.178.21 : home network WLAN 
// - localhost   : download mode
//changes:
// I2C line576.
// 
// 
///////////////////////////////////////////////////////////////////////////////


#include "FtTxtLib.h"
#include <iostream>
#include "conio.h"
#include <chrono>
#include <time.h>
#include <fstream>
#include <sstream>
#include <mutex>
#include <map>


using namespace fischertechnik::txt::ids;
//using  fischertechnik::txt::remote::api::ftIF2013TransferAreaComHandlerEx2;
using  namespace fischertechnik::txt::remote::api;
using  namespace fischertechnik::txt::remote;

using namespace std;


const std::string MyIP = "192.168.10.171";
const std::string TaPort = "65000";
const std::string logMap = "H:\\workspaceVS\\txt_demo_c_online.git\\SolutionTxtApps\\";//map for the internal log file

const  Motor IdMotorA = Motor::M1; 
const  Counter IdCntA = Counter::C1;
const   Motor IdMotorB = Motor::M2;
const  Counter IdCntB = Counter::C2;
const   Motor IdMotorC = Motor::M3;
const int speedMax = 512, speedLow = 200;
const   Input IdSensorIUltraA = Input::I1;//distance sensor on I1
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
    ComHandler = new ftIF2013TransferAreaComHandlerEx2( IF_TXT_MAX, MyIP.c_str(), TaPort.c_str(), logMap.c_str(), LogLevel::LvLOG);
	if (ComHandler == nullptr) {
		cerr << "Main: ComHandler nullptr" << endl;
		return -3;
	}	
	
	TransArea = ComHandler->GetTransferAreasArrayAddr();
	/****************************************************************************/
	/* setup the communication thread with the TXT                              */
	/****************************************************************************/
	ComHandler->SetTransferMode(true);

	ComHandler->SetFtUniConfig( ShmIfId_TXT::LOCAL_IO, IdSensorIUltraA, MODE_ULTRASONIC, false);
	ComHandler->SetFtMotorConfig( ShmIfId_TXT::LOCAL_IO, IdMotorA, true);//As full bridge (M functionality) (default)
	ComHandler->SetFtMotorConfig( ShmIfId_TXT::LOCAL_IO, IdMotorB, true); //As full bridge (M functionality)(default)
	ComHandler->SetFtMotorConfig( ShmIfId_TXT::LOCAL_IO, IdMotorC, false);//As two half  bridges (O functionality)
	ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntA, false);
	ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntB, true);
	PrintInfo(TransArea);
	//start the handler, includes update configuration.
if (!ComHandler->ftxStartTransferArea() == 0)
	{
		cout << "Main: Error: ftxStartTransferArear" << endl;
		do
		{
			cerr << '\n' << "Press a key to continue...";
		} while (cin.get() != '\n');
		return -4;
	}

	PrintInfo(TransArea);
	/*********************************************************************************/
		// ready to start the main loop
	int iLoop = 0;
	printf("Two encoder motors [on M1/C1 and M2/C2] \n");
	
	while (!stopWhile)
	{
		try {
			printf("Typed  [t=exit program ] \n");
			printf("Typed  [for normal motor use: a, s, d, w, h=stop motors ] \n");
			printf("Typed  [for enhanced use: x, X, c and v =  experiment, q=exit exhanced, z=print status info, b=reset counters] \n");
			input = _getch();//Keyboard is blocking
			printf("You  typed: %c!\n", input);

			switch (input)
			{
				//*****************************************************************
				// This part shows the basics of the normal motor use
				//******************************************************************
				// s,w,d and a: does not work when the motor is in the extended mode
				// q is needed to reset the extended mode
			case 's':
				//MotorA=ccw and MotorB=ccw
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorA, speedMax, Direction::CCW);
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorB, speedMax, Direction::CCW);
				input = ' ';
				break;

			case 'w':
				//MotorA=cw and MotorB=cw
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorA, speedMax, Direction::CW);
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorB, speedMax);
				input = ' ';
				break;
			case 'W':
				//MotorA=cw and MotorB=cw
				ComHandler->SetOutMotorInverse(ShmIfId_TXT::LOCAL_IO, IdMotorA);
				ComHandler->SetOutMotorInverse(ShmIfId_TXT::LOCAL_IO, IdMotorB);
				ComHandler->SetOutMotorInverse(ShmIfId_TXT::LOCAL_IO, IdMotorC);
				input = ' ';
				break;
			case 'd':
				//MotorA=ccw and MotorB=cw
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorA, -1*speedLow);
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorB, speedLow);
				input = ' ';
				break;

			case 'a':
				//MotorA=cw and MotorB=ccw
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorA, speedLow, Direction::CW);
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorB, speedLow, Direction::CCW);
				input = ' ';
				break;
			case 'h':
				//MotorA=stop and MotorB=stop
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorA, 0, Direction::Halt);
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorB, 0, Direction::Halt);
				input = ' ';
				break;
				//*****************************************************************
				// This part shows the basics of the enhandched motor use.
				// Synchro with Motor A as master.
				// A master can have 1, 2 or 3 slave motors (on the same controller)
				// Motors on different controllers can not be mix-up.
				//******************************************************************

			case 'b':
				//reset the counters and wait until ready
				//The cnt_resetted will be set by the ftProInterface2013 interface when ready.
				// Note: A start motor sychro will also reset the counters.
				ComHandler->StartCounterReset(ShmIfId_TXT::LOCAL_IO, IdCntA);
				ComHandler->StartCounterReset(ShmIfId_TXT::LOCAL_IO, IdCntB);
				bool ready3;
				do {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));//avoid blocking
					ready3 = ComHandler->IsCntResetReady(ShmIfId_TXT::LOCAL_IO, IdCntA) && ComHandler->IsCntResetReady(ShmIfId_TXT::LOCAL_IO, IdCntB);

				} while (!ready3);
				break;
			case 'X':

				do {
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					ComHandler->PrintIO(ShmIfId_TXT::LOCAL_IO);
				}
                while ( !ComHandler->IsMotorExReady(ShmIfId_TXT::LOCAL_IO, IdMotorA) || !ComHandler->IsMotorExReady(ShmIfId_TXT::LOCAL_IO, IdMotorB));
				//ComHandler->StopMotorExCmd(ShmIfId_TXT::LOCAL_IO, IdMotorA);
				//ComHandler->StopMotorExCmd(ShmIfId_TXT::LOCAL_IO, IdMotorB);
				input = ' ';

				break;
			case 'x':
				ComHandler->StartMotorExCmd(ShmIfId_TXT::LOCAL_IO, IdMotorA, 256, Direction::CCW, counterA);
				input = ' ';
				break;
				case 'v':
				ComHandler->StartMotorExCmd(ShmIfId_TXT::LOCAL_IO, IdMotorA, 256, Direction::CCW, IdMotorB, Direction::CW, counterA);
				input = ' ';
				break;	
				case 'c':
				//Same as x but the other function
				ComHandler->StartMotorExCmd(ShmIfId_TXT::LOCAL_IO, IdMotorA, 256, Direction::CCW, 
					IdMotorB, Direction::CW, Motor::NoMotor, Direction::NoDirection, Motor::NoMotor, Direction::NoDirection,
					counterA);
				input = ' ';
				break;


				//look what the progress is
			case 'o':
				ComHandler->SetOutPwmValues(ShmIfId_TXT::LOCAL_IO, Output::O5, 512);
				ComHandler->SetOutPwmValues(ShmIfId_TXT::LOCAL_IO, Output::O6, 512);
				ComHandler->SetOutPwmValues(ShmIfId_TXT::LOCAL_IO, Output::O7, 512);

				PrintInfo(TransArea);
				break;
			case 'p':
				ComHandler->SetOutPwmValues(ShmIfId_TXT::LOCAL_IO, Output::O5, 0);
				ComHandler->SetOutPwmValues(ShmIfId_TXT::LOCAL_IO, Output::O6, 0);
				ComHandler->SetOutPwmValues(ShmIfId_TXT::LOCAL_IO, Output::O7, 0);
				int r1, r2;

				ComHandler->GetOutPwmValues(ShmIfId_TXT::LOCAL_IO, Output::O5, r1);
				ComHandler->GetOutPwmValues(ShmIfId_TXT::LOCAL_IO, Output::O6, r2);
				cout << "Output      O5,O6 = [" << r1 << ", " << r2 << "]"  << endl;
				PrintInfo(TransArea);
				break;
			case 'i':
				INT16 rr1, rr2;
				bool bb1, bb2;
				ComHandler->GetInIOValue(ShmIfId_TXT::LOCAL_IO, Input::I6, rr1, bb1);
				ComHandler->GetInIOValue(ShmIfId_TXT::LOCAL_IO, Input::I1, rr2, bb2);
				cout << "Input      I1,I6 = [ " << rr1 << " , " << bb1 << " ],[  " << bb2 << ", " << rr2 << " ]" << endl;
				PrintInfo(TransArea);
				break;

			case 'q'://reset (stop) exsteded motor control
									
				ComHandler->StopMotorExCmd(ShmIfId_TXT::LOCAL_IO, IdMotorA);
				ComHandler->StopMotorExCmd(ShmIfId_TXT::LOCAL_IO, IdMotorB);
				break;
			case 'm':  //test sound and microphone
              ComHandler->SetSound(ShmIfId_TXT::LOCAL_IO,1,2);
			 
				do {

					cout << "Sound not ready mic=[" <<
					ComHandler->GetMicLin(ShmIfId_TXT::LOCAL_IO)<<" , "<<
					 ComHandler->GetMicLog(ShmIfId_TXT::LOCAL_IO)<<"]"<< endl;

					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					
				} while (!ComHandler->IsSoundReady(ShmIfId_TXT::LOCAL_IO) );
				input = ' ';

				break;

			case 't':
				/*****************************************************************
				* Exit loop                                                      *
				*******************************************************************/
				input = ' ';
				stopWhile = true;
				break; 
				/******************************************************************
				* I2C related operations                                          *
				* Experimental bacause the protocol is not available              *
				*******************************************************************/
			case 'k':
				if (!ComHandler->StartI2C()) {
					cout << endl << "Main: StartI2C() error" << endl;
				}
				else cout << endl << "Main: StartI2C() Ok" << endl;
				break;
			case 'l':
				if (!ComHandler->StopI2C()) {
					cout << endl << "Main: StopI2C() error" << endl;
				}
				else cout << endl << "Main: StopI2C() Ok" << endl;
				break;
				/******************************************************************
				* End I2C related operations                                      *
				*******************************************************************/
			default:
				input = ' ';
				break;

			}
			// Print I/O values for master and extension, only every 4th IO (console IO is slow)
			if (iLoop % 10 == 0)
			{
				//ComHandler->PrintIO(ShmIfId::LOCAL_IO);
				//ComHandler->PrintIO(ShmIfId::REMOTE_IO_1);
			}
			iLoop++;
		}
		catch (exception& ex) {
			cout << ex.what() << '\n';
			stopWhile = true;
		}//end try
	} //end while

	cout << endl << "Main: normal end, transfers done iLoop=" << iLoop << endl;
	//******************************************************
	std::cout << "Asking Thread to Stop" << std::endl;
	ComHandler->ftxStopTransferArea();
	
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	//std::cout << "Thread join" << std::endl;
	if (ComHandler != nullptr) delete ComHandler;	// Delete transfer area and communication area
	std::cout << "pause 1" << std::endl;
	system("pause");

	return 0;
}

void PrintInfo(volatile  FISH_X1_TRANSFER* TransArea) {
	volatile FTX1_INPUT* MftX1in = &TransArea[ShmIfId_TXT::LOCAL_IO].ftX1in;
	volatile FTX1_OUTPUT* MftX1out = &TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out;
	volatile FTX1_CONFIG* MftX1config = &TransArea[ShmIfId_TXT::LOCAL_IO].ftX1config;
	volatile FTX1_STATE* MftX1state = &TransArea[ShmIfId_TXT::LOCAL_IO].ftX1state;

	bool MotReady = (MftX1in->motor_ex_reached[IdMotorA] == 1) && (MftX1in->motor_ex_reached[IdMotorB] == 1);
	bool CntReady = (MftX1in->cnt_resetted[IdCntA] == 1) && (MftX1in->cnt_resetted[IdCntB] == 1);

	int CntA = MftX1in->counter[IdCntA], CntB = MftX1in->counter[IdCntB];
	int CntCmdInA = MftX1in->cnt_reset_cmd_id[IdCntA], CntCmdInB = MftX1in->cnt_reset_cmd_id[IdCntB];
	int CntCmdOutA = MftX1out->cnt_reset_cmd_id[IdMotorA], CntCmdOutB = MftX1out->cnt_reset_cmd_id[IdMotorB];
	int MotCmdInA = MftX1in->motor_ex_cmd_id[IdMotorA], MotCmdInB = MftX1in->motor_ex_cmd_id[IdMotorB];
	int MotCmdOutA = MftX1out->motor_ex_cmd_id[IdMotorA], MotCmdOutB = MftX1out->motor_ex_cmd_id[IdMotorB];
	int MotDistA = MftX1out->distance[IdMotorA], MotDistB = MftX1out->distance[IdMotorB];
	int MotPowA1 = MftX1out->duty[2 * IdMotorA], MotPowB1 = MftX1out->duty[2 * IdMotorB];
	int MotPowA2 = MftX1out->duty[2 * IdMotorA + 1], MotPowB2 = MftX1out->duty[2 * IdMotorB + 1];

	cout << "*******************************************************************************" << endl;
	cout << " Counter      A =" << CntA << ", B = " << CntB << " Counters resetted = " << CntReady << endl;
	cout << " Distance     A =" << MotDistA << ", B = " << MotDistB << " Motors ready = " << MotReady << endl;
	cout << " Motor duty         [l,r] MotA = [" << MotPowA1 << ", " << MotPowA2 << "], MotB= [" << MotPowB1 << ", " << MotPowB2 << "]" << endl;
	cout << " Counter Command [in,out] MotA = [" << CntCmdInA << ", " << CntCmdOutA << "], MotB= [" << CntCmdInB << ", " << CntCmdOutB << "]" << endl;
	cout << " Motor Command   [in,out] MotA = [" << MotCmdInA << ", " << MotCmdOutA << "], MotB= [" << MotCmdInB << ", " << MotCmdOutB << "]" << endl;
	cout << "*******************************************************************************" << endl;
}



