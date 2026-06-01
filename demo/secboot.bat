cls
@ECHO OFF
CLS
set vmbit=

set CURDIR=%~dp0

:menu
Title Functional Configuration
color 0a
cd /d "%~dp0"
cls
set choice=
echo.
rem echo                   Ö÷²Ëµ¥
echo         ===========================
echo.
echo         1¡¢Close SecBoot
echo         2¡¢OPEN SecBoot
echo         0¡¢quit
:cl
echo.
set /p choice=         please select item number, then click enter button:
if /i "%choice%"=="1" goto secclose
if /i "%choice%"=="2" goto secopen
if /i "%choice%"=="0" goto close

echo.
echo         Invalid selection, please enter again
echo.
goto cl

:secclose
rem echo "secclose"
		set NAME_EXT1=nosec
		set NAME_EXT2=nolzma
rem goto menu
goto EX


:secopen
rem echo "secopen"
		set NAME_EXT1=secureboot
		set NAME_EXT2=secureboot
rem goto menu
goto EX




rem pause

:EX
rem echo "EX========="
rem echo on

set COPYFILE1=%CURDIR%\ql-config\quec-project\aboot\config\partition\CRANEL_QUEC_FLASH_LAYOUT_DS_8M_8M_OPEN.json
set COPYFILE2=%CURDIR%\ql-config\quec-project\aboot\config\product\CRANELRH_QUEC_PRODUCT.json
set COPYFILE3=%CURDIR%\ql-config\quec-project\aboot\config\template\CRANEL_QUEC_TEMPLATE_DS_8M_8M_OPEN.json
set COPYFILE4=%CURDIR%\ql-config\quec-project\scripts\win32\build_package.bat
set COPYFILE5=%CURDIR%\ql-config\quec-project\aboot\config\template\CRANELRH_QUEC_TEMPLATE_SS_04M?08M_OPEN.json
set COPYFILE6=%CURDIR%\ql-config\quec-project\aboot\config\partition\CRANELRH_QUEC_FLASH_LAYOUT_SS_04M?08M_OPEN.json
set COPYFILE7=%CURDIR%\ql-config\quec-project\aboot\config\product\CRANEL_QUEC_PRODUCT.json

copy /Y %COPYFILE1%.%NAME_EXT1% %COPYFILE1%
copy /Y %COPYFILE2%.%NAME_EXT1% %COPYFILE2%
copy /Y %COPYFILE3%.%NAME_EXT1% %COPYFILE3%
copy /Y %COPYFILE4%.%NAME_EXT2% %COPYFILE4%
copy /Y %COPYFILE5%.%NAME_EXT1% %COPYFILE5%
copy /Y %COPYFILE6%.%NAME_EXT2% %COPYFILE6%
copy /Y %COPYFILE7%.%NAME_EXT1% %COPYFILE7%

echo.
if /i "%choice%"=="2" echo         -------Sign Open Success-------
if /i "%choice%"=="1" echo         -------Sign Close Success-------
echo         Press Any Key to exit
echo.

pause >nul
:close
exit