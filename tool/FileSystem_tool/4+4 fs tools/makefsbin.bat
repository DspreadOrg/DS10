::mklfs.exe -cbackup -icustomer_fs.bin -b4096 -p4096 -r4096 -s524288   ::512
::mklfs.exe -cbackup -icustomer_fs.bin -b4096 -p4096 -r4096  
::mklfs.exe -cbackup -icustomer_fs.bin -b4096 -p4096 -r4096 -s296960     ::290KB
::mklfs-v2.exe -o -c backup -i customer_fs.bin -b 4096 -p 4096 -r 4096 -s 278528  ::瀵煎嚭鏂囦欢
cls
@ECHO OFF
CLS

echo.
echo         Select the size of the file
echo         ===========================
echo.
echo         1: 2M
echo         2: 8M
echo         3: custom size
set /p fsSize=         Enter the corresponding number: 
if /i "%fsSize%"=="1" (
    set /a fsSize=2048
    goto exe
)
if /i "%fsSize%"=="2" (
    set /a fsSize=8192
    goto exe
)
goto cl


:cl
echo.
echo         otherNumber: input digit
echo         0:   exit
set /p fsSize=         Enter the file system size(kb): 
if /i "%fsSize%"=="0" goto EX

rem Determine if the input is a number
set "valid=true"
for /f "delims=0123456789" %%i in ("%fsSize%") do (
    set "valid=false"
)

if %valid%==false (
    echo Input is not a number
) else (
    goto exe
)

echo.
echo         Input error, please retype
goto cl

:exe
set /a result=fsSize * 1024
echo.
echo         1:Make FS files
echo         2:Export FS files
echo         0:exit
set /p choice=         Enter the corresponding number: 
if /i "%choice%"=="1" goto make_file
if /i "%choice%"=="2" goto export_file
if /i "%choice%"=="0" goto EX

echo.
echo         Input error, please retype
goto exe




:make_file
mklfs-v2.exe   -m -c backup -i customer_fs.bin -b 4096 -p 4096 -r 4096 -s "%result%"  ::Input error, please retype


:export_file
mklfs-v2.exe -o -c backup -i customer_fs.bin -b 4096 -p 4096 -r 4096 -s "%result%"  ::导出FS文件

:EX
pause >nul

exit