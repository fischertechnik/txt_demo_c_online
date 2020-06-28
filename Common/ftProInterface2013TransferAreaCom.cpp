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
// Implementation details for module ftProInterface2013TransferAreaCom
//
// see also:
///////////////////////////////////////////////////////////////////////////////
// Changes: 2018-09-18 C .van Leeuwen
//          Add some firmware 4.2.4 variable to the communication.
//          It is now working for firmware 4.4.3.
//          Made some adjust so that the Simple mode is working too.
//Changes:  2019 - 06 - 18 C.van Leeuwen
//          Commpressed mode is working, update of the TA
// Changes: 2020 - 06 - 18 C.van Leeuwen
//          It is now working for firmware 4.6.6. and 4.7.0 pre-release.
//          Reduce the size of the TA: max=2 TXT master + TXT slave01,
//          Add check in destructor to avoid double EndTransver
//          
///////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <memory.h>
#include <time.h>
#include <thread>
#include <type_traits>
#include <chrono>

#include "ftProInterface2013TransferAreaCom.h"
#include "ftProInterface2013SocketCom.h"



#pragma comment(lib, "Ws2_32.lib")

using namespace std;

//******************************************************************************
//*
//* Utility functions
//*
//******************************************************************************

// Send a command to a socket
// This is not part of the ftIF2013TransferAreaComHandler class to avoid
// dependence on the command and response structures

bool SendCommand( SOCKET socket, const struct ftIF2013Command_Base *commanddata, size_t commandsize, enum ftIF2013ResponseId responseid, struct ftIF2013Response_Base *responsedata, size_t responsesize )
{
#ifdef TEST
	//wat is char voor een type: 8 bits    std::hex <<
	cout << "SendCommand 1: start sending command size= "<< std::dec << commandsize << endl;
#endif	
    int result;
    size_t nRead=0;
    char *pos = (char*) responsedata; 
   result = send( socket, (const char*)commanddata, commandsize, 0 );
 	if (result != commandsize)
    {
        cerr << "SendCommand 1: Error sending command" << endl;
        return false;
    }
#ifdef TEST
	cout << "SendCommand 1: succes sending command" << endl;
#endif

    while( nRead < responsesize )
    {
        result = recv(socket, pos, responsesize-nRead, 0 );
        // result 0 means socket closed, <0 means error, otherwise #bytes received
        if( result <= 0 )
		{
			cerr << "SendCommand 1: Error receiving response last error:" << std::dec << WSAGetLastError() << " :";
			switch (WSAGetLastError())
			{//https://support.microsoft.com/en-us/kb/811087
				//http://answers.microsoft.com/en-us/windows/forum/windows_10-other_settings/socket-10060-error-connection-timed-out-in-windows/586c2f6c-393d-40f0-b933-3aba910421d2
			case WSAETIMEDOUT: cerr << "WSAETIMEDOUT: result=0x" << std::hex << result << endl;break;
			case WSAECONNABORTED: cerr << "WSAECONNABORTED: result=0x" << std::hex << result << endl;break;
			default:cerr  <<" result=0x" << std::hex << result<<endl;
				break;
			}
           
			//WSACleanup();
			return false;
        }
#ifdef TEST
		cout << "SendCommand 1: succes receiving response nread=0x" << std::hex << nRead<<" result=0x"<<result << endl;
#endif
        nRead += result;
        pos += result;
    }
    if( responsedata->m_id != responseid )
    {
        cerr << "SendCommand 1: Response ID " << (UINT32)responsedata->m_id << " does't match " << (UINT32)responseid << endl;
        return false;
    }
#ifdef TEST
	cout << "SendCommand 1: Response ID " << (UINT32)responsedata->m_id << "  match " << (UINT32)responseid << endl;
#endif
    return true;
}

// Send an extended command to a socket
// This is not part of the ftIF2013TransferAreaComHandler class to avoid
// dependence on the command and response structures

bool SendCommandEx(
    SOCKET socket, 
    const struct ftIF2013Command_BaseEx *commanddata, size_t commandsize,
    enum ftIF2013ResponseId responseid, struct ftIF2013Response_BaseEx *responsedata, size_t responsesize, UINT32 exrespmaxsize )
{
    int result;
    result = send( socket, (const char*) commanddata, commandsize + commanddata->m_extrasize, 0 );
    if( result!=commandsize + commanddata->m_extrasize)
    {
        cerr << "SendCommandEx 2: Error sending command" << endl;
        return false;
    }

  //  size_t nRead=0;
    UINT32 nRead=0;
	char *pos = (char*) responsedata;
    while( nRead < responsesize )
    {
        result = recv(socket, pos, (UINT32)responsesize-nRead, 0 );
        if( result <= 0 )
        {
            cerr << "SendCommandEx 2: Error receiving response" << endl;
            return false;
        }
        nRead += result;
        pos += result;
    }
    if( responsedata->m_id != responseid )
    {
        cerr << "SendCommandEx 2: Response ID " << (UINT32)responsedata->m_id << " does't match " << (UINT32)responseid << endl;
        return false;
    }

    nRead=0;
    pos = ((char*) responsedata) + responsesize;
    if( responsedata->m_extrasize >= exrespmaxsize )
    {
        cerr << "SendCommandEx 3: Error, extra size to large " << responsedata->m_extrasize << " > " << exrespmaxsize << endl;
        return false;
    }
    while( nRead < responsedata->m_extrasize )
    {
        result = recv(socket, pos, responsedata->m_extrasize-nRead, 0 );
        if( result <= 0 )
        {
            cerr << "SendCommandEx 3: Error, receiving extra response" << endl;
            return false;
        }
        nRead += result;
        pos += result;
    }

    return true;
}

//******************************************************************************
//*
//* Implementation of class ftIF2013TransferAreaComHandler
//*
//******************************************************************************

