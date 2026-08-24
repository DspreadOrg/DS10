# DS10 Compilation Instructions

## Environment Configuration

Please visit the following address to download the compilation chain package "owtoolchain.zip" 

https://drive.google.com/file/d/1yVfltWdgUpjrMy-ondYa0T5ihrqVkj3v/view?usp=sharing

Extract the owtoolchain. zip file from the tool And put it on the C drive (compiler default path, Pay attention to the decompressed path) View the first line of build.bat in the demo.

![](./images/projectbuild1.png)

<img src="./images/projectbuild2.png" title="" alt="" width="527">

## About the project path

Please note that the project save path cannot contain spaces. Otherwise, the firmware won't be able to be downloaded to the device after successful compilation.

## Set Device Type

- Open the project directory, enter the demo directory, double-click project_build.bat

![](./images/projectbuild3.png)

- After running the script, the following window will prompt.

![](./images/projectbuild4.png)

- Set the device type and enter the number 5 in the newly popped up window

![](./images/projectbuild5.png)

- Choose different types based on your device type
  
  ![](./images/projectbuild6.png)
  
  Taking DS10-S-CN as an example, enter the number 1 and press the confirm button

- Device type setting successful
  
  ![](./images/projectbuild7.png)

- Continue pressing the enter key.After entering the menu again, it indicates that the configuration has been completed

## Start Building

- If the first compilation, you should select 1option to clean the project to avoid leaving any other compiled information.

- Enter 2 to build the app
  
  ![](./images/projectbuild8.png)
  
  ![](./images/projectbuild9.png)
  
  The above information indicates that the compilation was successful.

- Select 3 option to packet firmware.

![](./images/projectbuild10.png)

![](./images/projectbuild11.png)

The above information indicates that the firmware has been successfully packaged.
 The firmware file will be available. Generate a zip format firmware package under **demo\target\DS10_4GW_V071502_100_R07A15_EG800AKCN_91LC**

## Firmware Download

- Run tool/download_tool/QMulti_SL_V2.6/QMulti_SL_V2.6.exe.Select the generated firmware package.

![](./images/projectbuild12.png)

- When the device is turned off, insert the USB cable and press the power button. The tool will detect the device status and start firmware download.
  
  ![](./images/projectbuild13.png)

- Firmware download success

![](./images/projectbuild14.png)

- Close the download tool and unplug the USB first, then press the power button to turn on the device, and the new firmware will run (**Please note that the download tool must be closed, otherwise the device will auto enter download mode again**)

## Generate OTA firmware

![](./images/projectbuild15.png)

You need to follow steps 2, 3, and 4 in sequence.

![](./images/projectbuild16.png)

After the fourth step is successfully executed, OTA firmware will be generated in the current directory of the script. It is a bin file.

![](./images/projectbuild17.png)

OTA firmware can be updated through TMS.

## How to create ota package and Use TMS

[Click here to get more detail](./How to create ota package.md)
