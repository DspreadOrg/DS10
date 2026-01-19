cls
@ECHO OFF
CLS
set vmbit=

set CURDIR=%~dp0
if exist %CURDIR%\module.conf (
    set /p module=<%CURDIR%\module.conf
)else (
    set module=
)
if exist %CURDIR%\devtype.conf (
    set /p devtype=<%CURDIR%\devtype.conf
)else (
    set devtype=
)
if /i %PROCESSOR_IDENTIFIER:~0,3% neq x86 set vmbit=64

:menu
Title cloud code projection build
color 0a
cd /d "%~dp0"
cls
set choice=
echo.
echo                   main menu
echo         ===========================
echo.
echo         1:build clean
echo         2:build app
echo         3:packet firmware (zip file)
echo         4:packet APP OTA file(bin file,Step three must be executed first before generating OTA firmware )
rem echo         5:packet DIFF SYS OTA file(bin file)
rem echo         6:packet DIFF APP SYS OTA file(bin file)
echo         7:set device type
echo         0:quit
echo.
echo         nots: if you select item 6 and item 7, kernel will be rebuilded , dn't do it if no needs. just only build app and packet firmware in normol
echo.
echo         prj-dir:%~dp0
:cl
echo.
set /p choice=         please select item number, then click enter button:
IF NOT "%choice%"=="" SET choice=%choice:~0,1%
if /i "%choice%"=="1" goto s_build_clean
if /i "%choice%"=="2" goto s_build_app
if /i "%choice%"=="3" goto s_make_firmware
if /i "%choice%"=="4" goto s_make_ota_firmware
rem if /i "%choice%"=="5" goto s_make_diff_sys_ota_firmware
rem if /i "%choice%"=="6" goto s_make_diff_app_sys_ota_firmware
if /i "%choice%"=="7" goto s_select_module
if /i "%choice%"=="0" goto EX

echo.
echo         Invalid selection, please enter again.
echo.
goto cl
:s_select_module
set choice=
cls
echo.
echo          		module selection
echo         ===========================
echo.
echo         1:NO SCREEN			
echo         2:WITH SCREEN				
echo         0:clean device type configuration and return main menu
echo.
:check_select_module
echo.
set /p choice=         please select item number, then click enter button:
IF NOT "%choice%"=="" SET choice=%choice:~0,1%

if /i "%choice%"=="1" (
	set module=EG800AKCN_91LC
	set module_ver=4GW_V071501_100_R07A15
)
if /i "%choice%"=="2" (
	set module=EC600MEU_LA
	set module_ver=4GW_V071501_100_R07A15
)
if /i "%choice%"=="0" (
	set module=UNKNOW
	set module_ver=UNKNOW
)
if "%module%" equ "" (
echo.
echo         Invalid selection, please enter again
echo.
goto check_select_module
)

set COPYFILE1=%CURDIR%\ql-cross-tool\FBFMake_CF\config_app_system
set COPYFILE2=%CURDIR%\ql-cross-tool\FBFMake_CF\config_full_app
set COPYFILE3=%CURDIR%\ql-cross-tool\FBFMake_CF\config_app_system
set COPYFILE4=%CURDIR%\ql-application\threadx\config\common\app_linkscript.ld

copy /Y %COPYFILE1%_%module% %COPYFILE1%
copy /Y %COPYFILE2%_%module% %COPYFILE2%
copy /Y %COPYFILE3%_%module% %COPYFILE3%
copy /Y %COPYFILE4%_%module% %COPYFILE4%
set ex_fshsize=EXT_FLASH_M08
set APP_MODE=XIP
echo %module%>module.conf
echo %module_ver%>project_name.conf
echo *****************  MODULE:%module%	***************************
call build.bat project_name -p %module% -b %ex_fshsize% -x %APP_MODE% -c %cust_ver%
echo *****************  MODULE:%module%	***************************

echo.
pause >nul


:s_select_devtype
set choice=
cls
echo.
echo                device selectio
echo         ===========================
echo         1:DS10
echo.
echo.
echo         0:clean device type configuration and return main menu
echo.
:check_select
echo.
set /p choice=         please select item number, then click enter button:
set devtype=
IF NOT "%choice%"=="" SET choice=%choice:~0,1%
if /i "%choice%"=="1" (
	set devtype=DS10
)

if /i "%choice%"=="0" (
	set devtype=UNKNOW
)
if "%devtype%" equ "" (
echo.
echo         Invalid selection, please enter again
echo.
goto check_select
)



goto save_devtype

:packname	

:save_devtype



echo.
rem pause >nul

echo *****************  DEVTYPE:%devtype%	***************************

set dev_head_file=%CURDIR%ql-application\threadx\common\include\dsp\conf_devtype.h
set config_mk_file=%CURDIR%ql-application\threadx\config.mk
if not exist %dev_head_file% (
    echo %dev_head_file% is not exist,please retry
    echo.
    set CURDIR=%~dp0
    pause >nul
    goto s_select_devtype
)
echo %devtype%>devtype.conf
echo #ifndef __CONF_DEVTYPE_H__ > %dev_head_file%
echo #define __CONF_DEVTYPE_H__ >> %dev_head_file%
echo #define CONF_DEVTYPE_DEFAULT "%devtype%">>%dev_head_file%
echo #endif >> %dev_head_file%
echo CONF_DEVTYPE_NAME=%devtype%> %config_mk_file%
echo OK
echo The configuration has been saved to devtype.conf
echo %dev_head_file%
echo.
pause >nul
goto menu


pause >nul
goto menu
:s_build_clean
rem call build.bat app clean
call build.bat clean
echo.
echo         clean successfully
echo.
pause >nul
goto menu



:s_build_app
rem set module=%choice%
call build.bat app
echo.
echo         App compilation successfully.Please confirm the compilation process prompt.
echo.
pause >nul
goto menu

:s_make_firmware
call build.bat firmware
rem call build.bat firmware
echo.
echo         Firmware packet successfully.Please confirm the compilation process prompt.
echo.
pause >nul
goto menu

:s_make_ota_firmware
call make_ota_firmware.bat
echo         OTA bin packet successfully.Please confirm the compilation process prompt.
pause >nul
goto menu

:s_make_diff_sys_ota_firmware
call make_diff_sys_ota_firmware.bat
echo         OTA bin packet successfully.Please confirm the compilation process prompt.
pause >nul
goto menu

:s_make_diff_app_sys_ota_firmware
call make_diff_app_sys_ota_firmware.bat
echo         OTA bin packet successfully.Please confirm the compilation process prompt.
pause >nul
goto menu



:EX
exit