ftIF2013TransferAreaComHandler::ftIF2013TransferAreaComHandler( FISH_X1_TRANSFER *transferarea, int nAreas, const char *name, const char *port ) :
    m_transferarea( transferarea),
    m_nAreas( nAreas ),
    m_name( name ),
    m_port( port ),
    m_online( false ),
    m_socket( INVALID_SOCKET ),
    m_buffersize( 4096 ),
    m_exchange_cmpr_command( (ftIF2013Command_ExchangeDataCmpr*) (void*) new UINT8[ sizeof(ftIF2013Command_ExchangeDataCmpr) + m_buffersize] ),
    m_exchange_cmpr_response( (ftIF2013Response_ExchangeDataCmpr*) (void*) new UINT8[ sizeof(ftIF2013Response_ExchangeDataCmpr) + m_buffersize] ),
    m_comprbuffer( new CompressionBuffer( m_exchange_cmpr_command->m_data,(int) m_buffersize ) ),
    m_expbuffer( new ExpansionBuffer( m_exchange_cmpr_response->m_data, (int) m_buffersize ) ),
    m_camerastarted( false ),
    m_camerasocket( INVALID_SOCKET ),
    m_camerabuffersize( 0 ),
    m_camerabuffer( 0 )
{
#ifdef TEST
    cout << "ftIF2013TransferAreaComHandler start" << endl;
#endif
	// clear transfer area
    memset( m_transferarea, 0, m_nAreas*sizeof(*m_transferarea) );
    SetDefaultConfig();

    // initialize transfer area time handlers
    memset( m_timelast, 0, sizeof(m_timelast) );

    // Initialize Windows winsock
    WSADATA wsaData;
    memset( &wsaData, 0, sizeof(wsaData) );
   if( WSAStartup( MAKEWORD(2,0), &wsaData )!=0)cerr << "ftIF2013TransferAreaComHandler WSAStartup error" << endl; ;
#ifdef TEST
	cout << "ftIF2013TransferAreaComHandler end" << endl;
#endif
}

ftIF2013TransferAreaComHandler::~ftIF2013TransferAreaComHandler()
{
    cout << "ftIF2013TransferAreaComHandler: destructor " << endl;
    if (m_online) EndTransfer();
    int tt= WSACleanup();
    cout << "ftIF2013TransferAreaComHandler: destructor clean up socket2 ="<<tt << endl;
}

// Get Interface Version
UINT32 ftIF2013TransferAreaComHandler::GetVersion()
{
    // Send a query status command
    ftIF2013Command_QueryStatus command;
    memset( &command, 0, sizeof(command) );
    command.m_id = ftIF2013CommandId::ftIF2013CommandId_QueryStatus;
    ftIF2013Response_QueryStatus response;
#ifdef TEST	
	cout << "GetVersion: Before SendCommand: ftIF2013CommandId_QueryStatus m_id=0x" << std::hex << (UINT32)command.m_id << endl;
#endif
    if( !SendCommand( m_socket, &command, sizeof(command), ftIF2013ResponseId::ftIF2013ResponseId_QueryStatus, &response, sizeof(response) ) )
    {
		cerr << "GetVersion 4 After SendCommand: ftIF2013CommandId_QueryStatus is false" << endl;
		return 0;
    }
	//cout << "GetVersion:  After SendCommand: ftIF2013CommandId_QueryStatus is true" << endl;
	strncpy( m_info_devicetype, response.m_devicename, sizeof(m_info_devicetype)/sizeof(*m_info_devicetype) );
    m_info_version = response.m_version;
#ifdef TEST	
	cout << "GetVersion: m_info_devicetype =" << m_info_devicetype << endl;
	cout << "          : m_info_version    =" << m_info_version << endl;
#endif
    return m_info_version;
}
/// <summary>
    /// Set the transfer mode <br/>
    /// Default is Compressed.
    /// </summary>
    /// <param name="Compressed"> Simple mode =false: no compression only for master<br/> 
    /// Compression mode = true: works for both only master and master+ slave.</param>
    /// <remarks> since 2020-06-18 </remarks>
    /// <returns></returns>
void ftIF2013TransferAreaComHandler::SetTransferMode(bool Compressed) {
#ifdef TEST	
    cout << "SetTransferMode: Compressed ="<< Compressed << endl;
#endif
    this->IsCompressedMode = Compressed;
};

bool ftIF2013TransferAreaComHandler::BeginTransfer()
{
#ifdef TEST	
    cout << "Methode BeginTransfer" << endl;
#endif
    if( m_online )
    {
        cerr << "BeginTransfer: transfer alread started" << endl;
        return false;
    }
#ifdef TEST	
	cout << "BeginTransfer: transfer not started" << endl;
#endif
    m_socket = OpenSocket( m_port );
    if( m_socket == INVALID_SOCKET )
    {
        cerr << "BeginTransfer: Could not open sockt" << endl;
        return false;
    }
#ifdef TEST	
    cout << "BeginTransfer: Has  open sockt" << endl;
#endif
    // Check TXT Version
	UINT32 version = GetVersion();
    if( GetVersion()<0x04060600 )
    {
        cerr << "BeginTransfer: TXT Version is lower then 4.6.6:  0x" << std::hex << (int) version <<endl;
        return false;
    }
#ifdef TEST	
    cout << "BeginTransfer: TXT Version is >= 4.6.6   found=0x" << std::hex << version << endl;
#endif
    // Send a start transfer command (e.g. ignore key presses on display)
    {
        ftIF2013Command_StartOnline command;
        memset( &command, 0, sizeof(command) );
        command.m_id = ftIF2013CommandId::ftIF2013CommandId_StartOnline;
        ftIF2013Response_StartOnline response;
        //   strncpy( command.m_name, "Online", 6 );
		strncpy(command.m_name, "Online", sizeof(command.m_name) / sizeof(*command.m_name));
        // strncpy_s( command.m_name, sizeof(command.m_name), "Online", sizeof(command.m_name)/sizeof(*command.m_name) );
#ifdef TEST	
        cout << "BeginTransfer Before SendCommand: ftIF2013CommandId_StartOnline command name="  << command.m_name <<endl;
        cout << "BeginTransfer Before SendCommand: ftIF2013CommandId_StartOnline command id=0x" << hex <<(UINT32) command.m_id << endl;
        cout << "BeginTransfer Before SendCommand: ftIF2013CommandId_StartOnline response size id=0x" << hex << sizeof(response) << endl;
#endif
        if( !SendCommand( m_socket, &command, sizeof(command), ftIF2013ResponseId::ftIF2013ResponseId_StartOnline, &response, sizeof(response) ) )
        {
			cerr << "BeginTransfer 5 SendCommand: ftIF2013CommandId_StartOnline is false" <<  endl;
			return false;
        }
#ifdef TEST	
        cout << "BeginTransfer SendCommand: ftIF2013CommandId_StartOnline is true" <<  endl;
#endif
	}

   // cout << "BeginTransfer UpdateConfig  start" << endl;
    // Update I/O Configuration
    if (!this->UpdateConfig()) {
        cerr << "BeginTransfer UpdateConfig error" << endl;
        return false;
    };
	//cout << "BeginTransfer UpdateConfig  end" << endl;	

    // initialize transfer area time handlers
    long now = clock();
    m_timelast[0] = now;
    m_timelast[1] = now;
    m_timelast[2] = now;
    m_timelast[3] = now;
    m_timelast[4] = now;
    m_timelast[5] = now;
    m_online = true;
	return true;
}

