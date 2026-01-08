/**
 * @file
 * @author  Jinhua Huang <jinhuahuang@asrmicro.com>
 * @version 1.0
 *
 * @section LICENSE
 * Copyright (C) 2020, ASR microelectronics, All rights reserved.
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 */
#ifndef ABOOT_TINY_H
#define ABOOT_TINY_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __linux__
#include <sys/select.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__ || defined __MINGW32__
#if defined(BUILD_DLL)
#define ABOOT_API __declspec(dllexport)
#elif defined(USE_DLL)
#define ABOOT_API __declspec(dllimport)
#else
#define ABOOT_API
#endif
#else
#define ABOOT_API __attribute__ ((visibility("default")))
#endif

/**
 * Callback events to upper layer
 */
typedef enum {
  ABOOT_TINY_EVENT_START,
  ABOOT_TINY_EVENT_DOWNLOAD,
  ABOOT_TINY_EVENT_STOP,
  ABOOT_TINY_EVENT_PROGRESS,
  ABOOT_TINY_EVENT_STATUS,
} aboot_tiny_event_t;

/**
 * Aboot device running status
 */
#define ABOOT_TINY_STATUS_CONNECTING  "CONNECTING"
#define ABOOT_TINY_STATUS_RUNNING     "RUNNING"
#define ABOOT_TINY_STATUS_FAILED      "FAILED"
#define ABOOT_TINY_STATUS_SUCCEEDED   "SUCCEEDED"

/**
 * Aboot command/response size definition
 */
#define ABOOT_COMMAND_SZ        128
#define ABOOT_RESPONSE_SZ       128

/**
 * Aboot error code definition
 */
typedef enum {
  ABOOT_TINY_ERROR_SUCCESS,
  ABOOT_TINY_ERROR_PREAMBLE_TIMEOUT,
  ABOOT_TINY_ERROR_TRANSPORT_INIT_FAILED,
  ABOOT_TINY_ERROR_LINK_LOST,
  ABOOT_TINY_ERROR_NO_RESPONSE,
  ABOOT_TINY_ERROR_HANDLE_GETVAR,
  ABOOT_TINY_ERROR_HANDLE_DOWNLOAD,
  ABOOT_TINY_ERROR_READ_DATA,
  ABOOT_TINY_ERROR_UNEXPECT_RESPONSE,
  ABOOT_TINY_ERROR_FORCE_STOP,
} aboot_tiny_error_t;

/**
 * Callback message to upper layer
 */
typedef struct {
  aboot_tiny_event_t event;
  aboot_tiny_error_t error;
  union {
    const char *message;
    int progress;
    const char *status;
  } u;
} aboot_tiny_message_t;

/**
 * Callback function pointer type
 *
 * @param msg callback info
 *
 */
typedef void (*aboot_tiny_callback_t)(const aboot_tiny_message_t *msg);

/**
 * Function poiner type for customization layer interface
 */
typedef struct firmware_handle firmware_handle_t;
typedef int (*aboot_tiny_uart_send_t)(const uint8_t *data, size_t size);
typedef int (*aboot_tiny_firmware_read_line_t)(firmware_handle_t *handle,
                                               char *data);
typedef int (*aboot_tiny_firmware_read_data_t)(firmware_handle_t *handle,
                                               uint8_t *data, size_t size);
typedef unsigned (*aboot_tiny_clock_get_t)(void);
typedef size_t (*aboot_tiny_uart_rx_callback_t)(const uint8_t *data,
                                                size_t size);
typedef void *(*aboot_tiny_mem_alloc_t)(size_t size);
typedef void (*aboot_tiny_mem_free_t)(void *ptr);
typedef int (*aboot_tiny_log_printf_t)(const char *format, ...);
typedef int (*aboot_tiny_usleep_t)(unsigned long usec);

/**
 * Global function pointer should be assign by porting implementation
 */
extern aboot_tiny_uart_send_t aboot_tiny_uart_send;
extern aboot_tiny_firmware_read_line_t aboot_tiny_firmware_read_line;
extern aboot_tiny_firmware_read_data_t aboot_tiny_firmware_read_data;
extern aboot_tiny_clock_get_t aboot_tiny_clock_get;
extern aboot_tiny_mem_alloc_t aboot_tiny_mem_alloc;
extern aboot_tiny_mem_free_t aboot_tiny_mem_free;
extern aboot_tiny_log_printf_t aboot_tiny_log_printf;
extern aboot_tiny_usleep_t aboot_tiny_usleep;

#ifndef WITH_CONTIKI

/**
 * System main loop needed for non-protothread environment
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_main_loop(void);

#ifdef __linux__
struct select_callback {
  int (*set_fd)(fd_set *fdr, fd_set *fdw);
  void (*handle_fd)(fd_set *fdr, fd_set *fdw);
};
int aboot_tiny_select_set_callback(int fd, const struct select_callback *callback);
#endif

#endif /* WITH_CONTIKI */

/**
 * Initialize base platform environment
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_platform_init(void);

/**
 * Callback for uart rx data ready.
 *
 * @param data uart rx data pointer
 * @param size uart rx data size
 *
 * @return data size processed
 *
 */
size_t ABOOT_API aboot_tiny_uart_rx_callback(const uint8_t *data, size_t size);

/**
 * Get aboot max download size.
 *
 * @return max download size
 *
 */
size_t ABOOT_API aboot_tiny_get_max_download_size(void);

/**
 * Set aboot link lost check timeout.
 *
 * @param second link lost check timeout in seconds
 *
 */
void ABOOT_API aboot_tiny_set_link_lost_timeout(int second);

/**
 * Set aboot command response check timeout.
 *
 * @param second command response check timeout in seconds
 *
 */
void ABOOT_API aboot_tiny_set_cmd_response_timeout(int second);

/**
 * Initialize aboot tiny engine.
 *
 * @param cb callback function pointer
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_init(aboot_tiny_callback_t cb);

/**
 * Start aboot download engine.
 *
 * @param progress_fixup downloading extra image progress
 * @param reboot if reboot after completed
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_start(firmware_handle_t *firmware,
                               size_t progress_fixup, bool reboot);

/**
 * Begin aboot download process
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_download(void);

/**
 * Stop aboot download engine.
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_stop(void);

/**
 * Cleanup aboot tiny engine.
 */
int ABOOT_API aboot_tiny_exit(void);

/**
 * Check if aboot tiny engine is running
 *
 * @return true for running, otherwise false
 */
bool aboot_tiny_is_running(void);

#ifdef  __cplusplus
}
#endif

#endif /* ABOOT_TINY_H */
