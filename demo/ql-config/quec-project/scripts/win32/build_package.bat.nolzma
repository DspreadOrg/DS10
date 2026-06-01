set TOP_DIR=%1

set buildver=

set OPTION=%2

set CROSS_DIR=%cross_tool%

set YEAR=%date:~3,4%
set MONTH=%date:~8,2%
set DAY=%date:~11,2%

set BUILD_TIME=%YEAR%%MONTH%%DAY%

if exist %TOP_DIR%\devtype.conf (
	set /p devtype_name=<%TOP_DIR%\devtype.conf
)else (
	set devtype_name=FW
)
if exist %TOP_DIR%\project_name.conf (
	set /p build_version=<%TOP_DIR%\project_name.conf
)else (
	set build_version=%buildver%
)

set /P project_name=<%TOP_DIR%\ql-config\config\QuecCurPrj.txt
set soc_platform_path=%TOP_DIR%\ql-config\soc_platform
set /p OEM_PROJECT=<%TOP_DIR%\ql-config\config\QuecOemPrj.txt
set /P FLASH_SIZE=<%TOP_DIR%\ql-config\config\FLASH_SIZE.ini
set /P FLASHTYPE=<%TOP_DIR%\ql-config\config\QuecFLashType.ini
set /P ExtFlashType=<%TOP_DIR%\ql-config\config\EXT_FLASH.ini
set /p ASR_PLATFORM=<%TOP_DIR%\ql-config\config\CHIP_PLAT.ini
set /p DFLAG_1602=<%TOP_DIR%\ql-kernel\threadx\config\common\default_dflags_1602.mk
set /p DFLAG_1606=<%TOP_DIR%\ql-kernel\threadx\config\common\default_dflags_1606.mk
set /P BT_MODE=<%TOP_DIR%\ql-config\config\BT_MODE.ini
set buildver_name=%devtype_name%_%build_version%_%project_name%

echo  %ASR_PLATFORM%|findstr "1606" >nul
if %errorlevel% neq 1 (
	set PACKAGE_PROJECT=CRANEL
	set aboot_image=1606
)

echo  %ASR_PLATFORM%|findstr "1609" >nul
if %errorlevel% neq 1 (
	set PACKAGE_PROJECT=CRANELS
	set aboot_image=1609
)

echo  %ASR_PLATFORM%|findstr "1602" >nul
if %errorlevel% neq 1 (
	set PACKAGE_PROJECT=CRANELR
	set aboot_image=1602
)

echo  %ASR_PLATFORM%|findstr "1605" >nul
if %errorlevel% neq 1 (
	set PACKAGE_PROJECT=CRANELRH
	set aboot_image=1605
)

echo  %ASR_PLATFORM%|findstr "1607" >nul
if %errorlevel% neq 1 (
	set PACKAGE_PROJECT=CRANELG
	set aboot_image=1607
)
set QuecProductName=CRANEL_QUEC_PRODUCT
set dsp_image=%soc_platform_path%\%PACKAGE_PROJECT%\images\DSP_Reduce_SuLog_Buffer\dsp.bin
if "%aboot_image%"=="1602" (
setlocal EnableDelayedExpansion
  	echo %DFLAG_1602%|findstr "ENABLE_WIFI_SCAN" >nul
	if !errorlevel! neq 1 (
endlocal
		set dsp_image=%soc_platform_path%\%PACKAGE_PROJECT%\images\DSP_WiFiScan_Reduce_SuLog_Buffer\dsp.bin
	)	 
)
set boot2_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\boot2.bin
set preboot_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\preboot.bin
set flasher_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\flasher.bin
set rd_image=%TOP_DIR%\ql-config\config\%project_name%\%rd_image_type%\ReliableData_SingleSim.bin
set rfbin_image=%TOP_DIR%\ql-config\config\%project_name%\rf.bin
set customer_fs_image=%TOP_DIR%\ql-config\config\%project_name%\customer_fs.bin
set customer_backup_fs_image=%TOP_DIR%\ql-config\config\%project_name%\customer_backup_fs.bin
set logo_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\logo.bin