// Update the I/O (e.g. universal input) configuration
bool ftIF2013TransferAreaComHandler::UpdateConfig()
{
#ifdef TEST	   
	cout << "UpdateConfig: start" << endl;
#endif  
    for( int iExt=0; iExt<IF_TXT_MAX; iExt++ )
    {
        ftIF2013Command_UpdateConfig command;
        memset( &command, 0, sizeof(command) );
        command.m_id = ftIF2013CommandId::ftIF2013CommandId_UpdateConfig;
        command.m_config = m_transferarea[iExt].ftX1config;
        command.m_config_id = m_transferarea[iExt].ftX1state.config_id;
        command.m_extension_id = iExt;

//		cout << "UpdateConfig Before SendCommand: ftIF2013CommandId_UpdateConfig" << endl;

        ftIF2013Response_UpdateConfig response;
        if( !SendCommand( m_socket, &command, sizeof(command), ftIF2013ResponseId::ftIF2013ResponseId_UpdateConfig, &response, sizeof(response) ) )
        {
            cerr << "UpdateConfig: Error sending ftIF2013ResponseId_UpdateConfig command" << endl;
            return false;
        }
    }
#ifdef TEST	
	cout << "UpdateConfig: end" << endl;
#endif  
    return true;
}

bool ftIF2013TransferAreaComHandler::DoTransferSimple()
{
	//cout << "Methode DoTransferSimple" << endl;

    if( m_socket == INVALID_SOCKET )
    {
        cerr << "DoTransferSimple: Socket not open" << endl;
        return false;
    }
#ifdef TEST	
    cout << "DoTransferSimple: Socket is open" << endl;
#endif
    // Uncompressed transfer mode
    ftIF2013Command_ExchangeData command;
    memset( &command, 0, sizeof(command) );
    command.m_id = ftIF2013CommandId::ftIF2013CommandId_ExchangeData;

    // Transfer data from transfer struct to communication struct
    for( int i=0; i<ftIF2013_nPwmOutputs; i++ )
    {
        command.m_pwmOutputValues[i] = m_transferarea[0].ftX1out.duty[i];
    }
    for( int i=0; i<ftIF2013_nMotorOutputs && i<IZ_MOTOR; i++ )
    {
        command.m_motor_master[i] = m_transferarea[0].ftX1out.master[i];
        command.m_motor_distance[i] = m_transferarea[0].ftX1out.distance[i];
        command.m_motor_command_id[i] = m_transferarea[0].ftX1out.motor_ex_cmd_id[i];
    }
    for( int i=0; i<ftIF2013_nCounters && i<IZ_COUNTER; i++ )
    {
        command.m_counter_reset_command_id[i] = m_transferarea[0].ftX1out.cnt_reset_cmd_id[i];
    }
    command.m_sound_index = m_transferarea[0].sTxtOutputs.u16SoundIndex;
    command.m_sound_repeat = m_transferarea[0].sTxtOutputs.u16SoundRepeat;
    command.m_sound_command_id = m_transferarea[0].sTxtOutputs.u16SoundCmdId;
#ifdef TEST	
    cout << "DoTransferSimple: Before SendCommand: ftIF2013CommandId_ExchangeData command" << endl;
#endif
    ftIF2013Response_ExchangeData response;
    {
        if( !SendCommand( m_socket, &command, sizeof(command), ftIF2013ResponseId::ftIF2013ResponseId_ExchangeData, &response, sizeof(response) ) )
        {
            cerr << "DoTransferSimple: Error sending ftIF2013CommandId_ExchangeData command" << endl;
            return false;
        }
    }
#ifdef TEST	
	cout << "DoTransferSimple: After SendCommand: ftIF2013CommandId_ExchangeData command" << endl;
#endif
    // Transfer data from communication struct to transfer struct
    // Universal Inputs
    for( int i=0; i<ftIF2013_nUniversalInputs && i<IZ_UNI_INPUT; i++ )
    {
        m_transferarea[0].ftX1in.uni[i] = response.m_universalInputs[i];
    }

    // Counters
    for( int i=0; i<ftIF2013_nCounters && i<IZ_COUNTER; i++ )
    {
        m_transferarea[0].ftX1in.cnt_in[i] = response.m_counter_input[i];
        m_transferarea[0].ftX1in.counter[i] = response.m_counter_value[i];

        // Check if finished reset counter command ID changed
        if( response.m_counter_command_id[i] != m_transferarea[0].ftX1in.cnt_reset_cmd_id[i] )
        {
            // remember current finished reset counter command ID
            m_transferarea[0].ftX1in.cnt_reset_cmd_id[i] = response.m_counter_command_id[i];
            // If currently requested command is finished, set the cnt_resetted signal
            if( response.m_counter_command_id[i] == m_transferarea[0].ftX1out.cnt_reset_cmd_id[i] )
            {
                m_transferarea[0].ftX1in.cnt_resetted[i] = 1;
            }
        }
    }

    // Motors
    for( int i=0; i<ftIF2013_nMotorOutputs && i<IZ_COUNTER; i++ )
    {
        // Check if finished motor command ID changed
        if( response.m_motor_command_id[i] != m_transferarea[0].ftX1in.motor_ex_cmd_id[i] )
        {
            // remember current finished motor command ID
            m_transferarea[0].ftX1in.motor_ex_cmd_id[i] = response.m_motor_command_id[i];
            // If currently requested command is finshed, set the motor_ex_reached signal
            if( response.m_motor_command_id[i] == m_transferarea[0].ftX1out.motor_ex_cmd_id[i] )
            {
                m_transferarea[0].ftX1in.motor_ex_reached[i] = 1;
            }
        }
    }

    // Sound
    m_transferarea[0].sTxtInputs.u16SoundCmdId = response.m_sound_command_id;

    // IR
    for( int i=0; i<ftIF2013_nIRChannels+1 && i<NUM_OF_IR_RECEIVER+1; i++ )
    {
        KE_IR_INPUT_V01 *channel = &m_transferarea[0].sTxtInputs.sIrInput[i]; 

        channel->i16JoyLeftX             = response.m_ir[i].m_ir_leftX;
        channel->u16JoyLeftXtoLeft       = response.m_ir[i].m_ir_leftX < 0 ? -response.m_ir[i].m_ir_leftX : 0;
        channel->u16JoyLeftXtoRight      = response.m_ir[i].m_ir_leftX > 0 ?  response.m_ir[i].m_ir_leftX : 0;

        channel->i16JoyLeftY             = response.m_ir[i].m_ir_leftY;     
        channel->u16JoyLeftYtoBackwards  = response.m_ir[i].m_ir_leftY < 0 ? -response.m_ir[i].m_ir_leftY : 0;     
        channel->u16JoyLeftYtoForward    = response.m_ir[i].m_ir_leftY > 0 ?  response.m_ir[i].m_ir_leftY : 0;     

        channel->i16JoyRightX            = response.m_ir[i].m_ir_rightX;   
        channel->u16JoyRightXtoLeft      = response.m_ir[i].m_ir_rightX < 0 ? -response.m_ir[i].m_ir_rightX : 0;   
        channel->u16JoyRightXtoRight     = response.m_ir[i].m_ir_rightX > 0 ?  response.m_ir[i].m_ir_rightX : 0;   

        channel->i16JoyRightY            = response.m_ir[i].m_ir_rightY;        
        channel->u16JoyRightYtoBackwards = response.m_ir[i].m_ir_rightY < 0 ? -response.m_ir[i].m_ir_rightY : 0;   
        channel->u16JoyRightYtoForward   = response.m_ir[i].m_ir_rightY > 0 ?  response.m_ir[i].m_ir_rightY : 0;   

        channel->u16ButtonOn             = ( response.m_ir[i].m_ir_bits & 1 ) ? 1 : 0;
        channel->u16ButtonOff            = ( response.m_ir[i].m_ir_bits & 2 ) ? 1 : 0;

        channel->u16DipSwitch1           = ( response.m_ir[i].m_ir_bits & 4 ) ? 1 : 0;
        channel->u16DipSwitch2           = ( response.m_ir[i].m_ir_bits & 8 ) ? 1 : 0;
    }

    // Timers
	//cout << "Methode Update Timers" << endl;
	UpdateTimers();
#ifdef TEST	
	cout << "Methode End TransferSimple" << endl;
#endif  
    return true;
}

