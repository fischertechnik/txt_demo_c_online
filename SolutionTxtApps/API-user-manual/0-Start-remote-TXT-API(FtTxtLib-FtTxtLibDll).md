

# FtTxtLib remote API user manual

## Contact
> If you have any questions, problems or suggestions, please contact us: fischertechnik-technik@fischer.de

> This is an experimental version, your comment and experiences can help to improve this product.

## Introduction

(This documentation is under development)<br/>
> The remote API has been build on top of the "online TXT" solution.<br/>
 
> The main set of API's is coming from the TX-Controler and Robo-Interface API-sets.<br/>
 
> The communication between the remote TXT and the MS-Windows 10 system is hidden in a separate thread now.<br/> 
 
> The socket communication is protected with a ``` mutex```. <br/>
 
> Also the image processing is put in a separate thread, see the renewed Camera example.<br/>   


## Code examples
See also the example projects in  ```SolutionTxtApps```
```C
#include "FtTxtLib.h"
using namespace fischertechnik::txt::ids;
using  namespace fischertechnik::txt::remote::api;

/****************************************************************************/
/// <summary>
/// Adjust for your own situation
/// </summary>
const std::string MyIP = "192.168.10.171";
const std::string logMap = "H:\\workspaceVS\\txt_demo_c_online.git\\SolutionTxtApps\\";//map for the internal log file//The internal log file: ftlib.log
/****************************************************************************/
const std::string TaPort = "65000";
//examples
const  Motor IdMotorA = Motor::M1, IdMotorB = Motor::M2, IdMotorC = Motor::M3;
const  Counter IdCntA = Counter::C1, IdCntB = Counter::C2;
const  Input IdSensorUltra = Input::I6, IdSensorSwitch = Input::I1;

ftIF2013TransferAreaComHandlerEx2* ComHandler;

int main()
{
/****************************************************************************/
//Step 1: create the object: ComHandler 
/****************************************************************************/
ComHandler = new ftIF2013TransferAreaComHandlerEx2(IF_TXT_MAX, MyIP.c_str(), TaPort.c_str(), logMap.c_str(), LogState::LOGFREE);
//optional, in case you like to use the TA directly	
FISH_X1_TRANSFER* TransArea = ComHandler->GetTransferAreasArrayAddr();
/****************************************************************************/
/* Step 2: Setup the configuration first*/
/****************************************************************************/
//Examples:
ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, IdSensorSwitch, InputMode::MODE_R, true);
ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, IdSensorUltra, MODE_ULTRASONIC, false);
ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorA, true);//As full bridge (M functionality) (default)
ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorB, true); //As full bridge (M functionality)(default)
ComHandler->SetFtMotorConfig(ShmIfId_TXT::LOCAL_IO, IdMotorC, false);//As two half  bridges (O functionality)
ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntA, false);
ComHandler->SetFtCntConfig(ShmIfId_TXT::LOCAL_IO, IdCntB, true);
/****************************************************************************/
//Step 3: start the communication thread, includes update configuration.
/****************************************************************************/
ComHandler->SetTransferMode(true);
ComHandler->ftxStartTransferArea() ;
/****************************************************************************/
//step 4
/****************************************************************************/
//Todo: your program code  

/****************************************************************************/
//step 5://Program end  
/****************************************************************************/

ComHandler->TaComThreadStop();
std::this_thread::sleep_for(std::chrono::milliseconds(10));

if (ComHandler != nullptr) delete ComHandler;	// Delete transfer area and communication area

```


## Namespace
The Api's are in the namespace:
```C
using namespace fischertechnik::txt::remote::api
```

## Overview of the remote API set in ```class ftIF2013TransferAreaComHandlerEx2```
This API is by analogy with :
- the ```Programming the ROBO TX Controller Part 2: Windows Library "ftMscLib"
MSC Vertriebs GmbH```. The function numeration comes partial from that document. 
- The ```HANDLE fthdl``` has been removed because the class instance is this HANDLE.<br/>
- Not all API functions return errors.<br/> 
  However there can be error reports in the internal log file.<br/> 

### 3.0A Constructor
```C
ftIF2013TransferAreaComHandlerEx2(
int nAreas = 1, //master=1, master+slave=2
const char* name = "192.168.7.2", 
const char* port = "65000",//fixed 
const char* logMapName = ".\\", 
LogState logLevel = LogState::INFO);

```
- Creates the connection.<br/> 
- Reset the Transfer Area to the default values.<br/>
  Counters:  mode=1, rising edge of the pulse.<br/>
	Inputs: mode = MODE_R, digital = 1 (true)<br/>
	Motors:  mode =1 => as output M1
- Opens the internal log file system.<br/>
```C
enum LogState {
	LOG = 0,
	INFO,
	LOGERR,
	WARNING,
	LOGPRINT,
	LOGFREE
};
```
Code snip of how this is done in the library:
```C
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
``` 

### 3.0B Destructor
```~ftIF2013TransferAreaComHandlerEx2()```
- Destroys the connection.<br/> 
- Closes the internal log file system.<br/>


### Functions about the TA remote communication
#### 3.1 ftxStartTransferArea
> Since: 2020-07-17

- Set the TXT configuration of the Inputs, Motors/Outputs and Counters.

Is needed for the camera functionality.
```C
FtErrors ftxStartTransferArea ()
```

Function activates the communication of the transfer area in the library for online mode: 
- The configuration of the Motors/Outputs, Counters and Universal Inputs are set.
- The communication thread is started.
- It carries out the IO commands in "online mode" every +/- 10 msec: 
 - It reads the current values from the output structure of the transfer area (configuration and output values) and sends these to the TXT Controller.
 - As a response to an IO request, the TXT controller sends
the current values back and the communication thread updates these values in the input
structure of the transfer area.


Return: 

> ```FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error, thread is activated) or error code

#### 3.2 ftxStopTransferArea
> Since: 2020-07-17

```C
 FtErrors ftxStopTransferArea ()
```

The communication thread is stopped. 

Return:
> ``` FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error, thread stopped) or
error code

#### 3.3 ftxIsTransferActiv
> Since: 2020-07-17

```C
bool ftxIsTransferActiv ()

```
Functions checks if the transfer area is active, i.e. if there is cyclical communication with the
TXT Controller.<br/>

Return:
> ``` bool``` Is running <br/>

####  3.4 GetTransferAreasArrayAddr 
> Since: 2020-07-17

```C  
volatile FISH_X1_TRANSFER* GetTransferAreasArrayAddr()
```
The function returns a pointer to the memory area of all transfer areas (transfer areas are
arranged as an array of structures).<br/>

Return:
> ```volatile FISH_X1_TRANSFER*``` - starting address of the memory area of all transfer
areas<br/>

Remark: <br/>
All transfer areas (master + up to 1 slave) are stored in this memory area.  For the layout of the structures within the transfer area, refer to the header file  ```FtShmemTxt.h```.


### For the other API's see:

[1 configuration related API's](./1-Configuration.md)

[2 Motors/Output related API's](./2-Actuators.md)

[3 Universal Input related API's ](./3-Inputs(Uni).md)

[4 Counters related API's ](./4-Inputs(Cnt).md)

[5 IR controller input related API's ](./5-Inputs(IR).md)

[6 Microphone and sound related API's ](./6-new(mic-sound).md)

[7 Definition of the constants ](./7-Const.md)


### Left over API functions:
#### GetVersion

```UINT32 ftIF2013TransferAreaComHandler::GetVersion()```


# Annex


# Document history
Version 1.1.2.1
(c) 2020 TesCaWeb ing. C. van Leeuwen Btw.