///////////////////////////////////////////////////////////////////////////////
//
// File:    Main.cpp
// version 1.1.3.1
// Project: ftPro - example of the TA-communication thread, simple keyboard control.
//                  basic use of the motor enchanced control
//
// Module:  TA-communication thread 
//
// Author:  2020-06-15 ing. C van Leeuwen Btw.
//              |  June 2020  bij [CvL] 
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


#include <iostream>
#include "conio.h"
//#include <thread>
#include <chrono>
//#include <future>
#include <time.h>
#include <fstream>
#include <sstream>
#include <mutex>
#include <map>


#include "../Common/ftProInterface2013TransferAreaCom.h"
#include "../Common/ftProInterface2013JpegDecode.h"

using namespace std;


const std::string MyIP = "192.168.10.171";
const std::string TaPort = "65000";

const  int IdMotorA = 0; //motor on M1
const  int IdMotorB = 1; //motor on M2
const  int IdMotorC = 2; //motor on M2
const int speedMax = 512, speedLow = 200;
const  int IdSensorIUltraA = 0;//distance sensor on I1
const  int counterA = 400, counterB = 400;


//end abriviations
ftIF2013TransferAreaComHandlerEx* ComHandler;


char input = ' ';
std::mutex TA_mutex;
bool stopWhile = false;


