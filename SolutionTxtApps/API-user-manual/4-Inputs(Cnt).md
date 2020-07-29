
<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Counter related functions](#counter-related-functions)
	- [3.18  GetInCounterValue](#318-getincountervalue)
	- [3.18A  GetInCounterState](#318a-getincounterstate)
	- [3.18B  GetInCntCount](#318b-getincntcount)
	- [3.6 StartCounterReset](#36-startcounterreset)
	- [3.6A IsCounterResetReady](#36a-iscounterresetready)
	- [3.7 SetCBCounterResetted](#37-setcbcounterresetted)

<!-- /TOC -->

# Counter related functions API's

## 3.18  GetInCounterValue

```C 
FtErrors GetInCounterValue(ShmIfId_TXT shmId, 
Counter iCnt, 
INT16& count, 
bool& state);
```

Function reads the current value of a counter input (counter) from the transfer area and
makes it available to an application.<br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>

> ```Counter iCnt``` [out]- counter index (0..3) of counter C1..C4<br/>

> ```INT16& count``` [out] - value of the counter. Note: is not reliable when the enhance motor control is running.<br/> 

> ```bool& state``` - state of the counter input<br/>
  -  TRUE: "INVERTED" mode<br/>
  -  FALSE: "NORMAL" mode<br/>
    
Return:<br/> 
> ```FtErrors``` - FTLIB_ERR_SUCCESS (no error) or error code

#### 3.18A  GetInCounterState
> (since 2020-07-20)

```C 
bool GetInCntState(ShmIfId_TXT shmId, Counter iCnt);
```
Get the logical state <br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
> ```Counter iCnt``` - counter index (0..3) of counter C1..C4<br/>

Return:<br/> 
> ```bool``` - the state<br/>

#### 3.18B  GetInCntCount
> (since 2020-07-20)

```C 
UINT16 GetInCntCount(ShmIfId_TXT shmId, Counter iCnt);
```
Get the value of the counter. <br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
> ```Counter iCnt``` - counter index (0..3) of counter C1..C4<br/>

Return: 
> ```UINT16``` - the counter value

#### 3.6 StartCounterReset

```C 
FtErrors StartCounterReset (ShmIfId_TXT shmId, Counter iCnt)
```
Function starts the procedure for resetting the counter input on the ROBO TX Controller to 0. This is an asynchronous process and is therefore not completed directly on return of the function call. A confirmation may take place via callback, if required. To do this, a callback function must be set using SetCBCounterResetted (see below).<br/>
Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
> ```Counter iCnt``` - counter index (0..3) of counter C1..C4<br/>
> 
Return: <br/>
>  ```FtErrors errCode``` - FTLIB_ERR_SUCCESS (no error) or error code.

#### 3.6A IsCounterResetReady
> (since 2020-07-20)

```C 
bool IsCntResetReady(ShmIfId_TXT shmId, Counter iCnt)
```
Has the counter [shmId:iCnt] been resetted?
Can be used for polling.

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
 
> ```Counter iCnt``` - counter index (0..3) of counter C1..C4<br/>

Return: <br/>
>   ```bool``` - is ready) .

#### 3.7 SetCBCounterResetted
> (not implemented yet.)<br/>

```C 
void SetCBCounterResetted (void (__stdcall *) cbFunct (hmIfId_TXT shmId,
                          Counter iCnt))
```
Function installs the specified callback function in the library. The callback function reports the
status "Counter input reset ready".

Call:<br/>
Callback function parameter:<br/>
>   ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
>   ```Counter iCnt``` - counter index (0 to 3) of counter 1 to 4


# Document history
Version 1.1.2.1
(c) 2020 TesCaWeb ing. C. van Leeuwen Btw.

