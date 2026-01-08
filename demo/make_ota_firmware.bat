@echo off
set CURDIR=%~dp0

if exist %CURDIR%\ql-application\threadx\build\app.bin (
	set app_path=%CURDIR%\ql-application\threadx\build
)else (
	echo Please compile before creating OTA firmware
    goto make_ota_end
)


if exist %CURDIR%\ql-cross-tool\FBFMake_CF (
	set ota_tool_path=%CURDIR%\ql-cross-tool\FBFMake_CF
)else (
	echo Please capy FBFMake_CF tool to %CURDIR%\ql-cross-tool 
    goto make_ota_end
)

copy /Y %app_path%\customer_app.bin %ota_tool_path%\full\customer_app.bin

pushd ql-cross-tool\FBFMake_CF\
call fbfmake_full_app.bat
popd

if exist %CURDIR%\ql-cross-tool\FBFMake_CF\fbf.bin (
	set ota_fw_path=%CURDIR%\ql-cross-tool\FBFMake_CF
)else (
	echo OTA Firmware create fail
    goto make_ota_end
)

if exist %CURDIR%\project_name.conf (
	set /p build_version=<%CURDIR%\project_name.conf
)else (
	set build_version=FW_4GW
)
set buildver_name=%build_version%_OTA.bin


move /y %ota_fw_path%\fbf.bin %buildver_name%
echo OTA Firmware create success

:make_ota_end
echo finish