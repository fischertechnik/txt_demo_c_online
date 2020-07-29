#  Motor/Output related functions

## 3.9 SetOutPwmValues
```C 
FtErrors  SetOutPwmValues (
ShmIfId_TXT shmId,
Output outIdx,
int duty)
```
Set the duty for outIdx. Needs to be configurated as single mode.
 
Call:<br/>

> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>

> ```Output outIdx``` - index of the PWM output (0 to -7) [O1..O8]<br/>

> ```int duty``` - duty value for the PMW output

Return:<br/>
> ```FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error) or error code<br/>
                 Is not in the Output mode but Motor mode

## N.2.3 GetOutPwmValues
> (since 2020-07-20)

 ```C 
 FtErrors GetOutPwmValues(ShmIfId_TXT shmId, Output outIdx, int& duty);
 ```	
 Get the duty for outIdx. Needs to be configurated as single mode.
  
 Call:<br/>
 
 > ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
 
 > ```Output outIdx``` - index of the PWM output (0 to -7) [O1..O8]<br/>
 
 > ```int& duty``` - duty value for the PMW output
 
 Return:<br/>
 > ```FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error) or error code<br/>
                  Is not in the Output mode but Motor mode



## 3.8A SetOutMotorValues
> (since 2020-07-20)

```C 
FtErrors SetOutMotorValues(ShmIfId_TXT shmId, Motor motorId, int duty, Direction direction);
```
Function sets the duty values for the two motor outputs M+ and M- for a motor in the
transfer area.<br/>
Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)

> ```Motor motorId``` - index of the motor to be controlled (0 to 3)<br/>

> ```int duty``` - duty value [0..512] for motor output  <br/>


> ```Direction direction``` - CW=1, Halt=0, CCW=-1<br/>

 
Return:<br/>
> ```FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error) or error code<br/>Is not in the Motor mode but Output mode.  See also the internal log file.
   
## 3.8B SetOutMotorValues
> (since 2020-07-20)
   
 ```C 
   FtErrors SetOutMotorValues(ShmIfId_TXT shmId, Motor motorId, int power);
   ```
Function sets the duty values for the two motor outputs M+ and M- for a motor in the
   transfer area.<br/>
   
Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)
   
> ```Motor motorId``` - index of the motor to be controlled (0 to 3)<br/>
   
> ```int power``` - duty value + direction [-512..0..512] for motor output M<br/>
   
Return:<br/>
> ```FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error) or error code<br/>
     Is not in the Motor mode but Output mode.
      See also the internal log file.
	 
## N.2.2 SetOutMotorInverse
> (since 2020-07-20)

```C 
FtErrors SetOutMotorInverse(ShmIfId_TXT shmId, Motor motorId);
```	 

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)
   
> ```Motor motorId``` - index of the motor to be controlled (0 to 3)<br/>

Invers the Motor ```motorId```

Return:<br/>
> ```FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error) or error code<br/>
     Is not in the Motor mode but Output mode.
      See also the internal log file.
	   

## N.2.1 GetOutMotorValues
> (since 2020-07-20)

```C 
FtErrors GetOutMotorValues(ShmIfId_TXT shmId, Motor motorId, int& power);
```	
Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)
   
> ```Motor motorId``` - index of the motor to be controlled (0 to 3)<br/>

> ```int& power``` - duty value + direction [-512..0..512] for motor output ```motorId```<br/>  

Return:<br/>
> ```FtErrors``` errCode - FTLIB_ERR_SUCCESS (no error) or error code<br/>
     Is not in the Motor mode but Output mode.
      See also the internal log file.
	 

## 3.13 StartMotorExCmd
There are 3 versions
1. For 1 motor, only distance.
2. For 1 master and a slave, distance and sync.
3. For 1 master and 1..3 slaves, distance and sync.
 
```C 
FtErrors  StartMotorExCmd(
 ShmIfId_TXT shmId, 
 Motor mIdx, 
 int duty, 
 Direction mDirection,
  Motor sIdx, 
  Direction sDirection, 
  int pulses);
