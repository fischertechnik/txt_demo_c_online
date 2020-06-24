///////////////////////////////////////////////////////////////////////////////
//
// File:    ftProInterface2013TransferAreaCom.h
//
// Project: ftPro - fischertechnik Control Graphical Programming System
//
// Module:  Transfer area based communication protocol with interface 2013
//
// Author:  Michael Sögtrop
//
///////////////////////////////////////////////////////////////////////////////
//
// Urheberrecht im Sinne des Urheberrechtsgesetzes bei
//
// Michael Sögtrop
// Germany
//
// Copyright (C) 2015
//
///////////////////////////////////////////////////////////////////////////////
//
// TO DO:
//
///////////////////////////////////////////////////////////////////////////////
//
// Usage details for module ftProInterface2013TransferAreaCom
//
// ===== Thread safety =====
// 
// The transfer class is generally not thread safe. In multi threaded applications
// It is recommended to run the data transfer in a separate thread.
//
// There is one exception: GetCameraFrameJpeg uses a separate socket and can be
// called from a separate thread. So if the camera is used, it makes sense to
// decouple the camera and I/O transfer by using separate threads. If I/O and
// camera transfers are done from one thread, the I/O will be slow.
//
// ATTENTION: Start/StopCamera must be called from the main thread because they
// start the camera server by communicating over the main socket.
//
// ===== JPEG images don't have EOI and other camera bugs =====
//
// The ft camera has a bug which has the effect that returned JPEG frames don't have
// an EOI marker. An example function for decoding such frames using libjpeg is in
// ftProInterface2012DecodeJpeg.
// The camera also has a few other bugs, e.g. it seems to return 1280x720 JPEG frames
// If 1280x960 is requested, although it lists 1280x960 as supported.
//
///////////////////////////////////////////////////////////////////////////////


#include <winsock2.h>

/*

Camera

*/

extern "C" {
#include "common.h"
#include "FtShmemTxt.h"
}

// Double inclusion protection 
#if(!defined(ftProInterface2013TransferAreaCom_H))
#define ftProInterface2013TransferAreaCom_H

//******************************************************************************
//*
//* Class for handling transfer area based communication over a TCP/IP
//* connection to a 2013 interface
//*
//******************************************************************************

class ftIF2013TransferAreaComHandler
{
public:
    // Constructor
    //
    // transferarea = pointer to transfer area to which this transfer handler
    //                shall transfer data.
    //                The user must ensure, that this persists as long as the
    //                ftIF2013TransferAreaComHandler object is active.
    // nAreas       = number of areas (=master+extensions) to handle, max=IF08_MAX
    // name         = TCP/IP address or host name
    //                usually 192.168.7.2 USB, 192.168.8.2 for WLAN and 192.168.9.2 for Bluetooth
    // port         = TCP/IP port number, usually 65000
    ftIF2013TransferAreaComHandler( FISH_X1_TRANSFER *transferarea, int nAreas=1, const char *name="192.168.7.2", const char *port="65000" );

    // Destructor
    // If transfer not yet ended it is also running EndTransfer 
    ~ftIF2013TransferAreaComHandler();

    /// <summary>
    /// Set the transfer mode <br/>
    /// Default is Compressed.
    /// </summary>
    /// <param name="Compressed"> Simple mode =false: no compression only for master<br/> 
    /// Compression mode = true: works for both only master and master+ slave.</param>
    /// <remarks> since 2020-06-18 </remarks>
    /// <returns></returns>
    void SetTransferMode(bool Compressed);

    // Get Interface Version
    UINT32 GetVersion();

    // Open the TCP/IP channel and initialize the data transfer
    // Includes a UpdateConfig();
    bool BeginTransfer();

    // Update the I/O (e.g. universal input) configuration.
    // Can only be used after BeginTransfer()
     bool UpdateConfig();

  
    // Do an I/O transfer with compressed data transmission.
    // This mode is always faster and more reliable than the simple mode.
    // Note: transfers are automatically timed by the interface to once per 10ms
    // The interface sends the response 10ms after it send the previous response
    
    /// <summary>
    /// Do an I/O transfer.<br/>  
    /// For the mode <see cref="ftIF2013TransferAreaComHandler::SetTransferMode"></see>
    ///  <seealso cref="ftIF2013TransferAreaComHandler::DoTransferCompressed"></seeaslo>
    ///  <seeaslo  cref="ftIF2013TransferAreaComHandler::DoTransferSimple"></seeaslo>
    /// 
    /// Compressed mode is always faster and more reliable than the simple mode.
    /// Note: transfers are automatically timed by the interface to once per 10ms
    /// The interface sends the response 10ms after it send the previous response
    /// </summary>
    /// <remarks> since 2020-06-18 </remarks>
    /// <returns>successful </returns>
    bool DoTransfer();

