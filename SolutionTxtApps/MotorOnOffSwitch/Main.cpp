///////////////////////////////////////////////////////////////////////////////
//
// File:    Main.cpp.h
//
// Project: MotorOnOffSwitch - fischertechnik Control Graphical Programming System
//
// Module:  MotorOnOffSwitch - TXT C++-Interface sample: switch motor on off
//
// Author:  Michael Sögtrop
// Modified : 2020-06-15 (c) TesCaWeb ing.C van Leeuwen Btw.
///////////////////////////////////////////////////////////////////////////////
//
// This sample program does the following:
// - Open connection to TXT interface 
// - Configure one input to Ultrasonic
// - Start transfer
// - switch motor 1 of master on/off depending on I1 of master
// - switch motor 1 of extension on/off depending on I1 of extension
// - do this for 5 seconds in a loop and show I/O in each transfer
//
// The typical IP addresses / hostnames are:
// - 192.168.7.2 : online via USB
// - 192.168.8.2 : online via WLAN
// - 192.168.9.2 : online via Bluetooth
// - localhost   : download mode
// 
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "conio.h"
// #include <thread>
#include <chrono>
//#include <future>

#include "FtTxtLib.h"
using namespace std;
using  fischertechnik::txt::remote::api::ftIF2013TransferAreaComHandlerEx2;
using  namespace fischertechnik::txt::ids;
using  namespace fischertechnik::txt::remote;



ftIF2013TransferAreaComHandlerEx2* ComHandler;
const std::string MyIP = "192.168.10.171";
const std::string TaPort = "65000"; 
const std::string logMap = "H:\\workspaceVS\\txt_demo_c_online.git\\SolutionTxtApps\\";

Motor motorId = Motor::M1; //motor M1
Input sensorId =Input::I6;//distance sensor on I6
 

int main()
{
    // Create transfer area (for maximum number of extensions)
     // Create communication handler
    ComHandler = new ftIF2013TransferAreaComHandlerEx2( IF_TXT_MAX, MyIP.c_str(), TaPort.c_str(), logMap.c_str(), LogLevel::LvLOG);
    if (ComHandler == nullptr) {
        cerr << "Main: ComHandler nullptr" << endl;
        return -3;
    }
    ComHandler->SetTransferMode(true);
    volatile FISH_X1_TRANSFER* TransArea = ComHandler->GetTransferAreasArrayAddr();

    cout << "Main: ComHandler creation done=" << endl;
    // Configure I2 of master and extension to ultrasonic
    ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, I1, InputMode::MODE_R, true);
    ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, sensorId, MODE_ULTRASONIC, false);
    //ComHandler->SetFtUniConfig(ShmIfId_TXT::REMOTE_IO_1, sensorId, MODE_ULTRASONIC, false);
    // Initialize communication handler
    if (!ComHandler->ftxStartTransferArea()==0)
    {
        cout << "Main: Error: BeginTransfer" << endl;
        do
        {
            cerr << '\n' << "Press a key to continue...";
        } while (cin.get() != '\n');
        return false;
    }
    // Loop for 10 seconds
    int iLoop;
    // for( iLoop=0; iLoop<10; iLoop++ )
    for (iLoop = 0; TransArea[0].IFTimer.Timer10ms < 500; iLoop++)
    {
        // Set output 0 (O1) depending on input 0 (I1)
        INT16 dis;bool  overrun;
        ComHandler->GetInIOValue(ShmIfId_TXT::LOCAL_IO, sensorId, dis, overrun);
        ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, motorId, dis > 25 ? 512 : -200);
        /*  //alternatief   
         if (dis > 10) {
            ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, motorId,  512, CW);
        }
        else {
            ComHandler->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, motorId, 150, CCW);
        }
        */
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
     
        if (!ComHandler->ftxIsTransferActiv())
        {
            cerr << "Transfer area communication thread is not running" << endl;	break;
        }

 
        // Print I/O values for master and extension, only every 4th IO (console IO is slow)
        if (iLoop % 10 == 0)
        {
            ComHandler->PrintIO(ShmIfId_TXT::LOCAL_IO);
            //ComHandler->PrintIO(ShmIfId_TXT::REMOTE_IO_1);
        }
    }

       cout << endl << "Main: normal end, transfers done " << endl;

    // Clean up communication handler
    ComHandler->ftxStopTransferArea();
    if (ComHandler != nullptr) delete ComHandler;
    
    do
    {
        cout << '\n' << "Press a key to continue...";
    } while (cin.get() != '\n');
    return 0;
}

