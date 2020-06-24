///////////////////////////////////////////////////////////////////////////////
//
// File:    Main.cpp.h
//
// Project: ftPro - fischertechnik Control Graphical Programming System
//
// Module:  Camera - TXT C-Interface sample: receive camera images
//
// Author:  Michael Sögtrop
//
///////////////////////////////////////////////////////////////////////////////
//
// This sample program does the following:
// - Open connection to TXT interface with IP 192.168.7.2
// - Start camera server
// - Receive 20 frames, decode to YUV422 and save as YUV and JPEG
//   The missing EOI is fixed in images received from the ft camera
// - Stop camera server
//
///////////////////////////////////////////////////////////////////////////////

#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../Common/ftProInterface2013TransferAreaCom.h"
#include "../Common/ftProInterface2013JpegDecode.h"

FISH_X1_TRANSFER *TransArea;
ftIF2013TransferAreaComHandler *ComHandler;
std::string fnBase = "H:/Log/RoboProImg_";
using namespace std;

int main()
{
    // Create transfer area (for maximum number of extensions)
    TransArea = new FISH_X1_TRANSFER[IF08_MAX];

    // Create communication handler
    ComHandler = new ftIF2013TransferAreaComHandler( TransArea, IF08_MAX, "192.168.10.171" );

    // Initialize communication handler
    ComHandler->BeginTransfer();
    // Start camera.
    // Tested resolutions / frame rates for the ft-camera are 320x240@30fps and 640x480@15fps
 //   ComHandler->StartCamera( 320, 240, 30, 50 );
    ComHandler->StartCamera( 640, 480, 15, 50 );
	// Allocate yuv buffer (size must match the numbers given above!
  //  size_t yuvsize = 320*240*2;
	size_t yuvsize = 640 * 480 * 2;
	unsigned char *yuv = new unsigned char[yuvsize];

    // Loop for 20 frames
    int iLoop;
    clock_t prev = clock();
    for( iLoop=0; iLoop<20; iLoop++ )
    {
        unsigned char *buffer;
        size_t size;
		if (!ComHandler->GetCameraFrameJpeg(&buffer, &size))
		{
			cerr << "GetCameraFrameJpeg in Error " << endl;
			return -1;
	    }
		   ;
        clock_t now = clock();
        cout << "Received frame with " << size << " bytes in " << now-prev << " clocks" << endl;
        prev = now;

        if( size )
        {
            // Decode the JPEG to YUV422
            size_t bytes_read=0;
            if( ftProJpegDec( buffer, size, yuv, yuvsize, &bytes_read) )
            {
                // Write YUV file (typically YUV422 interleaved, depends on camera)
                std::ostringstream filenameC;
                filenameC << fnBase << iLoop << ".yuv";
                ofstream file( filenameC.str().c_str(), ofstream::binary | ofstream::trunc );
                file.write( (char*)yuv, yuvsize );
                file.close();
            }

            // Fix the missing EOI marker using the number of bytes read by the decoder
            size = bytes_read;
            buffer[size++] = 0xFF;
            buffer[size++] = 0xD9;

            // Write JPEG
            std::ostringstream filename;
            filename << fnBase << iLoop << ".jpg";
            ofstream file( filename.str().c_str(), ofstream::binary | ofstream::trunc );
            file.write( (char*)buffer, size );
            file.close();
        } 

        // Do some dummy transfer on the main socket. Otherwise it will close cause of a timeout.
        // GetVersion is the most lightweight command supported
        ComHandler->GetVersion();
    }

    // Clean up communication handler
    // Note: The main socket might close after a timeout when no transfers are done on the main socket.
    ComHandler->StopCamera();
    ComHandler->EndTransfer();

    // Delete transfer area and communication area
    delete ComHandler;
    delete [] TransArea;
    delete [] yuv;

    return 0;
}
