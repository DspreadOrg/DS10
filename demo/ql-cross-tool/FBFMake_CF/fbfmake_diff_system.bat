
@echo off

del /s /q /f fbf_dfota.bin
del /s /q /f a\patchfolder\*
rem FBFMake_CF_V1.5.exe -o fbf_dfota.bin -f system.img -r 0x16000 -d 0x10000 -a new -b old
FBFMake_CF_V1.6-150.exe -f config_system -d 0x10000 -a new -b old -o fbf_dfotaSYS.bin -q