```
Function activates the intelligent motor mode for motor synchronization. The motor moves
to the desired position using the shared counter information. The application shares the
information that the motor has reached the end position by using 3.13B IsMotorExCmdReady.

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)

> ```Motor mIdx``` - motor index (0 to 3) from master (motor)

> ```int duty``` - duty value for master/slave motor

> ```Direction mDirection``` - direction for master motor (1= CW, 0=halt, -1= CCW)

> ```Motor sIdx``` - motor index (0 to 3) from slave (motor)

> ```Direction sDirection``` - direction for slave motor (1= CW, 0=halt, -1= CCW)

> ```int pulseCnt``` - number of count pulses for moving to a position,
relative to the starting position<br/>

Return:<br/>
> ```FtErrors```  errCode - FTLIB_ERR_SUCCESS (no error) or error code

## 3.13B StartMotorExCmd
> (since 2020-07-20)

```C 
FtErrors  StartMotorExCmd(
 ShmIfId_TXT shmId, 
 Motor mIdx, 
 int duty, 
 Direction mDirection,
int pulses);
```
Function activates the intelligent motor mode . The motor moves
to the desired position using the counter information. The application shares the
information that the motor has reached the end position by using 3.13B IsMotorExCmdReady.

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)

> ```Motor mIdx``` - motor index (0 to 3) from master (motor)

> ```int duty``` - duty value for master motor

> ```Direction mDirection``` - direction for master motor (1= CW, 0=halt, -1= CCW)

> ```int pulseCnt``` - number of count pulses for moving to a position,
relative to the starting position<br/>

Return:<br/>
> ```FtErrors```  errCode - FTLIB_ERR_SUCCESS (no error) or error code



## 3.13A StartMotorExCmd
> (since 2020-07-20)

```C 
FtErrors  StartMotorExCmd(
 ShmIfId_TXT shmId, 
 Motor mIdx, 
 int duty, 
 Direction mDirection,
  Motor sIdx, 
  Direction sDirection, 
  int pulses);
```
Function activates the intelligent motor mode for motor synchronization. The motor moves
to the desired position using the shared counter information. The application shares the
information that the motor has reached the end position by using 3.13B IsMotorExCmdReady.

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)

> ```Motor mIdx``` - motor index (0 to 3) from master (motor)

> ```int duty``` - duty value for master/slave motor

> ```Direction mDirection``` - direction for master motor (1= CW, 0=halt, -1= CCW)

> ```Motor s1Idx``` - motor index (0 to 3) from slave (motor) or ```NoMotor```

> ```Direction s1Direction``` - direction for slave motor (1= CW, 0=halt, -1= CCW)

> ```Motor s2Idx``` - motor index (0 to 3) from slave (motor) or ```NoMotor```

> ```Direction s2Direction``` - direction for slave motor (1= CW, 0=halt, -1= CCW)

> ```Motor s3Idx``` - motor index (0 to 3) from slave (motor) or ```NoMotor```

> ```Direction s3Direction``` - direction for slave motor (1= CW, 0=halt, -1= CCW)

> ```int pulseCnt``` - number of count pulses for moving to a position,
relative to the starting position<br/>

Return:<br/>
> ```FtErrors```  errCode - FTLIB_ERR_SUCCESS (no error) or error code



## N.2.2 IsMotorExCmdReady
> (since 2020-07-20)

```C 
bool IsMotorExReady(ShmIfId_TXT shmId, Motor mIdx);
```
Enhanced motor mode, has the motor reach his position?
Can be used for polling.

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)
 
> ```Motor mIdx``` - motor index (0..3) of motor M1..M4
 
Return:
> ```bool``` - Is ready.

Remark:<br/>
Needs to be check for each motor (master and slave(s)) involved in the black box action.


#### 3.14 StopMotorExCmd
```C 
FtErrors StopMotorExCmd(ShmIfId_TXT shmId, Motor mIdx);
```
Function immediately stops the previously activated intelligent motor mode for motor
synchronization of the specified motor by resetting the duty values as well as the distance
value in the output structure to 0.<br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)
 
> ```Motor mIdx``` - motor index (0..3) of motor M1..M4
 

Return:<br/> 
> ```FtErrors``` errCode- FTLIB_ERR_SUCCESS (no error) or error code<br/>


#### 3.15 StopAllMotorExCmd
```C 
FtErrors StopAllMotorExCmd(ShmIfId_TXT shmId);
```
Function immediately stops the previously activated intelligent motor mode for motor
synchronization of all motors of the TXT Controller by resetting the duty values as well
as the distance values in the output structure to 0.<br/>
Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)
 
Return:<br/> 
> ```FtErrors``` errCode- FTLIB_ERR_SUCCESS (no error) or error code<br/>

#### 3.16 SetCBMotorExReached
> (Not implemented yet.)

```C 
void SetCBMotorExReached (void (__stdcall *) cbFunct (DWORD devId, DWORD mtrIdx))
```
Function installs to the library the specified callback function that reports the "Motor Reached
State" status during active motor synchronization (intelligent motor mode).<br/>
Callback function parameter:<br/>
Call:<br/> 
```DWORD devId``` - controller ID (master or extension controller)<br/>
```DWORD mtrIdx``` - motor index (0 to 3)

# Document history
Version 1.1.2.1 (c) 2020 TesCaWeb ing. C. van Leeuwen Btw.