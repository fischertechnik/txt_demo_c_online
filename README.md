<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [TXT C/C++ Remote (on-line mode) examples](#txt-cc-remote-on-line-mode-examples)
	- [1 Contact](#1-contact)
	- [2 Overview](#2-overview)
		- [2.1 Which tools I need?](#21-which-tools-i-need)
		- [2.2 Overview of the available MS-Visual Studio solutions.](#22-overview-of-the-available-ms-visual-studio-solutions)
	- [3 About the PowerShell script `Copy(FtTxtLib).ps1`](#3-about-the-powershell-script-copyfttxtlibps1)
	- [4 Transfer Area (TA)](#4-transfer-area-ta)
- [History](#history)

<!-- /TOC -->


# Note about the behavior of the I2C or SLI in the online mode.

## Description:
An I2C or SLI connection will be lost in online mode if no new action is taken within 5 seconds, it does not restart. This point has already been addressed in some example programs. Because the ROBOPro elements of I2C also run via an SLI, the same workaround applies to both.

- For I2C a dummy operation that is repeated within 5 sec.
   This can be a simple read.
- For an SLI a dummy operation to a ROBOPro SLI extension function "StayAwake".
  Additional information for the SLI developpers only:
  - [See also SLI code example 1](https://github.com/fischertechnik/txt_demo_ROBOPro_SLI/tree/master/FtTxtWorkspace/TxtSharedLibraryInterface2)
  - [See also SLI code example 2](https://github.com/fischertechnik/txt_demo_ROBOPro_SLI/tree/master/FtTxtWorkspace/TxtSliTestTA2)

ROBOPro workarounds for SLI and I2C:

 - ![ROBOPro SLI](./docs/5secIssue/element(5s).png) 

 - ![ROBOPro I2C](./docs/5secIssue/element(5s_I2C).png) 


# TXT C/C++ Remote (on-line mode) examples
 
## 1 Contact 
> If you have any questions, problems or suggestions, please contact us: fischertechnik-technik@fischer.de

Replacement for the original fischertechnik TXT-C-Programming-Kit-4-1-6

## 2 Overview
This programming kit for the development of remote applications for the fischertechnik TXT controller contains a library which takes for the communication with the TXT and offers a rich set of higher level API to the end-user. But is also still possible to program directly on TA (transfer area) level.

### 2.1 Which tools I need?
The `solutions` and `projects` are developed and tested with 
[MS-Visual Studio Community 2019](https://visualstudio.microsoft.com/vs/community/) 
which is free to download and use.<br/>

The original `SolutionOnlineSamples` is using the **C14++ dialect** and can also be used with the Eclipse 2020-06 workbench.<br/> 
The new split-up version (into a solution for the library and a solution for the applications) is using the **C17++ dialect**.<br/>
After downloading a solution, it is directly usable with MS Visual Studio 2019.<br/>

### 2.2 Overview of the available MS-Visual Studio solutions.
The original `OnlineSamples` structure contains a mixer of the low level library stuff and end-user applications and has been split up into a library solution and a application solution now.
The library has been extended with similar higher level functions as there were available in the ROBO-interface and TC-controller programming kits.
The end-user needs only `SolutionTxtApps` and can add his projects to it; this solution is also to download directly as .zip file.<br/>

1.  [`SolutionTxtApps`](./SolutionTxtApps/README.md) 

  Some examples which are using the `FtTxtLib` or `FtTxtLibDll` library, including these libs, updates of the original example applications and some test examples.<br/> 
  - The library supports the communication thread and has callbacks (events) .
  - It also support a set of higher level API functions as they were also present in the TX-C and Robo-interface programming kits. Like (part of the class `ftIF2013TransferAreaComHandlerEx2`):
    - `ftxStartTransferArea`, `ftxStopTransferArea`, `ftxIsTransferActiv`, `GetTransferAreasArrayAddr`
    - `StartMotorExCmd`,  `IsMotorExReady`
    - `SetOutMotorValues`, `GetOutMotorValues`, `SetOutMotorInverse`
    - `SetOutPwmValues`, `GetOutPwmValues`
    - `SetFtUniConfig`, `SetFtMotorConfig`, `SetFtCntConfig`
    - `GetInIOValue`
    - `SetSound, IsSoundReady`
    - `GetMicLin`, `GetMicLog`
  
   [See also the API user manual](./SolutionTxtApps/API-user-manual/README.md) 
     
1. [`SolutionTxtLib`](./SolutionTxtLib/README.md) 
  
  The project to create the static library `FtTxtLib`  and a dynamic link library (DLL) `FtTxtLibDll`. <br/>

  >  The sources Will be come available later. 

1. [`SolutionOnlineSamples`](./SolutionOnlineSamples/README.md)
    
    The original `OnlineSamples`.

## 3 About the PowerShell script `Copy(FtTxtLib).ps1`
This [PowerShell](https://docs.microsoft.com/en-us/powershell/scripting/overview?view=powershell-7) script takes care for the distribution of the result files from the `SolutionTxtLib` into `SolutionTxtApps`.

##  4 Transfer Area (TA)
[An introduction about the structure of the transfer area](https://github.com/fischertechnik/txt_demo_c_download/blob/master/TransferArea.md).  

# History
- 2020-08-30 [(c) CvL, TesCaweb.nl]

 The FtTxtLib(dll) has been extended with [callbacks](./SolutionTxtApps/API-user-manual/8-events.md).<br/>
 
 > Resolve  issue ftxIsTransferActiv(): about TA start.
 
- 2020-08-07 [(c) CvL, TesCaweb.nl]

Minor changes. 
 
- 2020-07-16/30/31 [(c) CvL, TesCaweb.nl]

**TXT-C-Programming-Kit-4-6-6_beta-04**
Re-development of the solution structure.
Re-introduction of a set of higher level API functions. 
  
- 2020-06-28 [(c) CvL, TesCaweb.nl]

The `TA communication` thread has been moved from the application to the Txt Lib.
See also example `TestProg2`.
``` C
  /// <summary>
  /// 3.1 start the communication thread and configure the Motor/Outputs, Inputs and Counters.
  /// </summary>
  /// <remarks> The configurations need to be set before!  </remarks>
  /// <returns>success or error</returns>
  FtErrors ftxStartTransferArea();

  /// <summary>
  /// 3.2 stop the communication thread
  /// </summary>
  /// <returns>success or error </returns>
  FtErrors ftxStopTransferArea();

   /// <summary>
   /// 3.3 Is the communication thread still running?
   /// </summary>
   /// <returns>Is runningr</returns>
  bool   ftxIsTransferActiv();
```
- 2020-06-27 [CvL]

Published **beta version** for RoboPro 4.6.6/4.7.0 pre-release: 
**TXT-C-Programming-Kit-4-6-6_beta-03**

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

- 2021-02-13 [CvL]

> The issue with the I2C and the SLI in the online mode was already addressed but was difficult to find, A note has been added.

- 2019-10-13 [CvL]

> The problem  with the Compressed mode has been resolved.

- 2018-09-18 [CvL]

> The Windows 10 problem around the WinSocket time out settings has been resolved.  

> Note: Don't forget to fill in the right ip-address

> Some additional text output for debugging has also been added.
> 
- 2016 

Original solution published as `TXT-C-Programming-Kit-4.1.6`. 



