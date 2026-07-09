# How to connect to you server

If you want to connect to your server with the default firmware, you only need to complete the following steps.

1. The device can be connected to the network via wireless or [WIFI](https://github.com/DspreadOrg/DS10/blob/main/doc/How%20to%20use%20wifi.md).

2. Update device parameter via OTA

## About Parameter

- **Param File Name**: posparam.ini

- **Param File Format**: json
  
  ![](images/otapackage4.png)

- **Param List**:The following is a list of parameters that support remote updates
  
  ![](images/otapackage5.png)

**Note**:If set CertState with 0,
posparam.in file is needed to create CertParam_V1.0.6.zip

![](images/otapackage7.png)

**Please note that the zip file only contains the posparam.ini file.**

If set CertSate with 1 posparam.iniand cert files are needed to create CertParam_V1.0.6.zip

![](images/otapackage6.png)

## Create Ota Package

1. Open powershell and run ./CertParam/package_resource.ps1.The script requires three parameters to be entered.
   
   (1).Original files that need to be packaged
   
   (2).Device Mode
   
   (3).Customer Name
   
   ![](images/otapackage8.png)

2. Create Cert and Param OTA package.

<img title="" src="images/otapackage9.png" alt="" width="613" data-align="center">

## Update Parameter Via  OTA

[Click here to learn how to update ](https://github.com/DspreadOrg/DS10/blob/main/doc/How%20to%20use%20tms.md)
