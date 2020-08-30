# Set the TXT functional configuration
This needs to be done before starting the  TA communication with ```ftxStartTransferArea()```.

> For working with the callbacks events[ see 8](.\events.md).

## 3.10 SetFtUniConfig
```C 
FtErrors SetFtUniConfig(
ShmIfId_TXT shmId, 
Input idxIO, 
InputMode mode,
 bool digital);
```
Function configures a universal input (combination input) to measure analog and digital
voltage and resistance values and for analog distance measuring.
Call:<br/> 
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>

> ```Input idxIO``` - index of the universal input (0 to 7)<br/>

> ```InputMode mode``` - measurement mode<br/>
	  0= voltage (mV)<br/>
	  1= resistance (5 kΩ) (default)<br/>
	  3= ultrasonic distance sensor (distance measurement)
	 
> ```bool digital``` - identifier for whether the value is returned <br/>
	 - FALSE= analog<br/> 
	 - TRUE= digital (default)<br/>
  If it is a distance measurement, then only analogue.<br/>
		
> Return:<br/> 
```FtErrors errCode``` - FTLIB_ERR_SUCCESS (no error) or error code<br/>

## 3.11 SetFtCntConfig
```C 
FtErrors SetFtCntConfig(
ShmIfId_TXT shmId,
 Counter iCnt,
  int mode);
```
Function configures a counter input (counter); how the status of the counter is to be
interpreted.

Call: 
```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
```Counter iCnt``` - counter index (0 to 3)<br/>
```int mode``` <br/>- 0= NORMAL mode,<br/>- 1= INVERTED mode (falling edge)<br/>


Return:<br/> 
```FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error) or error code<br/>

## 3.12 SetFtMotorConfig
```C 
FtErrors SetFtMotorConfig(
ShmIfId_TXT shmId, 
Motor idxMotor, 
bool status);
```
Function activates or deactivates motor outputs. Analogous to that, the PMW outputs are
deactivated or activated accordingly.<br/>

Call: <br/>
>```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
> ```Motor idxMotor``` - motor index (0 to 3)<br/>
> ```bool status```<br/> - TRUE= motor on (motor outputs activated)(default)<br/>
              - FALSE= motor off (PMW output activated)<br/>
              
Return:<br/> 
> ```FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error) or error code<br/>


# Document history
Version 1.1.2.1
(c) 2020 TesCaWeb ing. C. van Leeuwen Btw.