set cp_image=%TOP_DIR%\ql-config\config\%project_name%\release_kernel.bin
set cp2_image=%TOP_DIR%\ql-config\config\%project_name%\release_kernelcp2.bin
set boot_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\boot33.bin
set boot_nologo_noupdater_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\boot33_NoLogo_NoUpdater.bin
set boot_nologo_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\boot33_NoLogo.bin
set customer_app_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\app.bin
set gnss_boot_image=%TOP_DIR%\ql-config\config\%project_name%\gnss_boot.pkg
set gnss_firm_image=%TOP_DIR%\ql-config\config\%project_name%\gnss_firm.pkg
set jacana_fw_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\jacana_fw.bin
set extflash_image=%TOP_DIR%\ql-config\config\EXT_FLASH\ext_flash.bin 
set tts_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\quectel_resource_tts.bin

if "%aboot_image%"=="1609" (
	set btbin=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\btbin.bin
	set btlst=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\btlst.bin
	set updater_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\updater_fbf.bin
) else (
	set btbin_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\btbin.bin
	if "%BT_MODE%"=="built_in" (
	   set btlst_image=%TOP_DIR%\ql-config\config\%project_name%\btlst.bin
	) else (
	   set btlst_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\btlst.bin
	)
	set updater_image=%TOP_DIR%\ql-config\quec-project\aboot\images\%aboot_image%\updater.bin
)

if exist %TOP_DIR%\ql-bootloader\boot33\release (
	set boot_image=%TOP_DIR%\ql-bootloader\boot33\release\boot33.bin
	set boot_nologo_noupdater_image=%TOP_DIR%\ql-bootloader\boot33\release\boot33_NoLogo_NoUpdater.bin
	set boot_nologo_image=%TOP_DIR%\ql-bootloader\boot33\release\boot33_NoLogo.bin
)

if exist %TOP_DIR%\ql-bootloader\updater\release (
	set updater_image=%TOP_DIR%\ql-bootloader\updater\release\updater.bin
)

if exist %TOP_DIR%\ql-application\threadx\build (
	set customer_app_image=%TOP_DIR%\ql-application\threadx\build\app.bin
	rem %TOP_DIR%\ql-cross-tool\win32\host\tools\lzma.exe ds 18 e %TOP_DIR%\ql-application\threadx\build\app.bin %TOP_DIR%\ql-application\threadx\build\app_lzma.bin
	rem set customer_app_image=%TOP_DIR%\ql-application\threadx\build\app_lzma.bin
)
rem only use for ota customer_app.bin
if exist %TOP_DIR%\ql-application\threadx\build (
	copy /Y %customer_app_image% %TOP_DIR%\ql-application\threadx\build\customer_app.bin
)
if exist %TOP_DIR%\ql-kernel\threadx\build (
	set cp_image=%TOP_DIR%\ql-kernel\threadx\build\release_kernel.bin
	set cp2_image=%TOP_DIR%\ql-kernel\threadx\build\release_kernelcp2.bin
)

