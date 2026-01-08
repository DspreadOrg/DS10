#ifndef _SYSCALL_AROM_H_
#define _SYSCALL_AROM_H_

/*---------------------------------------------------------------------------*/
typedef enum {
  SYSCALL_00_REGISTER                         = 0,
  SYSCALL_01_UNREGISTER                       = 1,
  /* mutex */
  SYSCALL_02_MUTEX_TRY_LOCK                   = 2,
  SYSCALL_03_MUTEX_UNLOCK                     = 3,
  /* clock */
  SYSCALL_04_CLOCK_TIME                       = 4,
  SYSCALL_05_CLOCK_WAIT                       = 5,
  SYSCALL_06_CLOCK_WAIT_MS                    = 6,
  /* cpu */
  SYSCALL_07_HW_PLATFORM_TYPE                 = 7,
  /* interrupts */
  SYSCALL_08_UNMASK_INT                       = 8,
  SYSCALL_09_REGISTER_INT                     = 9,
  /* dma */
  SYSCALL_10_DMAC_MAP_DEVICE_TO_CHANNEL       = 10,
  SYSCALL_11_DMA_XFER                         = 11,
  SYSCALL_12_DMA_CHANNEL_RUNNING              = 12,
  SYSCALL_13_DMA_MAP_ADDR                     = 13,
  /* gpio */
  SYSCALL_14_GPIO_INIT                        = 14,
  SYSCALL_15_GPIO_READ                        = 15,
  SYSCALL_16_GPIO_SET                         = 16,
  SYSCALL_17_GPIO_CLEAR                       = 17,
  /* qspi */
  SYSCALL_18_QSPI_INIT                        = 18,
  SYSCALL_19_QSPI_REGISTER_CONFIG             = 19,
  SYSCALL_20_QSPI_LUT_CONFIG                  = 20,
  SYSCALL_21_QSPI_CMD                         = 21,
  SYSCALL_22_QSPI_READ                        = 22,
  SYSCALL_23_QSPI_WRITE                       = 23,
  SYSCALL_24_QSPI_WRITE_DMA                   = 24,
  /* secureboot */
  SYSCALL_25_SB_FIP_IMAGE_SETUP               = 25,
  SYSCALL_26_SB_VOLUME_READ                   = 26,
  SYSCALL_27_SB_AUTH_IMAGE                    = 27,
  SYSCALL_28_SB_FLASH_READ                    = 28,
  /* heapmem */
  SYSCALL_29_HEAPMEM_ALLOC                    = 29,
  SYSCALL_30_HEAPMEM_REALLOC                  = 30,
  SYSCALL_31_HEAPMEM_FREE                     = 31,
  /* process */
  SYSCALL_32_PROCESS_START                    = 32,
  SYSCALL_33_PROCESS_EXIT                     = 33,
  SYSCALL_34_PROCESS_POST                     = 34,
  SYSCALL_35_PROCESS_GET_CURRENT              = 35,
  /* smux */
  SYSCALL_36_SMUX_USE_DEVICE                  = 36,
  SYSCALL_37_SMUX_GET_DEVICE                  = 37,
  SYSCALL_38_SMUX_WRITE_STDOUT                = 38,
  SYSCALL_39_SMUX_RUNNING_MODE_SET            = 39,
  SYSCALL_40_SMUX_RUNNING_MODE_GET            = 40,
  SYSCALL_41_SMUX_REGISTER_PREAMBLE_CALLBACK  = 41,
  /* fastboot */
  SYSCALL_42_FASTBOOT_OKAY                    = 42,
  SYSCALL_43_FASTBOOT_FAIL                    = 43,
  SYSCALL_44_FASTBOOT_PUBLISH                 = 44,
  SYSCALL_45_FASTBOOT_REGISTER                = 45,
  SYSCALL_46_FASTBOOT_UNPUBLISH_PARTITION     = 46,
  SYSCALL_47_FASTBOOT_INFO                    = 47,
  SYSCALL_48_FASTBOOT_PROGRESS                = 48,
  SYSCALL_49_FASTBOOT_UPLOAD_AREA_SET         = 49,
  SYSCALL_50_FASTBOOT_BLOCK_FREE              = 50,
  SYSCALL_51_FASTBOOT_PROGRESS_STEP           = 51,
  SYSCALL_52_FASTBOOT_REBOOT_SET              = 52,
  SYSCALL_53_FASTBOOT_REBOOT_GET              = 53,
  SYSCALL_54_FASTBOOT_FORMAT                  = 54,
  /* aboot cmd */
  SYSCALL_55_ABOOT_DOWNLOAD_SCRATCH_SETUP     = 55,
  SYSCALL_56_ABOOT_REGISTER_UPLOAD_STAGE_CB   = 56,
  SYSCALL_57_ABOOT_REGISTER_PTABLE_INFO       = 57,
  /* aboot */
  SYSCALL_58_ABOOT_REGISTER_FLASHER_PROCESS   = 58,
  SYSCALL_59_ABOOT_REGISTER_REBOOT_CALLBACK   = 59,
  SYSCALL_60_ABOOT_REBOOT                     = 60,
  SYSCALL_61_ABOOT_FLASHER_MESSAGE_DEQUEUE    = 61,
  SYSCALL_62_ABOOT_FLASHER_MESSAGE_RELEASE    = 62,
  /* dbg */
  SYSCALL_63_DBG_SEND_BYTES                   = 63,
  /* etimer */
  SYSCALL_64_ETIMER_SET                       = 64,
  SYSCALL_65_ETIMER_STOP                      = 65,
  SYSCALL_66_ETIMER_RESET                     = 66,
  /* sys_close_all */
  SYSCALL_67_SYS_CLOSE_ALL                    = 67,
  /* mfp */
  SYSCALL_68_MFP_CONFIG                       = 68,
  /* lzma */
  SYSCALL_69_LZMA_DECODE                      = 69,
  SYSCALL_70_LZMAPROPS_DECODE                 = 70,
  /* arom */
  SYSCALL_71_AROM_MAIN_PROCESS                = 71,
  SYSCALL_72_AROM_PTABLE_INIT                 = 72,

  /* added after arom-crane0730 */
  /* aboot cmd */
  SYSCALL_73_ABOOT_SYS_GETVERSION             = 73,
  /* syscall */
  SYSCALL_74_GET_SYSCALL_TABLE                = 74,
  /* secureboot */
  SYSCALL_75_TRANSFER_CONTROL                 = 75,
  /* fastboot */
  SYSCALL_76_FASTBOOT_BOOT_SET                = 76,
  SYSCALL_77_FASTBOOT_BOOT_GET                = 77,
  /* qspi */
  SYSCALL_78_QSPI_SET_FLASH_OFFSET_ADDR       = 78,
  SYSCALL_79_QSPI_GET_FLASH_OFFSET_ADDR       = 79,
  /* fastboot */
  SYSCALL_80_FASTBOOT_GO_SET                  = 80,
  SYSCALL_81_FASTBOOT_GO_GET                  = 81,
  /* aboot */
  SYSCALL_82_ABOOT_BOOT                       = 82,
  SYSCALL_83_ABOOT_GO                         = 83,
  SYSCALL_84_ABOOT_REGISTER_GO_CALLBACK       = 84,
  /* secureboot */
  SYSCALL_85_SB_VOLUME_INIT                   = 85,
  SYSCALL_86_SB_VOLUME_EXIT                   = 86,
  /* fastboot */
  SYSCALL_87_FASTBOOT_DISCONNECT_SET          = 87,
  SYSCALL_88_FASTBOOT_DISCONNECT_GET          = 88,
  SYSCALL_89_FASTBOOT_STATE_SET               = 89,
  SYSCALL_90_FASTBOOT_STATE_GET               = 90,

  SYSCALL_MAX                                 = 128,
} syscall_t;
/*---------------------------------------------------------------------------*/
extern void **syscall_tables;
/*---------------------------------------------------------------------------*/
void syscall_init(void);
int syscall_register(syscall_t index, void *fn);
void syscall_unregister(syscall_t index);
void *syscall_get_handler(syscall_t index);
/*---------------------------------------------------------------------------*/
#define AROM_VER_2020_07_30  0 /* craneG or craneM */
#define AROM_VER_MAX         1 /* craneL */

int arom_getversion(void);
/*---------------------------------------------------------------------------*/

#endif
