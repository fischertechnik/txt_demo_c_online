# (c) 2020 C. van Leeuwen TesCaWeb
# version 1.1.2.1 2020-07-29
# Use to copy the libraries from the SolutionTxtLib to the SolutionTxtApps
Write-Host $PSScriptRoot

$sourceRoot = "$PSScriptRoot\SolutionTxtLib"

#copy the libs and supporting files
$destinationRoot = "$PSScriptRoot\SolutionTxtApps"
Copy-Item -Path $sourceRoot"\FtLibs"  -Recurse -Destination $destinationRoot -Container
Write-Host  $sourceRoot"\FtLibs\Debug"
Write-Host  $destinationRoot"\Debug"

#copy the API user manual
Copy-Item -Path $sourceRoot"\API-user-manual"  -Recurse -Destination $destinationRoot -Container

# copy the header files
$incFiles= @("common.h",
             "framework.h" ,
             "ftlog.h", 
             "ftProInterface2013JpegDecode", 
             "ftProInterface2013SocketCom", 
             "ftProInterface2013TransferAreaCom.h", 
             "ftProInterface2013TransferAreaComEx1.h", 
             "ftProInterface2013TransferAreaComEx2.h", 
             "ftProInterface2013JpegDecode.h", 
             "FtShmemTxt.h", 
             "FtTxtLib.h" 
                )
$libDebugFiles   =@("FtTxtLibDll.dll", "FtTxtLibDll.pdb")
$libReleaseFiles =@("FtTxtLibDll.dll", "FtTxtLibDll.pdb")
#Copy-Item -Path $sourceRoot"\API-user-manual\*"      -Destination $destinationRoot"\API-user-manual\" 
Copy-Item -Path $sourceRoot"\FtLibs\Release\*"   -Include $libReleaseFiles    -Destination $destinationRoot"\Release" 
Copy-Item -Path $sourceRoot"\FtLibs\Debug\*"   -Include $libDebugFiles   -Destination $destinationRoot"\Debug" 
Copy-Item -Path $sourceRoot"\include\*"   -Include $incFiles    -Destination $destinationRoot"\Includes" 


