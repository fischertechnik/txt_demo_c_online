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
    ComHandler = new ftIF2013TransferAreaComHandler( TransArea, IF08_MAX, "192.168.7.2" );
    
    // Configure I2 of master and extension to ultrasonic
    TransArea[0].ftX1config.uni[1].mode = MODE_ULTRASONIC;
    TransArea[0].ftX1config.uni[1].digital = 0;
    TransArea[0].ftX1state.config_id ++;
    TransArea[1].ftX1config.uni[1].mode = MODE_ULTRASONIC;
    TransArea[1].ftX1config.uni[1].digital = 0;
    TransArea[1].ftX1state.config_id ++;

    // Initialize communication handler
    ComHandler->BeginTransfer();

    // Loop for 10 seconds
    int iLoop;
    for( iLoop=0; TransArea[0].IFTimer.Timer10ms<1000; iLoop++ )
    {
        // Set output 0 (O1) depending on input 0 (I1)
        // Note: for setting M1 to reverse direction, O2 (ftX1out.duty[1]) must be set
        // Note: the range of the motor duty cycle values is 0..512
        TransArea[0].ftX1out.duty[0] = TransArea[0].ftX1in.uni[0] ? 512 : 0;
        TransArea[1].ftX1out.duty[0] = TransArea[1].ftX1in.uni[0] ? 512 : 0;
        
        // Do the transfer. The transfer is done at most once every 10ms.
        if( !ComHandler->DoTransferCompressed() )
            break;

        // Print I/O values for master and extension, only every 4th IO (console IO is slow)
        if( iLoop%3 == 0 )
        {
            ComHandler->PrintIO( 0 );
            ComHandler->PrintIO( 1 );
        }
    }
    
    // This should be around 10s/10ms = 1000
    // The PrintIO might slow down things, so for maximum speed, remove the PrintIO
    cout << iLoop <<"transfers done" << endl;

    // Clean up communication handler
    ComHandler->EndTransfer();

    // Delete transfer area and communication area
    delete ComHandler;
    delete [] TransArea;

    return 0;
}
