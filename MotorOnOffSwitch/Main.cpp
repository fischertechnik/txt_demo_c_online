///////////////////////////////////////////////////////////////////////////////
//
// File:    Main.cpp.h
//
// Project: ftPro - fischertechnik Control Graphical Programming System
//
// Module:  MotorOnOffSwitch - TXT C-Interface sample: switch motor on off
//
// Author:  Michael Sögtrop
//
///////////////////////////////////////////////////////////////////////////////
//
// This sample program does the following:
// - Open connection to TXT interface with IP 192.168.7.2
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
//changes:
// 2018-09-18: C van Leeuwen
//             Some changes to let it work for TXT firmware 4.2.4 and 4.4.3. 
//       
//
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "../Common/ftProInterface2013TransferAreaCom.h"

using namespace std;

FISH_X1_TRANSFER *TransArea;
ftIF2013TransferAreaComHandler *ComHandler;

int main()
{
    // Create transfer area (for maximum number of extensions)
    TransArea = new FISH_X1_TRANSFER[IF08_MAX];
    
    // Create communication handler
    ComHandler = new ftIF2013TransferAreaComHandler( TransArea, IF08_MAX, "192.168.10.160","65000" );
	cout << "Main: ComHandler creation done=" << endl;
		int motorId = 0; //motor M1
		int sensorId = 5;//distance sensor on I6
    // Configure I2 of master and extension to ultrasonic
    TransArea[ShmIfId::LOCAL_IO].ftX1config.uni[sensorId].mode = MODE_ULTRASONIC;
    TransArea[ShmIfId::LOCAL_IO].ftX1config.uni[sensorId].digital = 0;
    TransArea[ShmIfId::LOCAL_IO].ftX1state.config_id ++;
    TransArea[ShmIfId::REMOTE_IO_1].ftX1config.uni[sensorId].mode = MODE_ULTRASONIC;
    TransArea[ShmIfId::REMOTE_IO_1].ftX1config.uni[sensorId].digital = 0;
    TransArea[ShmIfId::REMOTE_IO_1].ftX1state.config_id ++;

    // Initialize communication handler
	if (!ComHandler->BeginTransfer())
	{
		cout << "Main: Error: BeginTransfer" << endl;
		do
		{
			cerr << '\n' << "Press a key to continue...";
		} while (cin.get() != '\n');
		return false;
    }
	cout << "Main: BeginTransfer done" << endl;
	// Loop for 10 seconds
    int iLoop;
    // for( iLoop=0; iLoop<10; iLoop++ )
    for( iLoop=0; TransArea[0].IFTimer.Timer10ms<500; iLoop++ )
	{
        // Set output 0 (O1) depending on input 0 (I1)
        TransArea[ShmIfId::LOCAL_IO].ftX1out.duty[motorId*2] = TransArea[0].ftX1in.uni[sensorId]>10 ? 512 : 0;
 		TransArea[ShmIfId::REMOTE_IO_1].ftX1out.duty[motorId * 2] = TransArea[0].ftX1in.uni[sensorId] >10 ? 512 : 0;

        // Do the transfer. The transfer is done at most once every 10ms.
 		
		cout << "Main: DoTransferxxx" << endl;
       if( !ComHandler->DoTransferCompressed() )
 		{ cerr << "Main: Error DoTransferCompressed break" << endl;	break; }
//		if (!ComHandler->DoTransferSimple())
//		{ 	cerr << "Main: Error DoTransferSimple break" << endl;	break;}
			
		cout << "Main: DoTransferxxxx was Ok" << endl;

        // Print I/O values for master and extension, only every 4th IO (console IO is slow)
        if( iLoop%3 == 0 )
        {
            ComHandler->PrintIO(ShmIfId::LOCAL_IO);
            ComHandler->PrintIO(ShmIfId::REMOTE_IO_1);
        }
    }
    
    // This should be around 10s/10ms = 1000
    // The PrintIO might slow down things, so for maximum speed, remove the PrintIO
    cout <<endl<< "Main: normal end, transfers done iLoop=" << iLoop <<endl;

    // Clean up communication handler
    ComHandler->EndTransfer();

    // Delete transfer area and communication area
    delete ComHandler;
    delete [] TransArea;
	do
	{
	  cout << '\n' << "Press a key to continue...";
	} while (cin.get() != '\n');
    return 0;
}