bool ftIF2013TransferAreaComHandler::DoTransfer()
{
    bool res= (IsCompressedMode) ? DoTransferCompressed() : DoTransferSimple();
#ifdef TEST	
    cout << "DoTransfer end="<<res << endl;
#endif 
    return res;
}


bool ftIF2013TransferAreaComHandler::DoTransferCompressed()
{
    if( m_socket == INVALID_SOCKET )
    {
        cerr << "DoTransferCompressed: socket not open" << endl;
        return false;
    }

#ifdef TEST	
    cout << "DoTransferCompressed: socket open" << endl;
#endif   
    memset( m_exchange_cmpr_command, 0, sizeof(*m_exchange_cmpr_command) );
    m_exchange_cmpr_command->m_id = ftIF2013CommandId::ftIF2013CommandId_ExchangeDataCmpr;

    // Transfer order
    // INT16 m_pwmOutputValues[ftIF2013_nPwmOutputs];
    // INT16 m_motor_master[ftIF2013_nMotorOutputs];
    // INT16 m_motor_distance[ftIF2013_nMotorOutputs];
    // INT16 m_motor_command_id[ftIF2013_nMotorOutputs];
    // INT16 m_counter_reset_command_id[ftIF2013_nCounters];
    // UINT16  m_sound_command_id;
    // UINT16  m_sound_index;
    // UINT16  m_sound_repeat;

    // Transfer data from transfer struct to communication struct
    m_comprbuffer->Rewind();

    // Note: we transfer only extensions reported as existing by the interface
    // This means outputs will be one cycle late, but this is less of an issue than inputs.
    m_exchange_cmpr_command->m_active_extensions = 0;
    for( int i=0; i<m_nAreas-1 && i<SLAVE_CNT_MAX; i++ )
    {
        if( m_transferarea[0].ftX1state.io_slave_alive[i] )
        {
            m_exchange_cmpr_command->m_active_extensions |= (1<<i);
        }
    }

    for( int iIf=0; iIf<IF_TXT_MAX; iIf++ )
    {
        if( iIf>=1 && !(m_exchange_cmpr_command->m_active_extensions & (1<<(iIf-1))) )
        {
            continue;
        }

        FISH_X1_TRANSFER *area = &m_transferarea[iIf];
        for( int i=0; i<ftIF2013_nPwmOutputs; i++ )
        {
            m_comprbuffer->AddWord( area->ftX1out.duty[i] );//1
        }
        for( int i=0; i<ftIF2013_nMotorOutputs && i<IZ_MOTOR; i++ )
        {
            m_comprbuffer->AddWord( area->ftX1out.master[i] );//2
        }
        for( int i=0; i<ftIF2013_nMotorOutputs && i<IZ_MOTOR; i++ )
        {
            m_comprbuffer->AddWord( area->ftX1out.distance[i] );//3
        }
        for( int i=0; i<ftIF2013_nMotorOutputs && i<IZ_MOTOR; i++ )
        {
            m_comprbuffer->AddWord( area->ftX1out.motor_ex_cmd_id[i] ); //4
        }
        for( int i=0; i<ftIF2013_nCounters && i<IZ_COUNTER; i++ )
        {
            m_comprbuffer->AddWord( area->ftX1out.cnt_reset_cmd_id[i] ); //5
        }
        m_comprbuffer->AddWord( area->sTxtOutputs.u16SoundCmdId ); //6
        m_comprbuffer->AddWord( area->sTxtOutputs.u16SoundIndex ); //7
        m_comprbuffer->AddWord( area->sTxtOutputs.u16SoundRepeat );//8
    }
    m_comprbuffer->Finish();
    m_exchange_cmpr_command->m_crc = m_comprbuffer->GetCrc();
    m_exchange_cmpr_command->m_extrasize = m_comprbuffer->GetCompressedSize();
	// output info??
    m_expbuffer->Rewind();
    {
#ifdef TEST	
		cerr << "DoTransferCompressed SendCommand: before" << endl;
#endif
		if( !SendCommandEx( m_socket,
            m_exchange_cmpr_command, sizeof(*m_exchange_cmpr_command),
            ftIF2013ResponseId::ftIF2013ResponseId_ExchangeDataCmpr, m_exchange_cmpr_response, sizeof(*m_exchange_cmpr_response), m_expbuffer->GetMaxBufferSize() ) )
        {
            cerr << "DoTransferCompressed SendCommand: Error sending ftIF2013ResponseId_ExchangeDataCmpr command" << endl;
            return false;
        }
//		cout << "DoTransferCompressed SendCommand: Sending Ready ftIF2013ResponseId_ExchangeDataCmpr command, CRC="
//			<< m_comprbuffer->GetCrc()<< " compressed size send=" << m_comprbuffer->GetCompressedSize() << endl;
		m_expbuffer->SetBufferSize( m_exchange_cmpr_response->m_extrasize );
    }

    for( int i=0; i<SLAVE_CNT_MAX; i++ )
    {
        m_transferarea[0].ftX1state.io_slave_alive[i] = ( m_exchange_cmpr_response->m_active_extensions & (1<<i) ) != 0;
    }

    // Transfer data from communication struct to transfer struct
    // INT16 m_universalInputs[ftIF2013_nUniversalInputs];
    // INT16 m_counter_input[ftIF2013_nCounters];
    // INT16 m_counter_value[ftIF2013_nCounters];
    // INT16 m_counter_command_id[ftIF2013_nCounters];
    // INT16 m_motor_command_id[ftIF2013_nMotorOutputs];
    // UINT16  m_sound_command_id;
    // struct IR
    // {
    //     INT16  m_ir_leftX;  // left  handle, horizontal, -15..15
    //     INT16  m_ir_leftY;  // left  handle, vertical,   -15..15
    //     INT16  m_ir_rightX; // right handle, horizontal, -15..15
    //     INT16  m_ir_rightY; // right handle, vertical,   -15..15
    //     UINT16 m_ir_bits;   // 2^0=on, 2^1=off, 2^2=switch1, 2^3=switch2
    // } m_ir[5];

    for( int iIf=0; iIf<IF_TXT_MAX; iIf++ )
    {
        if( iIf>=1 && !(m_exchange_cmpr_response->m_active_extensions & (1<<(iIf-1))) )
        {
            continue;
        }

        FISH_X1_TRANSFER *area = &m_transferarea[iIf];
        // Universal Inputs
        for( int i=0; i<ftIF2013_nUniversalInputs && i<IZ_UNI_INPUT; i++ )
        {
            area->ftX1in.uni[i] = m_expbuffer->GetUINT16();  //1
        }

        // Counters
        for( int i=0; i<ftIF2013_nCounters && i<IZ_COUNTER; i++ )
        {
            area->ftX1in.cnt_in[i] = m_expbuffer->GetUINT16(); //2
        }
        for( int i=0; i<ftIF2013_nCounters && i<IZ_COUNTER; i++ )
        {
            area->ftX1in.counter[i] = m_expbuffer->GetUINT16();  //3
        }
        for( int i=0; i<ftIF2013_nCounters && i<IZ_COUNTER; i++ )
        {
            UINT16 cmdid = m_expbuffer->GetUINT16();

            // Check if finished reset counter command ID changed
            if( cmdid != area->ftX1in.cnt_reset_cmd_id[i] )
            {
                // remember current finished reset counter command ID
                area->ftX1in.cnt_reset_cmd_id[i] = cmdid;           //4
                // If currently requested command is finished, set the cnt_resetted signal
                if( cmdid == area->ftX1out.cnt_reset_cmd_id[i] )
                {
                    area->ftX1in.cnt_resetted[i] = 1;              //4 afgeleide
                }
            }
        }

        // Motors
        for( int i=0; i<ftIF2013_nMotorOutputs && i<IZ_COUNTER; i++ )
        {
            UINT16 cmdid = m_expbuffer->GetUINT16();

            // Check if finished motor command ID changed
            if( cmdid != area->ftX1in.motor_ex_cmd_id[i] )
            {
                // remember current finished motor command ID
                area->ftX1in.motor_ex_cmd_id[i] = cmdid;       //5
                // If currently requested command is finshed, set the motor_ex_reached signal
                if( cmdid == area->ftX1out.motor_ex_cmd_id[i] )
                {
                    area->ftX1in.motor_ex_reached[i] = 1;
                }
            }
        }

        // Sound
        area->sTxtInputs.u16SoundCmdId = m_expbuffer->GetUINT16();     //6

        // IR
        for( int i=0; i<ftIF2013_nIRChannels+1 && i<NUM_OF_IR_RECEIVER+1; i++ )
        {
            KE_IR_INPUT_V01 *channel = &m_transferarea[0].sTxtInputs.sIrInput[i];
            INT16 leftX = m_expbuffer->GetINT16();                                //7
            INT16 leftY = m_expbuffer->GetINT16();
            INT16 rightX = m_expbuffer->GetINT16();
            INT16 rightY = m_expbuffer->GetINT16();
            UINT16 bits = m_expbuffer->GetUINT16();

            channel->i16JoyLeftX             = leftX;
            channel->u16JoyLeftXtoLeft       = leftX < 0 ? -leftX : 0;
            channel->u16JoyLeftXtoRight      = leftX > 0 ?  leftX : 0;

            channel->i16JoyLeftY             = leftY;     
            channel->u16JoyLeftYtoBackwards  = leftY < 0 ? -leftY : 0;     
            channel->u16JoyLeftYtoForward    = leftY > 0 ?  leftY : 0;     

            channel->i16JoyRightX            = rightX;   
            channel->u16JoyRightXtoLeft      = rightX < 0 ? -rightX : 0;   
            channel->u16JoyRightXtoRight     = rightX > 0 ?  rightX : 0;   

            channel->i16JoyRightY            = rightY;        
            channel->u16JoyRightYtoBackwards = rightY < 0 ? -rightY : 0;   
            channel->u16JoyRightYtoForward   = rightY > 0 ?  rightY : 0;   

            channel->u16ButtonOn             = ( bits & 1 ) ? 1 : 0;
            channel->u16ButtonOff            = ( bits & 2 ) ? 1 : 0;

            channel->u16DipSwitch1           = ( bits & 4 ) ? 1 : 0;
            channel->u16DipSwitch2           = ( bits & 8 ) ? 1 : 0;
        }
		area->sTxtInputs2.u16MicLin = m_expbuffer->GetUINT16();     //6
		area->sTxtInputs2.u16MicLog = m_expbuffer->GetUINT16();     //6

    }

    if( m_exchange_cmpr_response->m_crc != m_expbuffer->GetCrc() )
    {
        cerr << "DoTransferCompressed: CRC Error,  CRC=" << m_exchange_cmpr_response->m_crc 
			    << " CRC from buffer"<< m_expbuffer->GetCrc() <<endl;
        return false;
    }
#ifdef TEST	
	cout << "Did TransferCompressed: no CRC Error" << endl;
#endif

    // Timers
    UpdateTimers();

    return true;
}

