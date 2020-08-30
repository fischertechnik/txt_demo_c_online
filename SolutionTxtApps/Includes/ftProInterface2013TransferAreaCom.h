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
// Changes: 2020 - 08 - 25 C.van Leeuwen, Copyright (C) 2020
//          Add callbacks for CounterReset, Counter, Universal Input and
//          MotoReachedSteps like on the TX-C/Robo interface
//          Use log file like on the TX-C
//Changes: 2020 - 06 - 18 C.van Leeuwen, Copyright (C) 2020
//          It is now working for firmware 4.6.6. and 4.7.0 pre-release.
//          Reduce the size of the TA: max=2 TXT master + TXT slave01,
//          Add check in destructor to avoid double EndTransver
// Changes: 2020 - 06 - 24 C.van Leeuwen, Copyright (C) 2020
//          Choise for Simple or Compressed mode is now a setting
//          void SetTransferMode(bool Compressed);
//          Add class ftIF2013TransferAreaComHandlerEx2
// Changes: 2020 - 06 - 28 C.van Leeuwen, Copyright (C) 2020
//          Add TA communication Thread in class ftIF2013TransferAreaComHandlerEx2
//          int ftxStartTransferArea();
//          int ftxStopTransferArea();
//          bool TaComThreadIsRunning();
///////////////////////////////////////////////////////////////////////////////
// Usage details for module ftProInterface2013TransferAreaCom
//
// ===== Thread safety =====
// 
// The transfer class is generally not thread safe. In multi threaded applications
// It is recommended to run the data transfer in a separate thread.
// See TaComThread in class ftIF2013TransferAreaComHandlerEx2
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
#include <future>
#include <bitset>

#ifdef _LIB
#define  FtPro_API 
#else
#ifdef FTTXTLIB_EXPORTS
#define FtPro_API __declspec(dllexport)
#else
#define  FtPro_API __declspec(dllimport)
#endif
#endif


extern "C" {
#include "common.h"
#include "FtShmemTxt.h"

}
using namespace std;
using namespace fischertechnik::txt::ids;

// Double inclusion protection 
#if(!defined(ftProInterface2013TransferAreaCom_H))
#define ftProInterface2013TransferAreaCom_H

//******************************************************************************
//*
//* Class for handling transfer area based communication over a TCP/IP
//* connection to a 2013 interface
//*
//******************************************************************************
namespace fischertechnik {
	namespace txt {
		namespace remote {
			/// <summary>
			/// Struc to generated the callback's.
			/// DoTransfer (Compressed only) will set if their is a change, the event generator will reset after calling the callback
			/// </summary>
			struct InputChanged {
				bitset<ftIF2013_nUniversalInputs>      uniIn = { 0 };
				bitset<ftIF2013_nCounters>      cnt = { 0 };
				bitset<ftIF2013_nCounters>      counter = { 0 };
				bitset<ftIF2013_nMotorOutputs>      motorReached = { 0 };
				bitset<ftIF2013_nIRChannels + 1>      gameControlLeft = { 0 };
				bitset<ftIF2013_nIRChannels + 1>      gameControlRight = { 0 };
				bitset<ftIF2013_nIRChannels + 1>      gameControlButon = { 0 };
			};



			class FtPro_API ftIF2013TransferAreaComHandler
			{
			protected:
				/// <summary>
				/// Constructor
				/// </summary>
				/// <param name="transferarea">pointer to transfer area to which this transfer handler
				///               shall transfer data.
				///                The user must ensure, that this persists as long as the
				///                ftIF2013TransferAreaComHandler object is active. </param>
				/// <param name="nAreas"> number of areas (=master+extensions) to handle, max=IF_TXT_MAX (1 or 2) </param>
				/// <param name="name">TCP/IP address or host name
				///                usually 192.168.7.2 USB, 192.168.8.2 for WLAN and 192.168.9.2 for Bluetooth</param>
				/// <param name="port">TCP/IP port number, usually 65000</param>
				/// <param name="logMapName">directory where the library stores his log file</param>
				/// <param name="logLevel">The level of logging</param>
				ftIF2013TransferAreaComHandler(FISH_X1_TRANSFER* transferarea, int nAreas = 1, const char* name = "192.168.7.2", const char* port = "65000", const char* logMapName = ".\\", LogLevel logLevel = LogLevel::LvLOGERR);

