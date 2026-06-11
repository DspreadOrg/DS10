# DS10 Project Documentation

## Overview

The DS10 is an embedded IoT project based on Quectel wireless communication modules, primarily used for intelligent voice interaction, network connectivity, and data transmission. The project is built on the ThreadX real-time operating system and integrates dual-mode communication capabilities for 4G LTE and WiFi, featuring rich audio processing functions.

## Features

- **OTA**:Support OTA function, firmware or resources can be updated through TMS.

- **Voice Broadcast**:Support subscribing to MQTT server's broadcast messages. Supports playing audio files in MP3 and WAV formats

- **Network Management**:The device supports connecting to MQTT servers through wireless networks or WIFI.

- **LCD Display**:If the device supports an LCD display screen, it can display the static or dynamic QR code.

- **Digital Display**:If the device supports a digital screen, it can display the current time and amount.

## Quick Start

The device supports two methods: quick access and custom firmware.

- **Quick Access**
  
  The default firmware installed on the device  supports connecting to the tms and emqx test servers of dspread.You only need to update your MQTT server parameters to the device through the TMS system to establish a connection with the new MQTT server. Please refer to the '**how to create ota package. docx**' document for specific methods.
  
  1.The default firmware subscribe topic is "**user/message/DS10DCN000001**". "DS10DCN000001" is the device serial number.
  
  2.The default subscribe message fomart:
  
  ```
  { "status": "success","orderId":"12345677","amount":1234.56 }
  ```

- **Custom Firmware**
  
  If the default firmware cannot meet your needs, you can clone this repository. And complete the setup of the compilation environment and firmware development according to the document "**project build.docx**".
  
  Project Architecture
  
  ```
  DS10/
  ├── demo/                 # Main demonstration application
  │   ├── ql-application/   # Application layer code
  │   │   └── threadx/      # ThreadX real-time operating system application
  │   │       ├── common/   # Common libraries and header files
  │   │       ├── config/   # Configuration files
  │   │       └── evb_audio/ # Main application code (audio version)
  │   ├── ql-config/        # Project configuration
  │   └── ql-cross-tool/    # Cross-compilation toolchain
  ├── doc/                  # Documentation
  │   ├── API.xlsx          # API interface table
  │   ├── DS10 API.docx     # API detailed documentation
  │   ├── How to use wifi.docx # WiFi usage instructions
  │   ├── How to create ota package.docx # TMS usage instructions
  │   └── project build.docx # Project build documentation
  └── tool/                 # Tools
  ```