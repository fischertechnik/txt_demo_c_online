<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Callbacks (change notification) in the TXT-API](#callbacks-change-notification-in-the-txt-api)
	- [Introduction](#introduction)
		- [Background information](#background-information)
		- [Links](#links)
	- [Orientation on the C++ implementation](#orientation-on-the-c-implementation)
		- [With a C-function or static method:](#with-a-c-function-or-static-method)
		- [With a method:](#with-a-method)
	- [Overview additional API's with callbacks](#overview-additional-apis-with-callbacks)
		- [N.8.10 (3.10)  SetFtUniConfig](#n810-310-setftuniconfig)
		- [N.8.11 (3.11) SetFtCntConfig](#n811-311-setftcntconfig)
		- [N.8.12 (3.12) SetFtMotorConfig](#n812-312-setftmotorconfig)
		- [N.8.21 SetFtCbJoyStick](#n821-setftcbjoystick)
		- [N.8.20 SetFtTaCompleted](#n820-setfttacompleted)
	- [Examples](#examples)
- [Document history](#document-history)

<!-- /TOC -->
 
# Callbacks (change notification) in the TXT-API

> [What are callbacks in general](https://en.wikipedia.org/wiki/Callback_%28computer_programming%29)

> [What are callbacks in C++?](https://stackoverflow.com/questions/2298242/callback-functions-in-c)

## Introduction

Like the original Robo-interface and TX-controller API is this API set also extended with a set of callback's (other names: events, notifications). The functional groups knows optional the possibility to use callbacks instead of polling:
1. Universal Input in de digital mode 
1. the joystick X and Y axis (fischertechnik IR-input device) 
1. the enhance motor has reached the position.
1. the counter
1. the reset of a counter is ready
The first two are  the easiest to give them places in your solution.  

The use of callback in C++ is not to difficult, it ask only some additional knowledge about: "How it works, how to use and some semantical stuff.". The examples will give some practical information and the [Links](#links) section you will find some help in discovering general information one the internet. 

### Background information
Original there exist some issues about callback in case that the function was a not static method; this because the `this` or `instance name` as part of the signature. Since C11++ there has been introduce some solutions for these issues. The callback functions definition have been defined in the FtTxtLib library with the [std::function =  general-purpose polymorphic function wrapper](https://en.cppreference.com/w/cpp/utility/functional/function). The `typedef`s for the callbacks can be found in [`Callback's definitions`](https://github.com/fischertechnik/txt_demo_c_online/blob/master/SolutionTxtLib/include/ftProInterface2013TransferAreaComEx2.h).


  
### Links
A selections of information but there is a lot more to find on the internet!
- [Pointers to Member Functions](https://isocpp.org/wiki/faq/pointers-to-members#cant-cvt-fnptr-to-voidptr)
-  [C11++ info about the bind option]( https://stackoverflow.com/questions/28055080/how-to-set-a-member-function-as-callback-using-stdbind) 
- [Using class memebrs](https://stackoverflow.com/questions/14189440/c-callback-using-class-member)
- [See for example `std::bind` tutorial](http://www.enseignement.polytechnique.fr/informatique/INF478/docs/Cpp/en/cpp/utility/functional/bind.html)
-  [About `std::placeholder`](http://www.cplusplus.com/forum/beginner/194023/)

## Orientation on the C++ implementation

The callback are implemented as C11++, this means that also object methods can be used, methods with `.this` or `object name`.

### With a C-function or static method:

Library (example):
```C
/// <summary>
/// T.N.8.10 The Digital Universal Input [shmId,  id]  has changed into [state]
/// </summary>
typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object, ShmIfId_TXT shmId, Input id, bool state)> tCbUniInputDigChanged;


class ftIF2013TransferAreaComHandlerEx2{  
/// <summary>
	/// N.8.11 (3.11)  Configurate the Counter Input
	/// </summary>
	/// <param name="shmId">master or slave</param>
	/// <param name="iCnt">id counter  </param>
	/// <param name="mode"> 1= rising edge, 0 falling edge</param>
	/// <param name="callbackCount">optional callback for counter value has been changed mode</param>
	/// <param name="callbackCnt">optional callback for reset has been finished.</param>
	/// <returns>error in case of parameter errors</returns>
	FtErrors SetFtCntConfig(ShmIfId_TXT shmId, Counter iCnt,int mode,tCbCount  callbackCount = nullptr,	tCbCntResetReady  callbackCnt = nullptr);
}  
```
User program. Be aware that `_1`, `_2`, `_3`, ... are identifiers, full names: `std::placeholders::_1`, `std::placeholders::_2`:
```C
/// <summary>
/// Example callback: print a text
/// </summary>
/// <param name="o">calling object</param>
/// <param name="shmId">master or slave</param>
/// <param name="id"></param>
/// <param name="state"></param>
void MyUniInCallback(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Input id, bool state) {
	std::cout << " MyUniInCallback [" << shmId << ":" << (uint16_t)id << " ] state ="<<(state? "true":"false") << endl;
	if (state)  stopWhile = true;
}

  
// options to use C-functions
ftIF2013TransferAreaComHandlerEx2 * ComHandler= new ftIF2013TransferAreaComHandlerEx2();
ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, Input::I1, InputMode::MODE_R, &MyUniInCallback);
// or with bind and placeholder
ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, Input::I1, 	InputMode::MODE_R, 
		std::bind(&MyUniInCallback, _1, _2, _3,_4));
```
### With a method:

User program:
```C
class MyCB{
  void MyUniInCallback(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, Input id, bool state) {
   	std::cout << " MyUniInCallback [" << shmId << ":" << (uint16_t)id << " ] state ="<<(state? "true":"false") << endl;
  	if (  (id==Input::I1) && state)  stopWhile = true;
  }
}
  
ftIF2013TransferAreaComHandlerEx2 * ComHandler= new ftIF2013TransferAreaComHandlerEx2();
MyCB * met = new MyCB();

//ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, Input::I1, InputMode::MODE_R, &met->MyUniInCallback);//gives errors
// with bind and placeholder
ComHandler->SetFtUniConfig(ShmIfId_TXT::LOCAL_IO, Input::I1, 	InputMode::MODE_R, 
		std::bind(&MyCB::MyUniInCallback, met ,_1, _2, _3,_4));
```

The above will come back in detail in the examples.


## Overview additional API's with callbacks
**Be aware that these callback runs under the communication thread and are synchronous. Your callback code doesn't be time consuming or blocking. For example: don't add wait, sleep or endless loops statements in this code.**
 
### N.8.10 (3.10)  SetFtUniConfig

Additional method: to use only in combination with the `digital` mode options `InputMode::MODE_R` or `InputMode::MODE_U`.<br/>
The `typedef` or `signature` of the callback: 
 ```C
 /// <summary>
 /// T.N.8.10  Type definition for the Digital Universal Input [shmId,  id]  has changed into [state] callbacks.
 /// </summary>
 // <param name="object">pointer to the caller instance</param>
   typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object, ShmIfId_TXT shmId, Input id, bool state)> tCbUniInputDigChanged;
 ```
 The methode to register the callback.
 ```C
 /// <summary>
/// N.8.10 (3.10) Configurate the Universal Input, digital with callback
/// </summary>
/// <param name="object">pointer to the caller instance</param>
/// <param name="shmId"></param>
/// <param name="idxIO"></param>
/// <param name="mode">only for digital InputMode::MODE_R or InputMode::MODE_U</param>
/// <param name="callback">optional callback for digital input has been changed mode or nullptr</param>
/// <returns>error if not R or V, error in case of parameter errors</returns>
FtErrors SetFtUniConfig(ftIF2013TransferAreaComHandlerEx2 * object,
                        ShmIfId_TXT shmId, Input idxIO,
                        InputMode mode,
                        tCbUniInputDigChanged callback);
 ```
### N.8.11 (3.11) SetFtCntConfig
Original method has now two additional optional fields.<br/>
For both callbacks a type definition:

```C
/// <param name="object">pointer to the caller instance</param>
/// <summary>
/// T.N.8.11A Type definition for the reset of C counter  [shmId,  id] has been finshed callbacks.
/// </summary>
/// <param name="object">pointer to the caller instance</param>
/// <param name="messageId">Id of the reset ready confirmation, >1 user reset, 1 init or motor enhance start.</param>
typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object,
                           ShmIfId_TXT shmId, Counter id,
                           uint16_t messageId)> tCbCntResetReady;
/// <summary>
/// T.N.8.11B Type definition for C counter  [shmId,  id] has changed, last known [count] callback.
/// </summary>
/// <param name="object">pointer to the caller instance</param>
typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object,
                           ShmIfId_TXT shmId, Counter id,
                           uint16_t count)> tCbCount;
```

```C
/// <summary>
/// N.8.11 (3.11)  Configurate the Counter Input
/// </summary>
/// <param name="shmId">master or slave</param>
/// <param name="iCnt">id counter  </param>
/// <param name="mode"> 1= rising edge, 0 falling edge</param>
/// <param name="callbackCount">optional callback for counter value has been changed mode</param>
/// <param name="callbackCnt">optional callback for reset has been finished.</param>
/// <returns>error in case of parameter errors</returns>
FtErrors SetFtCntConfig(
                        ShmIfId_TXT shmId, Counter iCnt, 
                        int mode,
                        tCbCount  callbackCount = nullptr,
                        tCbCntResetReady  callbackCnt = nullptr);

```
### N.8.12 (3.12) SetFtMotorConfig
Original method has now one additional optional field.
```C
/// <summary>
/// T.N.8.12 Type definition  for the Motor  [shmId,  id] (in enhance mode)  has reached its [position] callback.
/// </summary>
/// <param name="object">pointer to the caller instance</param>
typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object,
                           ShmIfId_TXT shmId, Motor id, 
                           uint16_t position, uint16_t messageId)> tCbMotorReached;
```

```C                           
/// <summary>
/// N.8.11 (3.12) Configurate the Motor/ Output mode
/// </summary>
/// <param name="shmId"></param>
/// <param name="idxMotor"></param>
/// <param name="status">true means Motor (full bridge), false means Output mode (half bridge)</param>
/// <param name="callback">optional: callback for enhance mode Motor Reached or nullptr</param>
/// <returns>error in case of parameter errors</returns>
FtErrors SetFtMotorConfig(
                          ShmIfId_TXT shmId, Motor idxMotor,
                          bool status,
                          tCbMotorReached  callback=nullptr);
```
### N.8.21 SetFtCbJoyStick
New [2020-08-27].<br/>
Callbacks (events) for the IR-device, the left and right joystick.

```C
/// <summary>
/// T.N.8.21 Type definition for the fischertechnik IR-controller (Joystick) callbacks
/// </summary>
/// <param name="object">pointer to the caller instance</param>
/// <param name="shmId">Which TXT controller, master or slave</param>
/// <param name="id">which IR-device</param>
/// <param name="group">which IR-devicejoystick, Left or Right</param>
/// <param name="AxisX">X-as value of the joystick [-15..0..15]</param>
/// <param name="AxisY">Y-as value of the joystick  [-15..0..15]</param>
typedef std::function<void(ftIF2013TransferAreaComHandlerEx2* object, ShmIfId_TXT shmId,
                          IrDev id, IrDevGroup group ,
                          int16_t AxisY,int16_t AxisX)> tCbTaJoyStick;
```

```C
/// <summary>
/// N.8.21 Set Callbacks (events) for the IT-device (Joystick controller)
/// </summary>
/// <param name="shmId">master or master+slave</param>
/// <param name="id">IR-controller id [0..4]</param>
/// <param name="JsX">callback for the X axis or nullptr</param>
/// <param name="JsY">callback for the Y axis or nullptr</param> 
/// <returns>success or error</returns>
FtErrors SetFtCbJoyStick(ShmIfId_TXT shmId, IrDev id,  
                         tCbTaJoyStick callbackJsX = nullptr,
                         tCbTaJoyStick  callbackJsY = nullptr);
```

Example
```C
using namespace std;//for the bind
using namespace std::placeholders;//for the placeholders _1 .. _6

void MyJoystick(ftIF2013TransferAreaComHandlerEx2* o, ShmIfId_TXT shmId, IrDev id, IrDevGroup gr, int16_t axisX, int16_t axisY) {
		if (id == IrDev::IR_OFF_ON) {
			switch (gr) {
			case  IrDevGroup::JoystickRight:
				o->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, Motor::M1, (512 / 15) * axisX);
				break;
			case  IrDevGroup::JoystickLeft:
				o->SetOutMotorValues(ShmIfId_TXT::LOCAL_IO, Motor::M2, (512 / 15) * axisX);
				break;
			default:	break;
			}
		}
	}

void main(){
MyCallBacks* mc = new MyCallBacks();
ComHandler = new ftIF2013TransferAreaComHandlerEx2(IF_TXT_MAX, MyIP.c_str());
TransArea = ComHandler->GetTransferAreasArrayAddr();
bool notStop=true;

//The start   
  ComHandler->SetFtCbJoyStick(ShmIfId_TXT::LOCAL_IO, IrDev::IR_OFF_ON,
		bind(&MyCallBacks::MyJoystick, mc, _1, _2, _3, _4, _5, 6), bind(&MyCallBacks::MyJoystick, mc, _1, _2, _3, _4, _5, _6));

//Todo your code
while(!notStop){
     std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //Todo: change notStop
    }


//The stop   
ComHandler->SetFtCbJoyStick(ShmIfId_TXT::LOCAL_IO, IrDev::IR_OFF_ON,nullptr,nullptr); // 
//Todo your code

//end program, clean up
ComHandler->ftxStopTransferArea();//stop the TA communication thread 
if (ComHandler != nullptr) delete ComHandler;	// Delete transfer area and communication area
std::system("pause");
}
```
###  N.8.20 SetFtTaCompleted

```C
/// <summary>
///  T.N.8.20 Type definition for TA communication callback.
//   The callback will be called  direct after the TA send and received inputs
/// </summary>
/// <param name="object">pointer to the caller instance</param>
typedef std::function<void(ftIF2013TransferAreaComHandlerEx2 * object)> tCbTaCompleted;
```
```C
/// <summary>
/// N.8.20 Set Callback for Transfer Area Cycle Complete (direct after a send/received
/// </summary>
/// <returns></returns>
FtErrors SetFtTaCompleted( tCbTaCompleted  callback = nullptr);

```

## Examples
The examples (test) projects gives your an impression about how to code. It is up to you to discover what you can do with them.<br/> Note: Event driven programming is a little bit different from sequential programming, it will ask from you a different way of thinking during the problem decomposition phase. 

The next 3 example projects in [`SolutionTxtApps`](https://github.com/fischertechnik/txt_demo_c_online/tree/master/SolutionTxtApps) gives some practical information about how to deal with this issues:
- `TestProjCallback`: The use of a C-function or static class method.
- `TestProjCallback2`: The use of methods [`std::bind`](https://en.cppreference.com/w/cpp/utility/functional/bind) and [`std::placeholders`](https://en.cppreference.com/w/cpp/utility/functional/placeholders)
- `TestProjCallback3`: The use of [`Lambda expressions`](https://en.cppreference.com/w/cpp/language/lambda) 

# Document history
2020-08-27 concept version 1.1.1.2 Add IR-device
2020-08-21 concept version 1.1.1.1 new

(c) 2020  ing. C. van Leeuwen Btw. [TesCaWeb.nl]