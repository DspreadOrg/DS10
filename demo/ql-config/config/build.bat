@echo off

set CURDIR=%~dp0
set project_name=%2


set file_ver=%CURDIR%\..\..\ql-kernel\threadx\common\include\quectel_version.h
set cp_file_prj=%CURDIR%\..\..\ql-kernel\threadx\common\include\QuecPrjName.h
set app_file_prj=%CURDIR%\..\..\ql-application\threadx\common\include\QuecPrjName.h
set file_bak=%CURDIR%\quectel_bak.h
set file_tmp=%CURDIR%\quectel_tmp.h
set mob_model_id_a="%project_name:~0,6%"
set mob_sw_rev_a="%project_name:~0,8%%project_name:~9,2%
set mob_intra_sw_rev_a=%mob_sw_rev_a%
set mob_usb_product_a=%mob_sw_rev_a:~0,7%-%mob_sw_rev_a:~7,4%"

if exist %file_tmp% ( 
	del %file_tmp%
)
if exist %file_bak% ( 
	del %file_bak%
)
if not exist %file_ver% ( 
	goto ERROR
)

if not exist %cp_file_prj% ( 
	goto ERROR
)

if "%project_name%"=="EC100YCN_AA" (
	set flag=true
) else if "%project_name%"=="EC100YCN_AB" (
	set flag=true
) else if "%project_name%"=="EC100YCN_EL" (
	set flag=true
) else if "%project_name%"=="EC100YCN_LA" (
	set flag=true
) else if "%project_name%"=="EC100NCN_AA" (
	set flag=true
) else if "%project_name%"=="EC600SCN_LA" (
	set flag=true
) else if "%project_name%"=="EC600SCN_AC" (
	set flag=true
) else if "%project_name%"=="EC600NCN_LA" (
	set flag=true
) else if "%project_name%"=="EC600NCN_LC" (
	set flag=true
) else if "%project_name%"=="" (
	set flag=true
) else (
	set flag=false
) 

if %flag%==true (
	@echo %project_name%>%CURDIR%\QuecCurPrj.txt
) else (
	goto ERROR
)


setlocal enabledelayedexpansion
for /f "delims=" %%i in (%file_ver%) do (
    set str=%%i
    echo !str!>>%file_bak%
)

for  /f  "skip=4 delims="  %%a  in  (%file_bak%)  do  (
set  txt=%%a
goto :skip_4
)
:skip_4
set mob_model_id_b=%txt:~28,8%

for  /f  "skip=5 delims="  %%a  in  (%file_bak%)  do  (
set  txt=%%a
goto :skip_5
)
:skip_5
set mob_sw_rev_b=%txt:~20,11%

for  /f  "skip=9 delims="  %%a  in  (%file_bak%)  do  (
set  txt=%%a
goto :skip_9
)
:skip_9
set mob_intra_sw_rev_b=%txt:~26,11%

for  /f  "skip=11 delims="  %%a  in  (%file_bak%)  do  (
set  txt=%%a
goto :skip_11
)

:skip_11
set mob_usb_product_b=%txt:~31,13%

for /f "delims=" %%i in (%file_ver%) do (
    set "str=%%i"
	REM echo !str!
	set "str=!str:%mob_model_id_b%=%mob_model_id_a%!"
	set "str=!str:%mob_sw_rev_b%=%mob_sw_rev_a%!"
	set "str=!str:%mob_intra_sw_rev_b%=%mob_intra_sw_rev_a%!"
	set "str=!str:%mob_usb_product_b%=%mob_usb_product_a%!"
	echo !str!>>%file_tmp%
)
move %file_tmp% %file_ver%
if exist %file_bak% ( 
	del %file_bak%
)
REM replae __QUECTEL_PROJECT_EC100YCN_AA__
for /f "delims=" %%i in (%cp_file_prj%) do (
    set str=%%i
	REM echo !str!
	echo !str!>>%file_bak%
)

for  /f  "skip=3 delims="  %%a  in  (%file_bak%)  do  (
set  txt=%%a
goto :skip_3
)
:skip_3
set project_name_b=%txt:~26,11%

for /f "delims=" %%i in (%cp_file_prj%) do (
    set "str=%%i"
	set "str=!str:%project_name_b%=%project_name%!"
	echo !str!>>%file_tmp%
)
move %file_tmp% %cp_file_prj%
copy /Y %cp_file_prj% %app_file_prj%

if exist %file_bak% ( 
	del %file_bak%
)

	
:SUCCESS
echo ****************************************************************
echo ***********************  SUCCESS  ******************************
echo ****************************************************************
goto END

:ERROR
echo ****************************************************************
echo ***********************  ERROR  ********************************
echo ****************************************************************

:END