// Print the most important inputs and outputs to the console
void ftIF2013TransferAreaComHandler::PrintIO( int master_ext )
{
#if 1
    /* The STL Version is too slow */
    cout << "Transfer ID=" << master_ext << " I=";
    for( int i=0; i<IZ_UNI_INPUT; i++ )
    {
        cout << m_transferarea[master_ext].ftX1in.uni[i] << " ";
    }

	cout << "#####C=";
    for( int i=0; i<IZ_COUNTER; i++ )
    {
        cout << m_transferarea[master_ext].ftX1in.counter[i] << " ";
    }

	cout << "#####O=";
    for( int i=0; i<IZ_PWM_CHAN; i++ )
    {
        cout << m_transferarea[master_ext].ftX1out.duty[i] << " ";
    }

    cout << endl;
#else
    /* The traditional printf is 10x faster */
    printf( "Transfer ID=%d I=%d %d %d %d %d %d %d %d C=%d %d %d %d O=%d-%d %d-%d %d-%d %d-%d\n",
        master_ext,
        m_transferarea[master_ext].ftX1in.uni[0], m_transferarea[master_ext].ftX1in.uni[1], 
        m_transferarea[master_ext].ftX1in.uni[2], m_transferarea[master_ext].ftX1in.uni[3],
        m_transferarea[master_ext].ftX1in.uni[4], m_transferarea[master_ext].ftX1in.uni[5], 
        m_transferarea[master_ext].ftX1in.uni[6], m_transferarea[master_ext].ftX1in.uni[7],
        m_transferarea[master_ext].ftX1in.counter[0], m_transferarea[master_ext].ftX1in.counter[1], 
        m_transferarea[master_ext].ftX1in.counter[2], m_transferarea[master_ext].ftX1in.counter[3], 
        m_transferarea[master_ext].ftX1out.duty[0], m_transferarea[master_ext].ftX1out.duty[1], 
        m_transferarea[master_ext].ftX1out.duty[2], m_transferarea[master_ext].ftX1out.duty[3],
        m_transferarea[master_ext].ftX1out.duty[4], m_transferarea[master_ext].ftX1out.duty[5], 
        m_transferarea[master_ext].ftX1out.duty[6], m_transferarea[master_ext].ftX1out.duty[7]
    );
#endif
}

