///////////////////////////////////////////////////////////////////////////////
//
// File:    Main.cpp.h
// Project: ftPro - fischertechnik Control Graphical Programming System
// Module:  Camera - TXT C-Interface sample: receive camera images
//
// 2020-06-15 (c) TesCaWeb ing.C van Leeuwen Btw.
//        based on the Camera example from Mr. Michael Sögtrop
//
///////////////////////////////////////////////////////////////////////////////
// The main objective for the example is to show that the model control 
// and the image processing can run in differt thread (parallel)
// This sample program does the following:
// - Open connection to TXT interface 
// - Start the communication thread and the camera thread
// - Combine a Model control loop and a camera
// - Stop camera server
// - The image processing is very simple, only saving the image.
// - However the image processing could be extended in a way that 
//   it produce control info for the model
///////////////////////////////////////////////////////////////////////////////

#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "conio.h"
#include <thread>

#include "FtTxtLib.h"
#include <cassert>

using namespace std;
using  fischertechnik::txt::remote::api::ftIF2013TransferAreaComHandlerEx2;
using  namespace fischertechnik::txt::ids;
ftIF2013TransferAreaComHandlerEx2* ComHandler;

const std::string MyIP = "192.168.7.2";//"192.168.10.171";
const std::string TaPort = "65000";
const std::string logMap = "H:\\workspaceVS\\txt_demo_c_online.git\\SolutionTxtApps\\";//map for the internal log file

																					   //model control related stuff
const  Motor IdMotorA = Motor::M1;
const   Motor IdMotorB = Motor::M2;
void ModelControl(Motor IdMotorA, Motor IdMotorB);

//Camera thread related stuff
std::string fnBase = "H:/Log/RoboProImg_";
std::promise<void>  exitSignal;// Create a std::promise object
std::future<void>  futureObj;// = exitSignal.get_future();
std::thread   threadCamera = thread();
void thread_Camera(std::future<void> futureObj);



int main()
{
	// Create transfer area (for maximum number of extensions)

	// Create communication handler
	ComHandler = new ftIF2013TransferAreaComHandlerEx2(IF_TXT_MAX, MyIP.c_str(), TaPort.c_str(), logMap.c_str(), LogLevel::LvLOG);
	//volatile FISH_X1_TRANSFER* TransAreax = ComHandler->GetTransferAreasArrayAddr();
	ComHandler->SetTransferMode(true);
	// Initialize TXT and start communication thread
	// It looks like that the communication thread needs to be active to have image transfer running.
	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorA, true);//As full bridge (M functionality) (default)
	ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorA, true); //As full bridge (M functionality)(default)
	ComHandler->ftxStartTransferArea();
	cout << "Version " << ComHandler->GetVersion() << endl;

	//Camera thread related example
	//Fetch std::future object associated with promise
	futureObj = exitSignal.get_future();
	// Starting Thread & move the future object in lambda function by reference
	//https://stackoverflow.com/questions/10673585/start-thread-with-member-function
	threadCamera = std::thread([=] {thread_Camera(std::move(futureObj)); });

	//Example for the model control logic
	//If the camera thread produce control info, then this info can be used here
	ModelControl(IdMotorA, IdMotorB);

	std::cout << "Asking Thread to Stop" << std::endl;
	exitSignal.set_value();
	threadCamera.join();

	// Delete transfer area and communication area
	ComHandler->ftxStopTransferArea();
	delete ComHandler;//or    ComHandler->~ftIF2013TransferAreaComHandlerEx2();

	return 0;
}



