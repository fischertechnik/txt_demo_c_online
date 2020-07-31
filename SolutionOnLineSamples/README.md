# TXT C Remote (on-line mode) examples
## Contact 
> If you have any questions, problems or suggestions, please contact us: fischertechnik-technik@fischer.de

Replacement for the original TXT-C-Programming-Kit-4-1-6

## Overview SolutionOnlineSamples 
This is the original TXT-C-Programming-Kit solution.



### Which tools I need?
This is a MS-Visual Studio project. <br/>
The `solution` and `project` are tested with
 
[MS-Visual Studio Community 2019](https://visualstudio.microsoft.com/vs/community/) 

and is free to download and use.<br/>
It is using the C14++ dialect. The .NET extension is not used, so the source files can also be used with for example Eclipse.<br/>
After downloading the solution, it is directly usable with MS Visual Studio 2019.<br/>

### Overview of the example code

#### `MotorOnOffSwitch` project map
Original basic starters project to learn how the TA (transfer area can be used).

#### `Camera` map
Basic example about the use of the camera socket.

#### `TestProj02` project map
Added [June 2020 CvL] to show the enhanced motor control functionality.<br/>
This project shows:
1. The basics for a communication remote <=> TXT thread.
2. Some examples about how to use the encoder motor in the enhanced mode.
   
#### The `Common`  and `Jpeg-9d` map
This map contains the general supporting files which describe the data structures (transfer area) and the Berkeley socket (WinSocket) based communication with the TXT-controller. In fact this is a library.<br/>
In theory, if a programming language supports Berkeley sockets, it would be possible to communicated with the TXT. See for example the [Python version FtRoboPy](https://github.com/ftrobopy/ftrobopy). MS Visual Studio 2019 can also be used for Python projects

The communication over IP is using 3 ports: 
- 65000 for the TA and commands, 
- 65001 for the camera images, 
- 65002 for I2C. However this one is not documented yet.
 
1. FtShmemTxt.h<br/>
  Adaptedto the TXT possibilities and limitations[June 2020 CvL].<br/> 
  Is about the structure of the transfer area aiming for the TXT.
  
1. Common<br/>
   Header only.<br/>
   alternative names for some data types.
   
2. ftProInterface2013TransferAreaCom<br/>
   header and source.<br/>
  
1. ftProInterface2013SocketCom<br/>
    header and source.<br/>
   Is about the compression of the data for the socket communication.
2. frProInterface2013JpegDecode<br/>
    header and source.<br/>
    Is about the decoding of the raw camera data into JPEG CODEX format.
1. Jpeg-9d<br/>
  Updated to a recent version of JPEG-lib [June 2020 CvL]<br/> 
  The distribution contains the ninth public release of the Independent JPEG
  Group's free JPEG software <br/>
  [Still-image compression – JPEG-1 extensions](http://ijg.org/files/T-REC-T.871-201105-I!!PDF-E.pdf)<br/>
  [The JPEG Still Picture Compression Standard, description](http://ijg.org/files/Wallace.JPEG.pdf) <br/> 
  [JPEG 9 info](https://jpegclub.org/reference/reference-sources/)
    
For you as end-user there is no need to fully understand the contend of these classes.

 

# History
- 2020-06-27 [CvL]

Bug in firmware: The MicLin and MicLog do not work in the Simple mode.

- 2020-06-28 [CvL]

The `TA communication` thread has been moved from the application to the Txt Lib.
See also example `TestProg2`.
``` C
/*!
*  @brief Start the communication thread for the TA with the TXT
* @return 0=successful, 1=thread is already running
*/
int TaComThreadStart();
/*!
* @brief Stop the communication thread for the TA with the TXT
*/
int TaComThreadStop();
/*!
 * @return The TaComThread is running.
* @return 0=successful, 1=thread is already not running
 */
bool TaComThreadIsRunning(); 
```

- 2020-06-24 [CvL]

Published  **draft version** for RoboPro 4.6.6: 
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

- 2019-10-13 [CvL]

> The problem  with the Compressed mode has been resolved.

- 2018-09-18 [CvL]

> The Windows 10 problem around the WinSocket time out settings has been resolved.  

> Note: Don't forget to fill in the right ip-address

> Some additional text output for debugging has also been added.
> 
- 2016 
Original published as `TXT-C-Programming-Kit-4.1.6`. 



