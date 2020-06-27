
# TXT C Remote (on-line mode) examples
## Contact 
> If you have any questions, problems or suggestions, please contact us: fischertechnik-technik@fischer.de

Replacement for the original TXT-C-Programming-Kit-4-1-6

## Overview

### Which tools I need?
This is a MS-Visual Studio project. <br/>
The `solution` and `project` are tested with
 
[MS-Visual Studio Community 2019](https://visualstudio.microsoft.com/vs/community/) 

and is free to download and use.<br/>
It is using the C14++ dialect. The .NET extension is not used, so the source files can also be used with for example Eclipse.<br/>
After downloading the solution, it is directly usable with MS Visual Studio 2019.<br/>



### Overview of the example code

#### `MotorOnOffSwitch` map
Starters project to learn how the TA (transfer area can be used)

#### `TestProj02` map
This project shows:
1. The basics for a communication remote <=> TXT thread.
2. Some examples about how to use the encoder motor in the enhanced mode.

#### `Camera` map
Is about how to get the camera images on your remote system.     

#### The `Common`  and `Jpeg` map
This map contains the general supporting files which describe the data structures (transfer area) and the Berkeley socket (WinSocket) based communication with the TXT-controller.<br/> In fact this is a library.<br/>
In theory, if a programming language supports Berkeley sockets, it would be possible to communicated with the TXT. See for example the [Python version FtRoboPy](https://github.com/ftrobopy/ftrobopy). MS Visual Studio 2019 can also be used for Python projects

The communication over IP is using 3 ports: 65000 for the TA and commands, 65001 for the camera images and 65002 for I2C. For the last one is no protocol description available yet.
1. FtShmemTxt.h<br/>
  Is about the structure of the transfer area aiming for the TXT.
1. Common<br/>
   Header only.<br/>
   alternative names for some data types.
   
2. ftProInterface2013TransferAreaCom<br/>
   header and source.<br/>
  Is about the compression of the data for the socket communication. 
1. ftProInterface2013SocketCom<br/>
    header and source.<br/>
   Is about the compression of the data for the socket communication.
2. frProInterface2013JpegDecode<br/>
    header and source.<br/>
    Is about the decoding of the raw camera data into JPEG CODEX format.
1. Jpeg<br/>
  The distribution contains the sixth public release of the Independent JPEG
  Group's free JPEG software <br/>
  [Still-image compression – JPEG-1 extensions](http://ijg.org/files/T-REC-T.871-201105-I!!PDF-E.pdf)<br/>
  [The JPEG Still Picture Compression Standard, description](http://ijg.org/files/Wallace.JPEG.pdf) <br/> 
  [JPEG 9 info](https://jpegclub.org/reference/reference-sources/)  
    
For you as end-user there is no need to fully understand the contend of these class.

####

## History
### 2020-06-27 [CvL]
This is a **beta version** for RoboPro 4.6.6/4.7.0 pre-release: 
**TXT-C-Programming-Kit-4-6-6_beta-03**

>jpeg library from version 6d (27-Mar-1998) to  [version 9d (2-Jan-2020)](http://ijg.org/)

### 2020-06-24 [CvL]
This is a **beta version** for RoboPro 4.6.6/4.7.0 pre-release:  
**TXT-C-Programming-Kit-4-6-6_beta-02**

Tested with firmware 4.6.6 and on MS-Windows 10 (64 bits)

> Development environment: MS-Visual Studio community 2019,<br/>
  C++ Dialect: C14++

> Added: TestProj02 - example about the use of the enhanced motor control (black box)<br/>
  with the communication between PC and TXT (TA) as thread.
  
> Choice between Compress mode and Simple as setting.<br/>
  
> Test console output conditional with TEST as C++ pre-processor definition. 

> Renamed FtShmem.h into FtShmemTxt.h, add some TXT related comment.<br/>
  Limited the TA size to master + 1 slave, this because the TXT can only work with 1 slave.
  
> I2C communication is not implemented yet because the protocol for send and responds the I2C communication is not available.

### 2019-10-13 [CvL]

> The problem  with the Compressed mode has been resolved.

### 2018-09-18 [CvL]

> The Windows 10 problem around the WinSocket time out settings has been resolved.  

> Note: Don't forget to fill in the right ip-address

> Some additional text output for debugging has also been added.
> 
### 2016 
Original published as `TXT-C-Programming-Kit-4.1.6`. 