if "%FLASH_SIZE%"=="2M" (
	if "%ExtFlashType%"=="2M" (
		SET OPEN_PACKAGE=+02M_DSP_EXT_OPEN 
	) else if "%ExtFlashType%"=="16M" (
		SET OPEN_PACKAGE=+16M_OPEN
	) else if "%ExtFlashType%"=="4M" (
		SET OPEN_PACKAGE=+04M_DSP_EXT_OPEN
	) else if "%ExtFlashType%"=="8M" (
		SET OPEN_PACKAGE=+08M_DSP_EXT_OPEN
	) else if "%ExtFlashType%"=="NO_EXTFLASH" (
		SET OPEN_PACKAGE=_OPEN
	) else (
		echo !!! ExtFlash %ExtFlashType% is not supported !!!
		goto end
	)
) else if "%FLASH_SIZE%"=="4M" (
	if "%ExtFlashType%"=="4M" (
		SET OPEN_PACKAGE=+04M_OPEN
	) else if "%ExtFlashType%"=="8M" (
		SET OPEN_PACKAGE=+08M_OPEN
	) else if "%ExtFlashType%"=="2M" (
		SET OPEN_PACKAGE=+02M_OPEN
	) else if "%ExtFlashType%"=="1M" (
		SET OPEN_PACKAGE=+01M_OPEN
	) 	else if "%ExtFlashType%"=="16M" (
		SET OPEN_PACKAGE=+16M_OPEN
	) else if "%ExtFlashType%"=="NO_EXTFLASH" (
		SET OPEN_PACKAGE=_OPEN
	) else (
		echo !!! ExtFlash %ExtFlashType% is not supported !!!
		goto end
	)
)
else if "%FLASH_SIZE%"=="8M" (
	if "%ExtFlashType%"=="4M" (
		SET OPEN_PACKAGE=_04M_OPEN
	) else if "%ExtFlashType%"=="8M" (
		SET OPEN_PACKAGE=_08M_OPEN
	) 	else if "%ExtFlashType%"=="16M" (
		SET OPEN_PACKAGE=_16M_OPEN
	) else (
		SET OPEN_PACKAGE=_OPEN
	)
)
if "%FLASHTYPE%"=="DEFAULT_BT" (
	SET OPEN_PACKAGE=_BT%OPEN_PACKAGE%
)
set aboot_dir=%TOP_DIR%\ql-cross-tool\win32\host\tools\aboot-tools-win-x86
set ImageInfoGet=%TOP_DIR%\ql-cross-tool\win32\host\tools\ImageInfoGet
set AddCheck=%TOP_DIR%\ql-cross-tool\win32\host\tools\AddCheck
set cmd_7z=%TOP_DIR%\ql-cross-tool\win32\host\tools\7z\7z

if not exist %TOP_DIR%\target mkdir %TOP_DIR%\target
rd /q/s %TOP_DIR%\target