void ftIF2013TransferAreaComHandler::EndTransfer()
{
    if( !m_online )
    {
		cout << "EndTransfer: not online" << endl;
		return;
    }

    // Stop all motors
	cout << "EndTransfer: stop motors" << endl;
	//StopMotorsSimple();
    StopMotors();

    // Stop online mode on interface (e.g. ignore key presses on display)
    if( m_socket != INVALID_SOCKET )
    {
        ftIF2013Command_StopOnline command;
        ftIF2013Response_StopOnline response;
		cout << "EndTransfer: Before SendCommand: ftIF2013ResponseId_StopOnline" << endl;

        command.m_id = ftIF2013CommandId::ftIF2013CommandId_StopOnline;
        if (!SendCommand(m_socket, &command, sizeof(command), ftIF2013ResponseId::ftIF2013ResponseId_StopOnline, &response, sizeof(response))) {
            cerr << "EndTransfer: SendCommand: error" << endl;
        };

        int tt=closesocket( m_socket );
        cout << "EndTransfer: m_socket is closed ="<<tt << endl;
    } else cout << "EndTransfer: m_socket is INVALID_SOCKE" << endl;

    m_online = false;
    cout << "EndTransfer: end" << endl;
}

bool ftIF2013TransferAreaComHandler::StartCamera( int width, int height, int framerate, int powerlinefreq, const char *port )
{
    if( m_camerastarted )
    {
        cerr << "StartCamera: Camera already started" << endl;
        return false;
    }

    m_camerabuffersize = width*height+1024;
    m_camerabuffer = new unsigned char[m_camerabuffersize];
    if( !m_camerabuffer )
    {
        m_camerabuffersize = 0;
        cerr << "StartCamera: Cannot allocate camera buffer" << endl;
        return false;
    }

    ftIF2013Command_StartCameraOnline command;
    memset( &command, 0, sizeof(command) );
    command.m_id = ftIF2013CommandId::ftIF2013CommandId_StartCameraOnline;
    command.m_width = width;
    command.m_height = height;
    command.m_framerate = framerate;
    command.m_powerlinefreq = powerlinefreq;
	cerr << "Before SendCommand: ftIF2013CommandId_StartCameraOnline" << endl;

    ftIF2013Response_StartCameraOnline response;

    if( !SendCommand( m_socket, &command, sizeof(command), ftIF2013ResponseId::ftIF2013ResponseId_StartCameraOnline, &response, sizeof(response) ) )
    {
        cerr << "StartCamera: Error sending ftIF2013ResponseId_StartCameraOnline command" << endl;
        return false;
    }

    // Open camera socket
    m_camerasocket = OpenSocket( port );
    if( m_camerasocket == INVALID_SOCKET )
    {
        cerr << "StartCamera: Cannot open camera socket" << endl;
        return false;
    }
    
    m_camerastarted = true;

    return true;
}

