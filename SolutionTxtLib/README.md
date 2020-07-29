# SolutionTxtLib
This solution generate the FtTxtLib and FtTxtLibDll Library files for the end-user.<br/>
The libraries offer a higher level API set for the fischertechnik TXT controller. Like this was also available with the TX-controller and the Robo-interface.

[See also the API user manual.](./API-user-manual/0-Start-remote-TXT-API(FtTxtLib-FtTxtLibDll).md)

## Development Environment
This solution has been made with Ms-Visual Studio 2019.

## Namespace
```fischertechnik\:\:txt\:\:remote```,
```fischertechnik\:\:txt\:\:remote\:\:api```
```fischertechnik\:\:txt\:\:ids```

## General info

> Version 4660.4 (set in project properties: Linker| General)
 


## Communication Thread
The thread which take care of the communication between the remote system and the fischertechnik TXT-controller is part of this library now.

## What it is.
The `ftIF2013TransferAreaComHandler` has been extended with 3 function concerning the communication thread
`ftIF2013TransferAreaComHandlerEx2 : public ftIF2013TransferAreaComHandler`<br/>

```C
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

# How to use?
After building both the Release and the Debug, the PowerShell script  `Copy(FtTxtLib).ps1` copies the necessary files (.lib, .dll and header files) from the `SolutionTxtLib`  to the  `SolutionTxtApps`. 
 
## During the development phase of an application.

See also the examples in `SolutionTxtApps`.
Normally the library is compiled in the configuration `Release'.
Both .lib are

The `FtTxtLibDll.dll`, `FtTxtLib.pdb`  and the `jpeg-9d.lib` needs to be copied into the same directory as were the <app>.exe is in.
The `jpeg-9d.lib` will be used by the `TxtLib.dll`.
The `FtTxtLib.pdb` is helpful when you are debugging your application.

## Distribution of an application
The `FtTxtLibDll.dll` and the `jpeg-9d.lib` needs to be part of the distribution.

# Some back ground notes: 
> [About Link Library Dependencies= On?](http://wiki.codeblocks.org/index.php?title=FAQ-Compiling_%28general%29#Q:_How_do_I_add_version_information_to_windows_executables_and_dll.27s.3F)

>[About libs](https://social.msdn.microsoft.com/Forums/en-US/5bc28da0-318c-4355-a6d8-47bc16ac389b/dll-and-libs)

> [How to: Export C++ classes from a DLL](
https://www.codeproject.com/articles/28969/howto-export-c-classes-from-a-dll)

> [When to use dynamic vs. static libraries](
https://stackoverflow.com/questions/140061/when-to-use-dynamic-vs-static-libraries)

> [Check dll version](https://stackoverflow.com/questions/940707/how-do-i-programmatically-get-the-version-of-a-dll-or-exe-file)

# Document history
- (c) 2020-07-15 TesCaWeb.nl [C van Leeuwen] 466.1.1 new