if "%ASR_PLATFORM%"=="1606" (
	if "%FLASH_SIZE%"=="8M" (
		set QuecProductName=QUEC_CRANEL_SS_08MB%OPEN_PACKAGE%
	) else if "%FLASH_SIZE%"=="4M" (
	  	if "%FLASHTYPE%"=="HX_GPS" (
			set QuecProductName=QUEC_CRANEL_SS_04MB_HX_GNSS%OPEN_PACKAGE%
		) else if "%FLASHTYPE%"=="ASR_GPS" (
	        set QuecProductName=CRANEL_QUEC_PRODUCT
	    ) else (
			if "%FLASHTYPE%"=="Factory_24KB" (
				set QuecProductName=QUEC_CRANEL_SS_04MB_FACT_24KB%OPEN_PACKAGE%
			) else (
				set QuecProductName=QUEC_CRANEL_SS_04MB%OPEN_PACKAGE%
			)
	    )
	) else if "%FLASH_SIZE%"=="2M" (
		set QuecProductName=QUEC_CRANEL_SS_02MB%OPEN_PACKAGE%
		setlocal EnableDelayedExpansion		
		echo %DFLAG_1606%|findstr "QUEC_NOTA" >nul
		if !errorlevel! neq 1 (
			if "!ExtFlashType!"=="NO_EXTFLASH" (
		endlocal
				set customer_fs_image=%TOP_DIR%\ql-config\config\%project_name%\customer_fs_in.bin
			)
		)
	) else (
		echo !!!err unknown flashsize !!!
	)
) else (
	if "%ASR_PLATFORM%"=="1602" (
		if "%FLASH_SIZE%"=="4M" (
			if "%FLASHTYPE%"=="ASR_GPS" (
				set QuecProductName=QUEC_CRANELR_SS_04MB_ASR_GNSS%OPEN_PACKAGE%
			) else (
				set QuecProductName=QUEC_CRANELR_SS_04MB%OPEN_PACKAGE%
			)
		) else if "%FLASH_SIZE%"=="2M" (
			set QuecProductName=QUEC_CRANELR_SS_02MB%OPEN_PACKAGE%
		) else (
			echo !!!err unknown flashsize !!!
		)	
	) else if "%ASR_PLATFORM%"=="1605" (
		if "%FLASH_SIZE%"=="4M" (
			if "%FLASHTYPE%"=="CC1177W_GPS" (
				set QuecProductName=QUEC_CRANELRH_SS_04MB_CC1177W_GNSS
			) else (
				set QuecProductName=QUEC_CRANELRH_SS_04MB%OPEN_PACKAGE%
			)
		) else if "%FLASH_SIZE%"=="2M" (
			set QuecProductName=QUEC_CRANELRH_SS_02MB%OPEN_PACKAGE%
		)
	) else if "%ASR_PLATFORM%"=="1607" (
			if "%FLASH_SIZE%"=="4M" (
			if "%FLASHTYPE%"=="ASR_GPS" (
				set QuecProductName=QUEC_CRANELG_SS_04MB_ASR_GNSS%OPEN_PACKAGE%
			) else (
				set QuecProductName=QUEC_CRANELG_SS_04MB%OPEN_PACKAGE%
			)
		)
	)else (
		set QuecProductName=QUEC_CRANELS_SS_08MB_OPEN
	)
)

	if "%OPEN_PACKAGE%"=="_OPEN" (
		if "%aboot_image%"=="1609" (
			set standard_arelease_iamges="cp=%cp_image%,cp2=%cp2_image%,dsp=%dsp_image%,boot33=%boot_nologo_image%,updater=%updater_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,ext_flash=%extflash_image%,gnss_boot=%gnss_boot_image%,gnss_firm=%gnss_firm_image%,jacana_fw=%jacana_fw_image%,bt_btbin=%btbin%,bt_btlst=%btlst%"
		) else (
			set standard_arelease_iamges="cp=%cp_image%,cp2=%cp2_image%,dsp=%dsp_image%,boot33=%boot_nologo_image%,updater=%updater_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,ext_flash=%extflash_image%,gnss_boot=%gnss_boot_image%,gnss_firm=%gnss_firm_image%,jacana_fw=%jacana_fw_image%"
		)
	) else (
		if "%aboot_image%"=="1602" (
			 if "%FLASHTYPE%"=="DEFAULT_BT" (
					set standard_arelease_iamges="cp=%cp_image%,cp2=%cp2_image%,dsp=%dsp_image%,boot33=%boot_nologo_image%,updater=%updater_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,ext_flash=%extflash_image%,gnss_boot=%gnss_boot_image%,gnss_firm=%gnss_firm_image%,jacana_fw=%jacana_fw_image%,btbin=%btbin_image%,btlst=%btlst_image%"
				) else (
					set standard_arelease_iamges="cp=%cp_image%,cp2=%cp2_image%,dsp=%dsp_image%,boot33=%boot_nologo_image%,updater=%updater_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,ext_flash=%extflash_image%,gnss_boot=%gnss_boot_image%,gnss_firm=%gnss_firm_image%,jacana_fw=%jacana_fw_image%,quectel_resource_tts=%tts_image%"
				)
		)else if "%aboot_image%"=="1605" (
			set standard_arelease_iamges="cp=%cp_image%,cp2=%cp2_image%,dsp=%dsp_image%,boot33=%boot_nologo_image%,updater=%updater_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,ext_flash=%extflash_image%,gnss_boot=%gnss_boot_image%,gnss_firm=%gnss_firm_image%,jacana_fw=%jacana_fw_image%,quectel_resource_tts=%tts_image%,btbin=%btbin_image%,btlst=%btlst_image%"
		) else if "%aboot_image%"=="1607" (
			set standard_arelease_iamges="cp=%cp_image%,cp2=%cp2_image%,dsp=%dsp_image%,boot33=%boot_nologo_image%,updater=%updater_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,ext_flash=%extflash_image%,gnss_boot=%gnss_boot_image%,gnss_firm=%gnss_firm_image%,jacana_fw=%jacana_fw_image%,quectel_resource_tts=%tts_image%"
		)else (		
			set standard_arelease_iamges="cp=%cp_image%,cp2=%cp2_image%,dsp=%dsp_image%,boot33=%boot_nologo_image%,updater=%updater_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,ext_flash=%extflash_image%,gnss_boot=%gnss_boot_image%,gnss_firm=%gnss_firm_image%,jacana_fw=%jacana_fw_image%,quectel_resource_tts=%tts_image%"
		)
	)