void PrintInfo(FISH_X1_TRANSFER* TransArea) {
	FTX1_INPUT MftX1in = TransArea[ShmIfId_TXT::LOCAL_IO].ftX1in;
	FTX1_OUTPUT MftX1out = TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out;
	FTX1_CONFIG MftX1config = TransArea[ShmIfId_TXT::LOCAL_IO].ftX1config;
	FTX1_STATE MftX1state = TransArea[ShmIfId_TXT::LOCAL_IO].ftX1state;

	bool MotReady = (MftX1in.motor_ex_reached[IdMotorA] == 1) && (MftX1in.motor_ex_reached[IdMotorB] == 1);
	bool CntReady = (MftX1in.cnt_resetted[IdMotorA] == 1) && (MftX1in.cnt_resetted[IdMotorB] == 1);

	int CntA = MftX1in.counter[IdMotorA], CntB = MftX1in.counter[IdMotorB];
	int CntCmdInA = MftX1in.cnt_reset_cmd_id[IdMotorA], CntCmdInB = MftX1in.cnt_reset_cmd_id[IdMotorB];
	int CntCmdOutA = MftX1out.cnt_reset_cmd_id[IdMotorA], CntCmdOutB = MftX1out.cnt_reset_cmd_id[IdMotorB];
	int MotCmdInA = MftX1in.motor_ex_cmd_id[IdMotorA], MotCmdInB = MftX1in.motor_ex_cmd_id[IdMotorB];
	int MotCmdOutA = MftX1out.motor_ex_cmd_id[IdMotorA], MotCmdOutB = MftX1out.motor_ex_cmd_id[IdMotorB];
	int MotDistA = MftX1out.distance[IdMotorA], MotDistB = MftX1out.distance[IdMotorB];
	cout << "*******************************************************************************" << endl;
	cout << " Counter A      =" << CntA << ", B = " << CntB << " Counters resetted = " << CntReady << endl;
	cout << " Distance A     =" << MotDistA << ", B = " << MotDistB << " Motors ready = " << MotReady << endl;
	cout << " Counter Command [in,out] MotA = [" << CntCmdInA << ", " << CntCmdOutA << "], MotB= [" << CntCmdInB << ", " << CntCmdOutB << "]" << endl;
	cout << " Motor Command   [in,out] MotA = [" << MotCmdInA << ", " << MotCmdOutA << "], MotB= [" << MotCmdInB << ", " << MotCmdOutB << "]" << endl;
	cout << "*******************************************************************************" << endl;
}
int motorSteps = 0;
int counterSteps = 0;
int main()
{

	FISH_X1_TRANSFER* TransArea = new FISH_X1_TRANSFER[IF_TXT_MAX];
	// Example is using the master, some abrviations. Optional use.
	//THis to show different way to address the data with a pointer.
	FTX1_INPUT* MftX1in = & TransArea[ShmIfId_TXT::LOCAL_IO].ftX1in;
	FTX1_OUTPUT * MftX1out = &TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out;
	UINT16* MftX1outDis = (&TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out)->distance;
		//or  MftX1out->distance;
	INT16 * MftX1outDut = MftX1out->duty;
	FTX1_CONFIG* MftX1config = & TransArea[ShmIfId_TXT::LOCAL_IO].ftX1config;
	FTX1_STATE*  MftX1state = & TransArea[ShmIfId_TXT::LOCAL_IO].ftX1state;
	// Configure sensorId of master and extension to ultrasonic
	MftX1config->uni[IdSensorIUltraA].mode = MODE_ULTRASONIC;//type
	MftX1config->uni[IdSensorIUltraA].digital = 0;//on I1
	MftX1config->cnt[IdMotorA].mode = 1; //As full bridge (M functionality) (default)
	MftX1config->cnt[IdMotorB].mode = 1; //As full bridge (M functionality)(default)
	MftX1config->cnt[IdMotorC].mode = 0; //As two half  bridges (O functionality)
	//Renew the settings
	MftX1state->config_id++;
	//start the handler, includes update configuration.

	ComHandler = new ftIF2013TransferAreaComHandlerEx(TransArea, IF_TXT_MAX, MyIP.c_str(), TaPort.c_str());
	if (ComHandler == nullptr) {
		cerr << "Main: ComHandler nullptr" << endl;
		return -3;
	}
	ComHandler->SetTransferMode(true);

	int res=ComHandler->TaComThreadStart();
	PrintInfo(TransArea);
	TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.motor_ex_cmd_id[IdMotorA]++;
	TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.motor_ex_cmd_id[IdMotorB]++;
	TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.cnt_reset_cmd_id[IdMotorA]++;
	TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.cnt_reset_cmd_id[IdMotorB]++;

	bool ready1;
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(4));
		ready1 = (MftX1in->cnt_resetted[IdMotorA] == 1) && (MftX1in->cnt_resetted[IdMotorB] == 1);

	} while (!ready1);
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		ready1 = (MftX1in->motor_ex_reached[IdMotorA] == 1) && (MftX1in->motor_ex_reached[IdMotorB] == 1);

	} while (!ready1);
	PrintInfo(TransArea);
	/*********************************************************************************/
		// ready to start the main loop
	int iLoop = 0;
	printf("Two encoder motors [on M1/C1 and M2/C2] \n");

	while (!stopWhile)
	{
		try {
			printf("Typed  [t=exit program ] \n");
			printf("Typed  [for normal motor use:a, s, d, w, h=stop motors ] \n");
			printf("Typed  [for enhanced use: x, c and v =  experiment, q=exit exhanced, z=print status info, b=reset counters] \n");
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
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA] = 0;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA + 1] = speedMax;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB] = 0;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB + 1] = speedMax;
				input = ' ';
				break;

			case 'w':
				//MotorA=cw and MotorB=cw

				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA] = speedMax;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA + 1] = 0;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB] = speedMax;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB + 1] = 0;
				input = ' ';
				break;

			case 'd':
				//MotorA=ccw and MotorB=cw
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA] = 0;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA + 1] = speedLow;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB] = speedLow;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB + 1] = 0;
				input = ' ';
				break;


			case 'a':
				//MotorA=cw and MotorB=ccw
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA] = speedLow;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA + 1] = 0;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB] = 0;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB + 1] = speedLow;
				input = ' ';
				break;

			case 'h':
				//MotorA=stop and MotorB=stop
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA] = 0;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA + 1] = 0;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB] = 0;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorB + 1] = 0;
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
				MftX1in->cnt_resetted[IdMotorA] = 0;//needs to be resetted by the user
				MftX1in->cnt_resetted[IdMotorB] = 0;//needs to be resetted by the user
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.cnt_reset_cmd_id[IdMotorA]++;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.cnt_reset_cmd_id[IdMotorB]++;
				bool ready3;
				do {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));//avoid blocking
					ready3 = (MftX1in->cnt_resetted[IdMotorA] == 1) && (MftX1in->cnt_resetted[IdMotorB] == 1);

				} while (!ready3);
				break;

			case 'x':
				//Enhanced motor step 1: set values
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA] = 256;
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.duty[2 * IdMotorA + 1] = 0;
				//Different ways of address the TA fields
				MftX1outDut[2 * IdMotorB] = 256;  //
				MftX1outDut[2 * IdMotorB + 1] = 0;
				MftX1outDis[IdMotorA] = counterB;
				MftX1outDis[IdMotorB] = counterA;
				//set motorA as master for motorB
				TransArea[ShmIfId_TXT::LOCAL_IO].ftX1out.master[IdMotorB] = IdMotorA + 1;			// synchro with Motor A as master

				motorSteps = 1;	input = ' ';
				break;
			case 'c':
				//Enhanced motor step 2: start the black box operation
				// Reset both the "motor_ex_reached" and "cnt_resetted",
				// this because they are used during this operation.
				MftX1in->motor_ex_reached[IdMotorA] = 0;  //needs to be resetted by the user
				MftX1in->motor_ex_reached[IdMotorB] = 0;//needs to be resetted by the user
				MftX1in->cnt_resetted[IdMotorA] = 0;//needs to be resetted by the user
				MftX1in->cnt_resetted[IdMotorB] = 0;//needs to be resetted by the user

				MftX1out->motor_ex_cmd_id[IdMotorA]++;  //send start command
				MftX1out->motor_ex_cmd_id[IdMotorB]++;
				motorSteps = 2;		input = ' ';
				break;
			case 'v':
				//Enhanced motor step 3: wait until ready and run again.
				MftX1outDis[IdMotorA] += (MftX1outDis[IdMotorA] >160)? -150:+480;
				MftX1outDis[IdMotorB] += (MftX1outDis[IdMotorB] > 160) ? -150 : +480;
		           std::this_thread::sleep_for(std::chrono::milliseconds(600));
			        // Reset both the "motor_ex_reached" and "cnt_resetted",
				    // this because they are used during this operation.
					MftX1in->motor_ex_reached[IdMotorA] = 0; //needs to be resetted by the user
					MftX1in->motor_ex_reached[IdMotorB] = 0;//needs to be resetted by the user
					MftX1in->cnt_resetted[IdMotorA] = 0;//needs to be resetted by the user
					MftX1in->cnt_resetted[IdMotorB] = 0;//needs to be resetted by the user

					MftX1out->motor_ex_cmd_id[IdMotorA]++;
					MftX1out->motor_ex_cmd_id[IdMotorB]++;
					motorSteps = 3;	input = ' ';
				break;

				//look what the progress is
			case 'z':
				PrintInfo(TransArea);
				break;



			case 'q'://reset (stop) exsteded motor control
									
					MftX1out->duty[2 * IdMotorA] = 0;
					MftX1out->duty[2 * IdMotorA + 1] = 0;
					MftX1out->duty[2 * IdMotorB] = 0;
					MftX1out->duty[2 * IdMotorB + 1] = 0;
					MftX1out->distance[IdMotorA] = 0;
					MftX1out->distance[IdMotorB] = 0;
					MftX1out->master[IdMotorB] = 0; //0 no master slave relation

					MftX1out->motor_ex_cmd_id[IdMotorA]++;
					MftX1out->motor_ex_cmd_id[IdMotorB]++;
					boolean ready2;
					do {
						std::this_thread::sleep_for(std::chrono::milliseconds(10));//avoid blocking
						ready2 = (MftX1in->motor_ex_reached[IdMotorA] == 1) && (MftX1in->motor_ex_reached[IdMotorB] == 1);

					} while (!ready2);
				
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
				
				break;
			case 'l':
				
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
				//ComHandler->PrintIO(ShmIfId_TXT::LOCAL_IO);
				//ComHandler->PrintIO(ShmIfId_TXT::REMOTE_IO_1);
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
		ComHandler->TaComThreadStop();
	std::cout << "Thread has been Stopped" << std::endl;

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	if (ComHandler != nullptr) delete ComHandler;	// Delete transfer area and communication area
	if (TransArea != nullptr)  delete[] TransArea;
	std::cout << "pause" << std::endl;
	system("pause");

	return 0;
}





