# DS10 Project Documentation

## Project Overview

The DS10 is an embedded IoT project based on Quectel wireless communication modules, primarily used for intelligent voice interaction, network connectivity, and data transmission. The project is built on the ThreadX real-time operating system and integrates dual-mode communication capabilities for 4G LTE and WiFi, featuring rich audio processing functions.

## Project Architecture

### Overall Structure

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
│   └── project build.docx # Project build documentation
└── tool/                 # Tools
```

### Core Components

1. **Application Layer** (`demo/ql-application/threadx/evb_audio/`)
   - Main application entry: `evb_audio_main.c`
   - Network management: `module_lte.c` (LTE), `wifi.c` (WiFi)
   - Audio processing: `tts_yt_task.c` (Text-to-Speech)
   - MQTT communication: `mqtt_aliyun.c` (Aliyun IoT)
   - Display control: UI and status management related to display

2. **Common Library** (`demo/ql-application/threadx/common/`)
   - Header files: Various API interface definitions
   - Library files: Pre-compiled library files

3. **Configuration System** (`demo/ql-config/`)
   - Device configuration: Support for different hardware models
   - Build configuration: Build parameters

## Feature Characteristics

### 1. Network Connectivity
- **4G LTE Connection**: Supports 4G network connection and data transmission
- **WiFi Connection**: Supports WiFi connection and configuration
- **Dual-mode Switching**: Supports mode switching between LTE and WiFi
- **Auto Reconnection**: Automatic reconnection mechanism after network disconnection

### 2. Audio Processing
- **Text-to-Speech (TTS)**: Supports Chinese text-to-speech conversion
- **Audio Playback**: Supports multiple formats including MP3, WAV, AMR
- **Voice Recognition**: Supports voice input processing
- **Volume Control**: Adjustable audio output volume

### 3. Cloud Communication
- **MQTT Protocol**: Supports Alibaba Cloud IoT platform
- **OTA Upgrade**: Supports remote firmware upgrade
- **Data Reporting**: Device status and data reporting functionality

### 4. Hardware Interface
- **GPIO Control**: General-purpose input/output port management
- **UART Communication**: Serial communication interface
- **I2C/SPI**: Supports multiple bus communications
- **Sensor Interface**: Supports various sensor connections

### 5. Power Management
- **Low Power Mode**: Supports standby and sleep modes
- **Battery Management**: Battery level monitoring
- **Charging Management**: Charging status detection

## Build System

### Build Environment
- **Operating System**: Windows
- **Compiler**: ARM Compiler 5
- **Toolchain**: Included in the `ql-cross-tool/` directory

### Build Steps

1. **Configure Device Type**:
   ```batch
   # Select module type (EG800AKCN_91LC or EC600MEU_LA)
   # Select device type (DS10)
   ```

2. **Build Application**:
   ```batch
   cd demo
   project_build.bat
   # Select menu items for building
   # 1: build clean
   # 2: build app
   # 3: packet firmware
   ```

3. **Build Options**:
   - Clean project: `build.bat clean`
   - Compile application: `build.bat app`
   - Package firmware: `build.bat firmware`

### Configuration Files
- `config.mk`: Configure device type (default is DS10)
- `conf_devtype.h`: Device type definition
- Various `.ini` configuration files located in `ql-config/config/`

## API Interface

### Core API Categories

1. **Operating System Related**
   - Task Management: `ql_rtos_task_*`
   - Semaphores/Mutex: `ql_rtos_semaphore_*` / `ql_rtos_mutex_*`
   - Queue Management: `ql_rtos_queue_*`
   - Timers: `ql_rtos_timer_*`

2. **Audio Interface**
   - Audio Playback: `ql_audio_track_*`, `ql_play_mp3_*`, `ql_wav_play_*`
   - TTS Interface: `ql_tts_*`
   - Volume Control: `ql_set_volume`, `ql_get_volume`

3. **Network Interface**
   - LTE Connection: `ql_nw_*`, `ql_data_call_*`
   - WiFi Control: WiFi related functions
   - Socket Operations: Standard socket interface

4. **Hardware Interface**
   - GPIO Control: `ql_gpio_*`
   - UART Communication: `ql_uart_*`
   - File System: `ql_fs_*`

### Main Data Structures

- `StructTermInfo`: Device status information structure
- `AudioResInfo`: Audio resource information
- `SaleInfo`: Sales-related information
- `QL_NW_REG_STATUS_INFO_T`: Network registration status information

## Application Flow

### Startup Flow

1. **System Initialization**:
   - Hardware initialization
   - File system mounting
   - Device configuration loading

2. **Network Initialization**:
   - SIM card detection
   - Network registration
   - Data connection establishment

3. **Service Startup**:
   - Audio service initialization
   - MQTT connection establishment
   - User interface startup

### Network Management Modes

- **GPRS_MODE**: 4G network mode
- **WIFI_MODE**: WiFi network mode
- **GPRS_BAKE_MODE/WIFI_BAKE_MODE**: Backup network mode
- **Mode_NULL**: No network mode

### Audio Playback Flow

1. **TTS Playback**: Text → TTS Engine → Audio Output
2. **File Playback**: Audio File → Decoding → Audio Output
3. **Stream Playback**: Audio Stream → Real-time Decoding → Audio Output

## Device Configuration

### Supported Hardware Models

- **EC600MEU_LA**: 8+8MB configuration
- **EG800AKCN_91LC**: 4+4MB configuration

### Device Features

- **Processor**: Cortex-R5 core
- **Memory**: Varies by module model (4+4MB or 8+8MB)
- **Communication**: 4G LTE + WiFi dual-mode
- **Audio**: Built-in audio codec
- **Interfaces**: GPIO, UART, I2C, SPI, etc.

## Error Handling

### Common Error Codes

- `QL_RET_OK (0)`: Operation successful
- `QL_RET_ERR_*`: Various error codes (parameter error, port error, memory error, etc.)

### Exception Handling Mechanism

- Network disconnection auto-reconnection
- Power anomaly protection
- Hardware fault detection
- Memory overflow protection

## Maintenance and Debugging

### Logging System

- USB logging output: Output debug information through USB serial port
- Log level control: Configurable different log output levels
- Runtime status monitoring: Real-time monitoring of system operation status

### Debugging Interface

- USB CDC interface: For debugging and log output
- UART interface: Serial communication
- AT command interface: Device control and query

## Notes

1. **Power Management**: Pay attention to battery level management to avoid deep discharge
2. **Network Configuration**: Correctly configure APN parameters to ensure network connection
3. **Audio Resources**: Ensure audio resource files are complete and in correct format
4. **Memory Usage**: Allocate memory reasonably to avoid memory leaks
5. **OTA Security**: Maintain stable power during OTA upgrade process

## Version Information

- **Software Version**: V1.0.4 (according to SOFTVER macro in code)
- **Kernel Version**: 5.x
- **Build Time**: Determined by compilation time

---
This document is compiled based on the analysis of the DS10 project source code, covering the main features and usage methods of the project.