echo %QuecProductName% >%TOP_DIR%\ql-config\quec-project\aboot\config\product\QuecProductName.txt

set standard_package_path=%TOP_DIR%\target\%buildver_name%
set standard_package_file=%buildver_name%_%BUILD_TIME%.zip

mkdir %standard_package_path%
if "%ASR_PLATFORM%"=="1606" (
rem	%TOP_DIR%\ql-cross-tool\win32\owtoolchain\Python\python.exe app_start_addr.py -p CRANEL_QUEC_PRODUCT.json -d %TOP_DIR%\ql-config\quec-project\aboot -a %QuecProductName% -k %TOP_DIR%\ql-kernel\threadx\build\kernel.map -l %TOP_DIR%\ql-application\threadx\config\common\app_linkscript.ld
rem	call build.bat app
	%aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p CRANEL_QUEC_PRODUCT -v %QuecProductName% -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file%
) else (
	if "%ASR_PLATFORM%"=="1602" (
rem		%TOP_DIR%\ql-cross-tool\win32\owtoolchain\Python\python.exe app_start_addr.py -p CRANELR_QUEC_PRODUCT.json -d %TOP_DIR%\ql-config\quec-project\aboot -a %QuecProductName% -k %TOP_DIR%\ql-kernel\threadx\build\kernel.map -l %TOP_DIR%\ql-application\threadx\config\common\app_linkscript.ld
rem		call build.bat app
		%aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p CRANELR_QUEC_PRODUCT -v %QuecProductName% -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file%
	) else if "%ASR_PLATFORM%"=="1605" (
rem		%TOP_DIR%\ql-cross-tool\win32\owtoolchain\Python\python.exe app_start_addr.py -p CRANELRH_QUEC_PRODUCT.json -d %TOP_DIR%\ql-config\quec-project\aboot -a %QuecProductName% -k %TOP_DIR%\ql-kernel\threadx\build\kernel.map -l %TOP_DIR%\ql-application\threadx\config\common\app_linkscript.ld
rem		call build.bat app
		%aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p CRANELRH_QUEC_PRODUCT -v %QuecProductName% -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file%
	) else if "%ASR_PLATFORM%"=="1607" (
rem		%TOP_DIR%\ql-cross-tool\win32\owtoolchain\Python\python.exe app_start_addr.py -p CRANELG_QUEC_PRODUCT.json -d %TOP_DIR%\ql-config\quec-project\aboot -a %QuecProductName% -k %TOP_DIR%\ql-kernel\threadx\build\kernel.map -l %TOP_DIR%\ql-application\threadx\config\common\app_linkscript.ld
rem		call build.bat app
		%aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p CRANELG_QUEC_PRODUCT -v %QuecProductName% -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file%
	) else (
rem		%TOP_DIR%\ql-cross-tool\win32\owtoolchain\Python\python.exe app_start_addr.py -p CRANELS_QUEC_PRODUCT.json -d %TOP_DIR%\ql-config\quec-project\aboot -a %QuecProductName% -k %TOP_DIR%\ql-kernel\threadx\build\kernel.map -l %TOP_DIR%\ql-application\threadx\config\common\app_linkscript.ld
rem		call build.bat app
		%aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p CRANELS_QUEC_PRODUCT -v %QuecProductName% -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file%
	)
)

