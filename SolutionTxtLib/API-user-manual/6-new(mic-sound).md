#  Sound/Mic related functions

## N.6.1
```
FtErrors SetSound(ShmIfId_TXT shmId, UINT16 index, UINT16 repeat);
```
Activate a sound file with index and repeat it.<br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
> ```UINT16 index``` - index of the sound file on the TXT. This can be found in the public map Sound. <br/>
> ```UINT16 repeat``` - repeat factor [1..] <br/>
    
Return:<br/> 
> INT16 - the state the halve axe [0..15]
 

## N.6.2 IsSoundReady
```
bool IsSoundReady(ShmIfId_TXT shmId);
```
Check if the activated sound has been finished.<br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
    
Return:<br/> 
> bool - Sound is ready


## N.6.3 GetMicLin
```
UINT16 GetMicLin(ShmIfId_TXT shmId);
```
Get microphone data.<br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
    
Return:<br/> 
> UINT16 - the microphone level


## N.6.4 GetMicLog

```
UINT16 GetMicLog(ShmIfId_TXT shmId);
```
Get microphone data compressed(logarithmic.<br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
    
Return:<br/> 
> UINT16 - the microphone level


# Document history
Version 1.1.2.1
(c) 2020 TesCaWeb ing. C. van Leeuwen Btw.