/// <summary>
/// Camera thead worker
/// </summary>
/// <returns></returns>
boolean GetImage(int iLoop, clock_t& prev) {

	// Allocate yuv buffer (size must match the numbers given above!
   //size_t yuvsize = 320*240*2;
	size_t yuvsize = 640 * 480 * 2;
	unsigned char* yuv = new unsigned char[yuvsize];

	unsigned char* buffer;
	size_t size=0, size1=0;
	clock_t now1 = clock(), now2=0L, now3=0L;
	if (!ComHandler->GetCameraFrameJpeg(&buffer, &size))
	{
		cerr << "GetCameraFrameJpeg in Error " << endl;
		return false;
	}
	else {

		if (size!=0)
		{
			
			// Decode the JPEG to YUV422
			size_t bytes_read = 0;
			if (ftProJpegDec(buffer, size, yuv, yuvsize, &bytes_read))
			{
				// Write YUV file (typically YUV422 interleaved, depends on camera)
				std::ostringstream filenameC;
				filenameC << fnBase << iLoop << "_T.yuv";
				ofstream file(filenameC.str().c_str(), ofstream::binary | ofstream::trunc);
				file.write((char*)yuv, yuvsize);
				file.close();
			}

			// Fix the missing EOI marker using the number of bytes read by the decoder
			 size1 = bytes_read;
			buffer[size1++] = 0xFF;
			buffer[size1++] = 0xD9;

			// Write JPEG
			std::ostringstream filename;
			filename << fnBase << iLoop << "_T.jpg";
			ofstream file(filename.str().c_str(), ofstream::binary | ofstream::trunc);
			file.write((char*)buffer, size);
			file.close();
		}
	}
	 now3 = clock();
	cout << "Received and  processed in "  << now3-now1 << " cl;  time between "<< now1- prev << " cl." << endl;
	prev = now3;
	delete[] yuv;
	// Clean up communication handler
	// Note: The main socket might close after a timeout when no transfers are done on the main socket.
   // ComHandler->StopCamera();
	return true;
}
/// <summary>
/// Camera thread
/// </summary>
/// <param name="futureObj"></param>
void thread_Camera(std::future<void> futureObj) {
	std::cout << "Thread Start" << std::endl;

	bool stop = false;
	// Start camera.
   // Tested resolutions / frame rates for the ft-camera are 320x240@30fps and 640x480@15fps
   // ComHandler->StartCamera( 320, 240, 30, 50 );
	if (!ComHandler->StartCamera(640, 480, 15, 50))
	{
		cerr << "thread_Camera: Error: StartCamera" << endl;
		stop = true;
	}
	cout << "thread_Camera: StartCamera done" << endl;
	//start loop
	clock_t prev = clock(); int count = 0;
	while (!stop && (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout))
	{
		if (!GetImage(count, prev))
		{
			cerr << "thread_Camera: Error GetImages break" << endl;	stop = true;
		}
		else cout << "thread_Camera:Transfer " << stop << endl;
		count++;

		std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for a little to prevent blocking
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 
	// Clean up communication handler
	// Note: The main socket might close after a timeout when no transfers are done on the main socket.
	cerr << "thread_Camera, StopCamera" << endl;
	ComHandler->StopCamera();
	std::cout << "thread_Camera: Thread End" << std::endl;
};

void printCoreInfo();
size_t NumberOfPhysicalCores() noexcept;
size_t NumberOfSystemCores() noexcept;
/// <summary>
/// 
/// </summary>
/// <param name="IdMotorA"></param>
/// <param name="IdMotorB"></param>
void ModelControl(Motor IdMotorA, Motor IdMotorB)
{
	bool stopWhile = false;
	const int speedMax = 512, speedLow = 200;
	const  int counterA = 400, counterB = 400;

	//Todo: put here the control of your model.
	while (!stopWhile)
	{
		char input = ' ';

		try {
			printf("Typed  [t=exit program ] \n");
			printf("Typed  [for normal motor use: a, s, d, w, h=stop motors ] \n");
			input = _getch();//Keyboard is blocking
			printf("You  typed: %c!\n", input);

			switch (input)
			{
				//*****************************************************************
				// This part shows the basics of the normal motor use
				//******************************************************************
				// s,w,d and a: does not work when the motor is in the extended mode
				// h is halt
			case 's':
				//MotorA=ccw and MotorB=ccw
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorA, speedMax, Direction::CCW);
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorB, speedMax, Direction::CCW);
				input = ' ';
				break;
			case 'w':
				//MotorA=cw and MotorB=cw
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorA, speedMax, Direction::CW);
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorB, speedMax, Direction::CW);
				input = ' ';
				break;
			case 'd':
				//MotorA=ccw and MotorB=cw
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorA, speedLow, Direction::CCW);
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, IdMotorB, speedLow, Direction::CW);
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
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, Motor::M1, 0, Direction::Halt);
				ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, Motor::M2, 0, Direction::Halt);
				input = ' ';
				break;
			case 'Z':
				printCoreInfo();

				input = ' ';
				break;



			case 't':
				/*****************************************************************
				* Exit loop                                                      *
				*******************************************************************/
				input = ' ';
				stopWhile = true;
				break;
			default:
				input = ' ';
				break;
			}
		}
		catch (exception& ex) {
			cout << ex.what() << '\n';
			stopWhile = true;
		}//end try
	} //end while
	printf("End Motor Control \n");

}

void printCoreInfo() {
				const auto processor_count = std::thread::hardware_concurrency();

				SYSTEM_INFO sysinfo;
				GetSystemInfo(&sysinfo);
				int numCPU = sysinfo.dwNumberOfProcessors;
				printf("Number of processors=%d \n", numCPU);
				printf("PhysicalCores num=%d \n", NumberOfPhysicalCores());
				printf("SystemCores num=%d \n", NumberOfSystemCores());

}

size_t NumberOfPhysicalCores() noexcept {

	DWORD length = 0;
	const BOOL result_first = GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length);
	assert(GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	std::unique_ptr< uint8_t[] > buffer(new uint8_t[length]);
	const PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info =
		reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get());

	const BOOL result_second = GetLogicalProcessorInformationEx(RelationProcessorCore, info, &length);
	assert(result_second != FALSE);

	size_t nb_physical_cores = 0;
	size_t offset = 0;
	do {
		const PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX current_info =
			reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get() + offset);
		offset += current_info->Size;
		++nb_physical_cores;
	} while (offset < length);

	return nb_physical_cores;
}
size_t NumberOfSystemCores() noexcept {
	SYSTEM_INFO system_info;
	ZeroMemory(&system_info, sizeof(system_info));

	GetSystemInfo(&system_info);

	return static_cast<size_t>(system_info.dwNumberOfProcessors);
}