if not exist %standard_package_path%\%standard_package_file% (
	echo =====================ERROR!!!=======================
	echo   ********  *****     *****       ***     *****
	echo   ********  **  **    **  **     ** **    **  **
	echo   **        **   **   **   **   **   **   **   **
	echo   ********  **  **    **  **    **   **   **  **
	echo   ********  *****     *****     **   **   *****
	echo   **        **  **    **  **    **   **   **  **		 
	echo   ********  **   **   **   **    ** **    **   **
	echo   ********  **    **  **    **    ***     **    ** 
	echo ====================================================
	goto end
)
	
copy /y %TOP_DIR%\ql-config\quec-project\download\quec_download.json %standard_package_path%\%buildver%.json
%ImageInfoGet% 1 0 1 %cp_image% %standard_package_path%\imageinfo.bin
%ImageInfoGet% 2 0 1 %dsp_image% %standard_package_path%\imageinfo.bin
%ImageInfoGet% 3 0 1 %rfbin_image% %standard_package_path%\imageinfo.bin
%AddCheck% %standard_package_path%\imageinfo.bin


mkdir %standard_package_path%\DBG
if exist %TOP_DIR%\ql-kernel\threadx\build (
	copy /Y %TOP_DIR%\ql-kernel\threadx\build\kernel.axf %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-kernel\threadx\build\kernel.map %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-kernel\threadx\build\KERNEL_MDB.txt %standard_package_path%\DBG
) else (
	copy /Y %TOP_DIR%\ql-config\config\%project_name%\kernel.axf %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-config\config\%project_name%\kernel.map %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-config\config\%project_name%\KERNEL_MDB.txt %standard_package_path%\DBG
rem	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\kernel.axf %standard_package_path%\DBG
rem	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\kernel.map %standard_package_path%\DBG
rem	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\KERNEL_MDB.txt %standard_package_path%\DBG
)
::rename %standard_package_path%\DBG\kernel.axf CRANE_DS_XIP_DM_GENERIC.axf
::rename %standard_package_path%\DBG\kernel.map CRANE_DS_XIP_DM_GENERIC.map

if exist %TOP_DIR%\ql-application\threadx\build (
	copy /Y %TOP_DIR%\ql-application\threadx\build\app.elf %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-application\threadx\build\app.map %standard_package_path%\DBG
) else (
	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\app.elf %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\app.map %standard_package_path%\DBG
)

::记录每次使用的feedback到DBG中
if exist %TOP_DIR%\ql-kernel\threadx\config\common\feedbackLink_backup.txt (
	copy /Y %TOP_DIR%\ql-kernel\threadx\config\common\feedbackLink_backup.txt %standard_package_path%\DBG
	del %TOP_DIR%\ql-kernel\threadx\config\common\feedbackLink_backup.txt
)
::rename %standard_package_path%\DBG\app.elf CRANE_DS_XIP_DM_GENERIC_APP.axf
::rename %standard_package_path%\DBG\app.map CRANE_DS_XIP_DM_GENERIC_APP.map

if "%OPTION%" == "NODBG" (
    echo "build firmware without dbg compress"
    goto build_end
)

%cmd_7z% a -tzip  %TOP_DIR%\target\%standard_package_file% -r %standard_package_path%\*.*

:build_end
echo   ============================SUCCESS=================================
echo       **    **     **     ******  ******  ********     **       **
echo     **  **  **     **    **      **       **         **  **   **  **
echo    **       **     **   **      **        **        **       **
echo    **       **     **  **      **         **        **       **
echo     **      **     **  **      **         ********   **       **
echo       **    **     **  **      **         **           **      **
echo        **   **     **   **      **        **            **       **
echo    **  **    **   **     **      **       **        **  **   **  **
echo     ***       *****       ******  ******  ********   ***      ***
echo   ====================================================================

:end