void ftIF2013TransferAreaComHandler::StopCamera()
{
    if( !m_camerastarted )
    {
        return;
    }

    m_camerastarted = false;

    closesocket(m_camerasocket);
    m_camerasocket = INVALID_SOCKET;

    ftIF2013Command_StopCameraOnline command;
    memset( &command, 0, sizeof(command) );
    command.m_id = ftIF2013CommandId::ftIF2013CommandId_StopCameraOnline;
	cerr << "Before SendCommand: ftIF2013CommandId_StopCameraOnline" << endl;

    ftIF2013Response_StopCameraOnline response;
    if( !SendCommand( m_socket, &command, sizeof(command), ftIF2013ResponseId::ftIF2013ResponseId_StopCameraOnline, &response, sizeof(response) ) )
    {
        return;
    }

    delete [] m_camerabuffer;
    m_camerabuffersize = 0;
}

bool ftIF2013TransferAreaComHandler::GetCameraFrameJpeg( unsigned char **buffer, size_t *buffersize )
{
    int result;

    *buffer = 0;
    *buffersize = 0;

    // Read frame header
    ftIF2013Response_CameraOnlineFrame response;
    result = recv( m_camerasocket, (char*)&response, sizeof(response), 0);
    // result 0 means socket closed, <0 means error, otherwise #bytes received
    if( result != sizeof(response) )
    {
        cerr << "GetCameraFrameJpeg: Error receiving camera frame" << endl;
        return false;
    }
    if( response.m_id != ftIF2013ResponseId::ftIF2013DataId_CameraOnlineFrame )
    {
        cerr << "GetCameraFrameJpeg: Camera response id doesn't match" << endl;
        return false;
    }

    // Read frame body
    {
        // Check compressed buffers size and reallocate if needed
        if( m_camerabuffersize<(size_t)response.m_framesizecompressed )
        {
            delete [] m_camerabuffer;
            m_camerabuffersize = 2*response.m_framesizecompressed;
            m_camerabuffer = new unsigned char[m_camerabuffersize];
            if( !m_camerabuffer )
            {
                m_camerabuffersize = 0;
                cerr << "GetCameraFrameJpeg: Cannot enlarge camera buffer" << endl;
                return false;
            }
        }

        // Read compressed frame
        size_t nRead=0;
        size_t framesize = response.m_framesizecompressed;
        unsigned char *pos = m_camerabuffer;
        while( nRead < framesize )
        {
            result = recv( m_camerasocket, (char*)pos, framesize-nRead, 0);
            if( result <=0 )
            {
                cerr << "GetCameraFrameJpeg: Error reading frame data" << endl;
                return false;
            }
            nRead += result;
            pos += result;
        }
    }

    // Send Acknowledge
    ftIF2013Acknowledge_CameraOnlineFrame ack;
    ack.m_id = ftIF2013CommandId::ftIF2013AcknowledgeId_CameraOnlineFrame;

    result = send( m_camerasocket, (const char*)&ack, sizeof(ack), 0 );
    if (result != sizeof(ack))
    {
        cerr << "GetCameraFrameJpeg: Error sending acknowledge" << endl;
        return false;
    }

    // return results
    *buffer = m_camerabuffer;
    *buffersize = response.m_framesizecompressed;
    return true;
}

// Open a socket
SOCKET ftIF2013TransferAreaComHandler::OpenSocket( const char *port )
{
    int result;
    SOCKET resultsocket = INVALID_SOCKET;

    // Resolve the server address and port
    struct addrinfo *adrlist = NULL;
    struct addrinfo hints;

    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    result = getaddrinfo( m_name, port, &hints, &adrlist);
    if( result != 0 )
    {
        cerr << "OpenSocket: Error getting address info " << result << endl;
        return false;
    }

    // Go through the list of possible detailed addresses matching the spec
    for(struct addrinfo *adr=adrlist; adr; adr=adr->ai_next )
    {
        // Create a socket for connecting to the interface
        resultsocket = socket(adr->ai_family, adr->ai_socktype, adr->ai_protocol);
        if (resultsocket == INVALID_SOCKET)
        {
            cerr << "OpenSocket: Error opening socket" << endl;
            return false;//SOCKET_ERROR?
        }
#ifdef TEST	
		cout << "OpenSocket: opening socket" << endl;
#endif
        // Connect to the interface.
        if( connect( resultsocket, adr->ai_addr, (int)adr->ai_addrlen) == SOCKET_ERROR )
        {
            closesocket( resultsocket );
            resultsocket = INVALID_SOCKET;
            cerr << "OpenSocket: Error connecting socket" << endl;
            continue;//next step in the for-statement
			//only in this case the for goes on with the iteration
        }
#ifdef TEST	
		cout << "OpenSocket: connected to socket" << endl;
#endif
        // Set timeout. Close connection if the interface doesn't send or receive a command for this long
        {
 			unsigned long iTimeout_recv =0;//DWORD
			result = setsockopt(resultsocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iTimeout_recv, sizeof(iTimeout_recv));
			if( result != 0 )
            { 
			//	Timeout value is a DWORD in milliseconds, address passed to setsockopt() is const char *
				//http://stackoverflow.com/questions/1824465/set-timeout-for-winsock-recvfrom
                closesocket( resultsocket );
                resultsocket = INVALID_SOCKET;
				cerr << "OpenSocket:  Close connection receiver error: SOL_SOCKET, SO_RCVTIMEO" << endl;
				break;//ends the for
            }

			unsigned long iTimeout_snd = 5;//DWORD
			result = setsockopt(resultsocket, SOL_SOCKET, SO_SNDTIMEO, (const char *) &iTimeout_snd, sizeof(iTimeout_snd));
            if( result != 0 )
            {
                closesocket( resultsocket );
                resultsocket = INVALID_SOCKET;
				cerr << "OpenSocket: Close connection sender error:   SOL_SOCKET, SO_SNDTIMEO" << endl;
				break;
            }
        }
#ifdef TEST	
		cout << "OpenSocket: Connection successfull" << endl;
#endif
        // Connection successfull, so end the loop
        break;
    }
    freeaddrinfo(adrlist);

    return resultsocket;
}

