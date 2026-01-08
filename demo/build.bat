set cross_tool=C:\owtoolchain
@echo off
chcp 65001 >nul



set CURDIR=%~dp0
set build_target=%1
set ARMCC_PATH=
set PATH_TMP=
@for %%i in (armcc.exe) do (
	@if not "%%~$PATH:i"=="" ( 
		set PATH_TMP=%%~$PATH:i
		SETLOCAL ENABLEDELAYEDEXPANSION
		for /f "delims=" %%i in ('armcc --vsn') do (set ret=!ret!%%i;)
		(echo !ret! | findstr /C:"Software supplied by" >nul) && (echo !ret! | findstr /C:"ARM Compiler 5" >nul) && (ENDLOCAL & goto SITE1)
		ENDLOCAL
		goto SITE2
	) else (
		goto SITE2
	)
)
:SITE1
set PATH_TMP=%PATH_TMP:\armcc.exe=%
set ARMCC_PATH=%PATH_TMP%

:SITE2
set COMPILE_ENV_SET_FLAG=
set PATH=%SystemRoot%;%SystemRoot%\system32;

if "%QUECTEL_COMPILE_TOOLS_V1%"=="" (
    set QUECTEL_COMPILE_TOOLS_V1=%cross_tool%
)
if "%ARMCC_PATH%"=="" (
    set ARMCC_PATH=%QUECTEL_COMPILE_TOOLS_V1%\ARM_Compiler_5\bin
    set ARMLMD_LICENSE_FILE=%QUECTEL_COMPILE_TOOLS_V1%\license.dat
)
set COMPILE_TOOLS_PATH=%PATH%;%ARMCC_PATH%;%QUECTEL_COMPILE_TOOLS_V1%;%QUECTEL_COMPILE_TOOLS_V1%\Perl\site\bin;%QUECTEL_COMPILE_TOOLS_V1%\Perl\bin;%QUECTEL_COMPILE_TOOLS_V1%\Gnumake;
if "%COMPILE_ENV_SET_FLAG%"==""  echo --------------set compile tools path------------------- && set PATH=%COMPILE_TOOLS_PATH% && set COMPILE_ENV_SET_FLAG=done
echo PATH:
echo %PATH%
echo -------------------------------------------------------
echo=

set compile_tools_file=%CURDIR%ql-cross-tool\win32\compile_tools.zip
set compile_tools_dir=%cross_tool%
set cmd_7z=%CURDIR%ql-cross-tool\win32\host\tools\7z\7z.exe

if not exist %compile_tools_dir% (
	echo ------------uncompress compile tools-------------
	%cmd_7z% x -y %compile_tools_file% -o%compile_tools_dir% || ( echo ------------uncompress compile tools failed------------- & goto END)
	echo ------------uncompress compile tools successfully-------------
)

if "%build_target%" == "app" (
	@call :compile_app %2
)

if "%build_target%" == "bootloader" (
	
	@call :bootloader %2
)

if "%build_target%" == "kernel" (
	@call :compile_kernel %2
)

if "%build_target%" == "firmware" (
	@call :create_firmware %2
)

if "%build_target%" == "project_name" (
	setlocal EnableDelayedExpansion
	set option=
	for %%a in (%*) do (
			if !count! gtr 0 (
	    	set option=!option! %%a
	    )
	    set /a "count+=1"
	)
	echo !option! ppp
	@call :project_name !option!
	ENDLOCAL
)

if "%build_target%" == "clean" (
	@call :compile_app clean
	@call :create_firmware clean
)

exit /b 0

:compile_app 
	pushd ql-application\threadx\
	@call build.bat %1
	popd
goto:eof

:bootloader 
	pushd ql-bootloader\boot33\
	@call build_bootloader_QUECTEL_OCPU_1605.bat %1
	popd
goto:eof

:create_firmware
	if "%1" == "clean" (
		if exist target (
			echo clean firmware 
			rd /s/q target
		)
	) else (
		@call ql-config\quec-project\scripts\win32\build_package.bat %CURDIR% NODBG
	)
goto:eof

:compile_kernel 

	
	pushd ql-kernel\threadx\
	@call build.bat %1
	
	popd
goto:eof

:project_name 
	setlocal EnableDelayedExpansion

	set parm_num=0
	for %%a in (%*) do set /a parm_num+=1
	if %parm_num% EQU 0 (
	goto paramSetting
	)

	set "option="
	for %%a in (%*) do (
		if not defined option (
			set arg=%%a
			if "!arg:~0,1!" equ "-" set "option=!arg!"
		) else (
			set "option!option!=%%a"
			set "option="
		)
	)	

	:paramSetting
	if defined option-p (
	echo Option -p given: "%option-p%"
	set "project_name=!option-p!"
	) else (
	echo Option -p not given
	set "project_name=EC600MCN_LC"
	)
	echo project_name:!project_name!

	if defined option-b (
	echo Option -b given: "%option-b%"
	set "ext_flash=!option-b!"
	) else (
	echo Option -b not given
	set "ext_flash=NO_EXTFLASH"
	)
	echo ext_flash:!ext_flash!

	if defined option-x (
	echo Option -x given: "%option-x%"
	set "APP_MODE=!option-x!"
	) else (
	echo Option -x not given
	set "APP_MODE=XIP"
	)
	echo APP_MODE:!APP_MODE!

	if defined option-c (
	echo Option -c given: "%option-c%"
	set "cust_ver=!option-c!"
	) else (
	echo Option -c not given
	set cust_ver=
	)
	echo cust_ver:!cust_ver!

	if defined option-o (
	echo Option -o given: "%option-o%"
	set "oem_def=!option-o!"
	) else (
	echo Option -o not given
	set oem_def=
	)
	echo oem_def:!oem_def!

	pushd ql-config\config\
	perl project_name.pl %project_name% %ext_flash% %APP_MODE% %cust_ver% %oem_def% 
	popd
	ENDLOCAL
goto:eof

:END
	
