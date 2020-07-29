
# Universal Input related functions

## 3.17 GetInIOValue
```C 
FtErrors GetInIOValue(ShmIfId_TXT shmId, Input idx, INT16& ftValue, bool& overrun);
```
Function reads the current value of a universal input from the transfer area and makes it
available to an application. The values of the universal inputs come as a response to an IO.

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>

> ```Input idx``` - index, universal input (0 to 7)<br/>

> ```INT16 &ftValue``` - variable that receives the value<br/>

> ```Bbool& overrun``` -  variable that receives the overrun message:
   - FALSE: no overrun<br/>
   - TRUE: overrun<br/>
 
Return:<br/> 
>```FtErrors ``` - errCode FTLIB_ERR_SUCCESS (no error) or error code


## N.3.1 GetInIOValueBinary
> (since 2020-07-20)

```C 
bool GetInIOValueBinary(ShmIfId_TXT shmId, Input idx);
```
Returns the state of ```idx```. The [shmId:idx, configuration] needs to be ````digital````,

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>

> ```Input idx``` - index, universal input (0 to 7)<br/>

## N.3.2 GetInIOValueAnalogue
> (since 2020-07-20)

```C 
INT16 GetInIOValueAnalogue(ShmIfId_TXT shmId, Input idx);
```
Returns the value of ```idx```. The [shmId:idx, configuration] needs to be ````analogue````,

Call: <br/>
> ```ShmIfId_TXT shmId``` - controller ID (master or extension controller)<br/>

> ```Input idx``` - index, universal input (0 to 7)<br/>

# Document history
Version 1.1.2.1
(c) 2020 TesCaWeb ing. C. van Leeuwen Btw.