void ftIF2013TransferAreaComHandler::SetDefaultConfig()
{
    for( int i=0; i<m_nAreas; i++ )
    {
        for( int j=0; j<IZ_UNI_INPUT; j++ )
        {
            m_transferarea[i].ftX1config.uni[j].mode = MODE_R;
            m_transferarea[i].ftX1config.uni[j].digital = 1;
        }
        for( int j=0; j<IZ_COUNTER; j++ )
        {
            // 1=normal, 0=inverted
            m_transferarea[i].ftX1config.cnt[j].mode = 1;
        }
        for( int j=0; j<IZ_MOTOR; j++ )
        {
            // 0=single output O1/O2, 1=motor output M1
            m_transferarea[i].ftX1config.motor[j] = 1;
        }
        m_transferarea[i].ftX1state.config_id ++;
    }
}

void ftIF2013TransferAreaComHandler::UpdateTimers()
{
    // Update timers
    long now = clock();
    m_transferarea[0].IFTimer.Timer1ms=(UINT16)(now-m_timelast[0]);
    while(now-(m_timelast[1]+10) >= 0) {
        m_timelast[1]+=10;
        m_transferarea[0].IFTimer.Timer10ms++;
        /* !!!!IF2008: Change fields missing !!! */
    }
    while(now-(m_timelast[2]+100) >= 0) {
        m_timelast[2]+=100;
        m_transferarea[0].IFTimer.Timer100ms++;
    }
    while(now-(m_timelast[3]+1000) >= 0) {
        m_timelast[3]+=1000;
        m_transferarea[0].IFTimer.Timer1s++;
    }
    while(now-(m_timelast[4]+10000) >= 0) {
        m_timelast[4]+=10000;
        m_transferarea[0].IFTimer.Timer10s++;
    }
    while(now-(m_timelast[5]+60000) >= 0) {
        m_timelast[5]+=60000;
        m_transferarea[0].IFTimer.Timer1min++;
    }
}




void ftIF2013TransferAreaComHandler::StopMotors()
{
	cout << "StopMotors: stop motors" << endl;

    for( int i=0; i<m_nAreas; i++ )
    {
        for( int j=0; j<IZ_PWM_CHAN; j++ )
        {
            m_transferarea[i].ftX1out.duty[j] = 0;
        }
    }
    this->DoTransfer();
}
/****************************************************************************/
/*   ftIF2013TransferAreaComHandlerEx::   Communication thread                    */
/****************************************************************************/
void ftIF2013TransferAreaComHandlerEx::thread_TAcommunication(std::future<void> futureObj) {
#ifdef TEST	
    std::cout << "Thread Start" << std::endl;
#endif	
    bool stop = false;
    if (!this->BeginTransfer())
    {
        cerr << "thread_TAcommunication: Error: BeginTransfer" << endl;
        stop = true;
    }
#ifdef TEST	
    else		cout << "thread_TAcommunication: BeginTransfer done" << endl;
#endif	
    while (!stop && (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout))
    {
        if (!this->DoTransfer())
        {
            cerr << "thread_TAcommunication: Error DoTransfer break" << endl;	stop = true;
        }
#ifdef TEST
        else cout << "thread_TAcommunication:Transfer " << stop << endl;
#endif	
        //DoTransfer will wait for 10 msec between two transfers.
        std::this_thread::sleep_for(std::chrono::milliseconds(7)); // Sleep for a little to prevent blocking
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 
    this->EndTransfer();
#ifdef TEST	
    std::cout << "thread_TAcommunication: Thread End" << std::endl;
#endif	        
};

int ftIF2013TransferAreaComHandlerEx::TaComThreadStart() {
    if (thread1.joinable()) {
        std::cout << "TaComThreadStart: TA communication thread is already running." << std::endl;
        return 1; };
    //Fetch std::future object associated with promise
    futureObj = exitSignal.get_future();
    // Starting Thread & move the future object in lambda function by reference
    //https://stackoverflow.com/questions/10673585/start-thread-with-member-function
    thread1 = std::thread([=] {thread_TAcommunication(std::move(futureObj)); });
    return 0;
};

int ftIF2013TransferAreaComHandlerEx::TaComThreadStop() {
    if (!thread1.joinable()) {
        std::cout << "TaComThreadStop: TA communication thread is already not running." << std::endl;
        return 1;
    };
    std::cout << "Asking Thread to Stop" << std::endl;
    //Set the value in promise
    exitSignal.set_value();

    thread1.join();
    std::cout << "Thread join" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    return 0;
};
bool ftIF2013TransferAreaComHandlerEx::TaComThreadIsRunning() {
    return thread1.joinable();
};
/****************************************************************************/
/*   ftIF2013TransferAreaComHandlerEx::   I2C section                       */
/****************************************************************************/