				/// <summary>
				/// Destructor
				/// </summary>
				~ftIF2013TransferAreaComHandler();
			public:
				/// <summary>
				/// Set the transfer mode <br/>
				/// Default is Compressed.
				/// </summary>
				/// <param name="Compressed"> Simple mode =false: no compression only for master<br/> 
				/// Compression mode = true: works for both only master and master+ slave.</param>
				/// <remarks> since 2020-06-18<br/>  MicLog and MicLin do not work in the Simple mode</remarks>
				/// <returns></returns>
				void SetTransferMode(bool Compressed);

				// Get Interface Version
				uint32_t GetVersion();



			protected:
				// Open the TCP/IP channel and initialize the data transfer
				// Includes a UpdateConfig();
				bool BeginTransfer();

				// Update the I/O (e.g. universal input) configuration.
				// Can only be used after BeginTransfer()
				bool UpdateConfig();

				/// <summary>
				/// Do an I/O transfer.<br/>  
				/// For the mode <see cref="ftIF2013TransferAreaComHandler::SetTransferMode"></see>
				///  <seealso cref="ftIF2013TransferAreaComHandler::DoTransferCompressed"></seealso>
				///  <seealso  cref="ftIF2013TransferAreaComHandler::DoTransferSimple"></seealso>
				/// 
				/// Compressed mode is always faster and more reliable than the simple mode.
				/// Note: transfers are automatically timed by the interface to once per 10ms
				/// The interface sends the response 10ms after it send the previous response
				/// </summary>
				/// <remarks> since 2020-06-18 </remarks>
				/// <returns>successful </returns>
				virtual bool DoTransfer();
				// Close the TCP/IP channel

				void EndTransfer();


			public:
				/// <summary>
			///  Print the most important inputs and outputs to the console
			/// </summary>
			/// <param name="master_ext"></param>
				void PrintIO(ShmIfId_TXT master_ext);
				/// <summary>
				/// Print the IR info to the console
				/// </summary>
				/// <param name="master_ext"></param>
				void PrintIR(ShmIfId_TXT master_ext);



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
				bool StartCamera(int width = 320, int height = 240, int framerate = 15, int powerlinefreq = 50, const char* port = "65001");

				// Stop camera server
				void StopCamera();

				// Receive a JPG frame from the camera server
				// buffer = pointer to pointer to output buffer
				// buffersize = pointer to size of output buffer
				bool GetCameraFrameJpeg(unsigned char** buffer, size_t* buffersize);

			protected:
				// Open a socket
				SOCKET OpenSocket(const char* port);

				// Set all universal input configurations to MODE_R, digital
				// Set all counter input configurations to normal (not inverted)
				// Set all motor output to dual (motor) output
				// This does just set the transfer area, but does not call UpdateCOnfig
				void SetDefaultConfig();

				// Update timer values in transfer area
				void UpdateTimers();

				// Stop all motors
				 // Internal use only
				void StopMotors();

				// Do a transfer (uncompressed MASTER ONLY mode)
			  // This function is mostly to illustrate the use of the simple uncompressed transfer mode e.g. for use in other languages.
			  // It is recommended to use the compressed transfer mode.
			  // Note: transfers are automatically timed by the interface to once per 10ms
			  // The interface sends the response 10ms after it send the previous response
				virtual bool DoTransferSimple();

				// Do an I/O transfer with compressed data transmission.
				// This mode is always faster and more reliable than the simple mode.
				// Note: transfers are automatically timed by the interface to once per 10ms
				// The interface sends the response 10ms after it send the previous response
				virtual bool DoTransferCompressed();

				// Pointer to transfer area to which this transfer handler shall transfer data, both are pointing to the same array
				FISH_X1_TRANSFER* m_transferarea;
				FISH_X1_TRANSFER* FishX1Transfer;//// [IF08_MAX] in the 
				//Pointer 
				InputChanged *  m_inputChanged;

