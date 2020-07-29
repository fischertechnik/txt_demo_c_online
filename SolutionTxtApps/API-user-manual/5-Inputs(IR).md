#  IR input related functions
The TXT in combination with 1..4 fischertechnik IR controller devices (double joystick)

## N.5.1
```
UINT16 GetJoystickPart(ShmIfId_TXT shmId, IrDev dev, IrAxisP axis);
```

Get partial data [0..15] from one of the 8 halve axis. The full axis have been spilt up in parts.<br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
> ```IrDev dev``` - IR device  index (0..5), depends on the setting of the two switches <br/>
    
Return:<br/> 
> INT16 - the state the halve axe [0..15]

## N.5.2 GetJoystickFull
```
INT16 GetJoystickFull(ShmIfId_TXT shmId, IrDev dev, IrAxisF axis);
```
Get full data  [-15..0..15] from one of the 4 full axis.<br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
> ```IrDev dev``` - IR device  index (0..5), depends on the setting of the two switches <br/>
    
Return:<br/> 
> INT16 - the state the full axe [-15..0..15]

## N.5.3 GetJoystickButSwi
```
bool GetJoystickButSwi(ShmIfId_TXT shmId, IrDev dev, IrButSwi button);
```
Get data from a switch or button.<br/>

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>
> ```IrDev dev``` - IR device  index (0..5), depends on the setting of the two switches <br/>
    
Return:<br/> 
> bool - the state of the switch or button


# Document history
Version 1.1.2.1
(c) 2020 TesCaWeb ing. C. van Leeuwen Btw.