    // Print the most important inputs and outputs to the console
    void PrintIO( int master_ext );

    // Close the TCP/IP channel
    void EndTransfer();

    // Start camera server
    // Tested resolutions/frame rates are
    //   160 x 120 @ 60fps (useful for closed loop control applications)
    //   320 x 240 @ 30fps
    //   640 x 480 @ 15fps (might lead to frame drops / distortions, especially over WiFi/BT)
    // Other resolutions might be supported, see "Resolutions.txt"
    // Many resolutions which are supported by the camera overload the TXT,
    // so there is no guarantee that any of these work!
    // Also the ft-camera seems to have some bugs, e.g. 1280x960 result in 1280x720.
    // More expensive cameras with large internal buffers might support higher resolutions.
    //
    // width         = requested frame width
    // height        = requested frame height
    // framerate     = requested frame rate in frames per second
    // powerlinefreq = Frequency of artificial illumination
    //                 This is required to adjust exposure to avoid flicker
    //                 Supported values are 50 and 60
    bool StartCamera( int width=320, int height=240, int framerate=15, int powerlinefreq=50, const char *port = "65001" );

    // Stop camera server
    void StopCamera();

    // Receive a JPG frame from the camera server
    // buffer = pointer to pointer to output buffer
    // buffersize = pointer to size of output buffer
    bool GetCameraFrameJpeg( unsigned char **buffer, size_t *buffersize );

protected:
    // Open a socket
    SOCKET OpenSocket( const char *port );

    // Set all universal input configurations to MODE_R, digital
    // Set all counter input configurations to normal (not inverted)
    // Set all motor output to dual (motor) output
    // This does just set the transfer area, but does not call UpdateCOnfig
    void SetDefaultConfig();

    // Update timer values in transfer area
    void UpdateTimers();

    // Stop all motors
    void StopMotors();

    // Do a transfer (uncompressed MASTER ONLY mode)
  // This function is mostly to illustrate the use of the simple uncompressed transfer mode e.g. for use in other languages.
  // It is recommended to use the compressed transfer mode.
  // Note: transfers are automatically timed by the interface to once per 10ms
  // The interface sends the response 10ms after it send the previous response
    bool DoTransferSimple();

    // Do an I/O transfer with compressed data transmission.
    // This mode is always faster and more reliable than the simple mode.
    // Note: transfers are automatically timed by the interface to once per 10ms
    // The interface sends the response 10ms after it send the previous response
    bool DoTransferCompressed();

protected:
    // Pointer to transfer area to which this transfer handler shall transfer data
    FISH_X1_TRANSFER * m_transferarea;
    // Number of areas (=master+extensions) to handle, max=IF08_MAX
    int m_nAreas;
    // TCP/IP address or host name
    const char *m_name;
    // Port number
    const char *m_port;
    // True if online mode is started
    bool m_online;
    // Times (in ms) when the corresponding timer was last updated
    long m_timelast[6];
    // TCP/IP communication socket handle
    SOCKET m_socket;
    // Info received from the device
    char m_info_devicetype[16];
    unsigned int m_info_version;

    // Data structures for compressed transfer
    bool IsCompressedMode = true; //default compressed mode
    size_t m_buffersize;
    struct ftIF2013Command_ExchangeDataCmpr *m_exchange_cmpr_command;
    struct ftIF2013Response_ExchangeDataCmpr *m_exchange_cmpr_response;
    class CompressionBuffer *m_comprbuffer;;
    class ExpansionBuffer *m_expbuffer;

    // Camera variables
    bool m_camerastarted;
    SOCKET m_camerasocket;
    size_t m_camerabuffersize;
    unsigned char *m_camerabuffer;
};

/*!
 * @brief I2C bus speed definitions
 * @remark TXT (2020-06-11) always runs in the 400kHz mode
 */
#define I2C_SPEED_100_KHZ       0  /*!< I2C bus clock speed */
#define I2C_SPEED_400_KHZ       1  /*!< I2C bus clock speed */

class ftIF2013TransferAreaComHandlerEx : public ftIF2013TransferAreaComHandler
{
protected:
   
public:
    ftIF2013TransferAreaComHandlerEx(FISH_X1_TRANSFER* transferarea, int nAreas = 1, const char* name = "192.168.7.2", const char* port = "65000") :
        ftIF2013TransferAreaComHandler(transferarea, nAreas, name, port)
    {


    };

    ~ftIF2013TransferAreaComHandlerEx() {
    
    };

 
};
#endif // ftProInterface2013TransferAreaCom_H