				// Number of areas (=master+extensions) to handle, max=IF08_MAX
				int m_nAreas;
				// TCP/IP address or host name
				const char* m_name;
				// Port number
				const char* m_port;
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
				struct ftIF2013Command_ExchangeDataCmpr* m_exchange_cmpr_command;
				struct ftIF2013Response_ExchangeDataCmpr* m_exchange_cmpr_response;
				class CompressionBuffer* m_comprbuffer;;
				class ExpansionBuffer* m_expbuffer;

				// Camera variables
				bool m_camerastarted;
				SOCKET m_camerasocket;
				size_t m_camerabuffersize;
				unsigned char* m_camerabuffer;
			};


			class FtPro_API ftIF2013TransferAreaComHandlerEx1 : public ftIF2013TransferAreaComHandler
			{
			protected:
				// I2C variables
				bool m_I2Cstarted;
				SOCKET m_I2Csocket;
				size_t m_I2Cbuffersize;
				unsigned char* m_I2Cbuffer;
				ftIF2013TransferAreaComHandlerEx1(FISH_X1_TRANSFER* transferarea, int nAreas = 1, const char* name = "192.168.7.2", const char* port = "65000", const char* logMapName = ".\\", LogLevel logLevel = LogLevel::LvLOGERR);
				~ftIF2013TransferAreaComHandlerEx1();
	
			public:
				/// <summary>
				/// Start the I2C communication
				/// However the communication protocol itself is not available
				/// </summary>
				/// <returns></returns>
				bool StartI2C();
				/// <summary>
				/// Stop the I2C communication
				/// However the communication protocol itself is not available
				/// </summary>
				/// <returns></returns>
				bool StopI2C();
			protected:
				/*!
				* @brief Function has not been documented
				* @param u16DevAddr        I2C device address,
				* @remarks                 for the moment only 7 bits I2C address are accepted.<br/>
				*                          10 bits address are giving a "not implemented" error
				* @param u16NumWrite       number of bytes to write<br/>
				* @remarks
				* @param pWriteData        pointer to write data (byte array)
				* @param u16NumRead        number of bytes to read
				* @param pReadData         pointer to read data (byte array)
				* @param u16Clock400kHz    u16Clock400kHz   or u16Clock100kHz
				* @remarks                 The TXT supports for the moment only 400kHz.
				* @return 0 for Success, non-zero for failure (ft-error code)
				*
				*/
				bool WriteI2C(UINT16 u16DevAddr, UINT16 u16NumWrite, UCHAR8* pWriteData);
				/*!
				 * @brief Function has not been documented<br/>
						  A WriteI2C needs to go before to select the device and (optional) a sub address.
				 * @param u16NumRead        number of bytes to read
				 * @param pReadData         pointer to read data (byte array)
				 * @param u16Clock400kHz    u16Clock400kHz   or u16Clock100kHz
				 * @remarks                 The TXT supports for the moment only 400kHz.
				 * @return 0 for Success, non-zero for failure (ft-error code)
				 *
				 */
				bool ReadI2C(UINT16 u16NumRead, UCHAR8* pReadData);
				bool WriteI2cByte();
				bool WriteI2cBlock();

				/*!
				* @brief Function has not been documented in FtShemTxt.h
				* @remark
				*     Adr, Anz Wr, Wr-Data, Anz Read, Rd-Data, Speed
				* @param u8DevAddr         I2C device address
				* @param u16NumWrite       number of bytes to write
				* @param pWriteData        pointer to write data (byte array)
				* @param u16NumRead        number of bytes to read
				* @param pReadData         pointer to read data (byte array)
				* @param u16Clock400kHz     u16Clock400kHz   or u16Clock100kHz
				* @return 0 for Success, non-zero for failure (ft-error code)
				*
				*/
				uint32_t KeLibI2cTransfer(UCHAR8 u8DevAddr, UINT16 u16NumWrite, UCHAR8* pWriteData, UINT16 u16NumRead, UCHAR8* pReadData, UINT16 u16Clock400kHz);
			};

		}//end namespace
	}//end namespace
}//end namespace
#endif // ftProInterface2013TransferAreaCom_H
