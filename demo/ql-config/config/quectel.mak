# ---------------------------------------------------------------------
# (C) Copyright [2021-2028] Quectel Wireless Solutions Co., Ltd. 
# All Rights Reserved
# ---------------------------------------------------------------------
# 使用注意事项
# QUEC_TARGET_DFLAGS 使用注意事项
# TARGET_ARMLINK_OPT 用于定义链接阶段的宏
# VARIANT_LIST       用于控制某些功能的C源文件是否参与编译
#######################################################################

$(warning ${QUECTEL_PROJECT_DEF})

QUEC_VARIANT_LIST := $(strip ${oem_ver})

QUEC_TARGET_DFLAGS := -DQUEC_UART_DMA_TX_ENABLE -DQUEC_UART_DMA_RX_ENABLE -DQUEC_USB_ENABLE -DEXT_AT_MODEM_SUPPORT -DQUEC_USB_CDC_ENABLE -DREDUCE_SULOG_IRBUF_DSP -DSUPPORT_FBP  -DQUEC_START_UP #-DQUEC_USE_NEWLIB_REENTRANT  

ifneq (,$(findstring CRANEM,${ASR_PLATFORM}))
	#1603(1603S/1603E/1603C)
	QUEC_TARGET_DFLAGS  += -DASR_PLATFORM_CRANEM
else
	ifneq (,$(findstring CRANELR,${ASR_PLATFORM}))
		#1602
		QUEC_TARGET_DFLAGS  += -DASR_PLATFORM_CRANELR
	else
		ifneq (,$(findstring CRANELS,${ASR_PLATFORM}))
			#1609
			QUEC_TARGET_DFLAGS  += -DASR_PLATFORM_CRANELS -DQUEC_BT_SUPPORT
		else
			#1606(1606C/1606S/1606L)
			QUEC_TARGET_DFLAGS  += -DASR_PLATFORM_CRANEL
		endif
	endif
endif

ifneq (,$(findstring CRANELRH,${ASR_PLATFORM}))
#1605
	QUEC_TARGET_DFLAGS  += -DASR_PLATFORM_CRANELRH
endif

ifneq (,$(findstring CRANELG,${ASR_PLATFORM}))
#1607
	QUEC_TARGET_DFLAGS  += -DASR_PLATFORM_CRANELG
endif
ifneq (,$(findstring TRUE,${QUEC_OPEN_DEF}))
########################################################################
##
##OPEN项目功能控制
########################################################################
QUEC_TARGET_DFLAGS  += -DQUECTEL_OPEN_CPU -DQUEC_LCD_SUPPORT -DREMOVE_SS -DQUEC_DUMP_NVM_SUPPORT
QUEC_TARGET_DFLAGS += -DDM_SOC_ADC_SUPPORT #使能adc功能


ifneq (,$(findstring TRUE,${QUEC_IMS_SMS_DEF}))
VARIANT_LIST += QUEC_IMSSMS
QUEC_TARGET_DFLAGS  += -DQUECTEL_SUPPORT_IMSSMS
TARGET_ARMLINK_OPT  += --predefine="-DQUECTEL_SUPPORT_IMSSMS"
endif

ifneq (,$(findstring CRANELR,${ASR_PLATFORM}))
#1602不支持双通道，只能客户外部加上模拟开关切换
else
#控制内核在单卡协议栈，支持双卡切换单待的功能[硬件要支持双卡才行,仅仅1603/1606支持双卡通道]
QUEC_TARGET_DFLAGS += -DQUEC_PS_SINGLE_PHYSICAL_DUAL_SIM_SUPPORT
endif
  
VARIANT_LIST +=NO_ENVSIM NOPMIC802 NOPMIC813 NOLWM2M NOPAHO SD_NOT_SUPPORT NOATNET QUECTEL_OPEN_CPU LZMA_SUPPORT NOMEP REMOVE_PB LFS_SUPPORT_V2 NODIALER
PMIC_RF_T +=  NOBIP_NOPPP_REDUCESULOGIRBUFDSP 
#4+2项目默认裁剪ftp
ifneq (,$(findstring M02,${Flash_type}))
QUEC_TARGET_DFLAGS+= -DQUEC_NOFTP
endif

##AES_SUPPORT

TARGET_ARMLINK_OPT  += --predefine="-DQUECTEL_OPEN_CPU"

##OEM空 场景1:默认场景,无gps,有audio,无sms,有lcd
ifeq (,$(oem_ver)) 
	PMIC_RF_T += NOL1WIFI
	VARIANT_LIST        += REMOVE_SMS MP3_DECODE QUEC_EXT_NVM_SUPPORT NOUSBNET SPINOR_SUPPORT 
	QUEC_TARGET_DFLAGS  += -DQUEC_NVM_IN_EXT_FLASH  -DQUEC_CAMERA_SUPPORT -DQUECTEL_MINIFOTA_SUPPORT -DQUEC_APP_FULL_FOTA 
	QUEC_TARGET_DFLAGS  += -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM
	####裁剪IMS相关宏####
	VARIANT_LIST        += NOIMS NOXML
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS  -DREMOVE_CC 
	#####################
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET -DQUEC_NAND_YAFFS
	####################	
	#此宏用于使能音频降噪算法
	QUEC_TARGET_DFLAGS  += -DQUEC_AUDIO_DENOISE
	#########################
	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		VARIANT_LIST += NAND_YAFFS_SUPPORT
		PACKAGE_LIST +=  \
			quectel/lcd \
			quectel/camera \
			quectel/slas \
			quectel/yaffs2
	endif
endif

##OEM TSW 场景2:默认场景,无gps,无audio,有cssms,有wifiscan,无小系统升级
ifneq (,$(findstring TSW,${oem_ver}))
	VARIANT_LIST        +=  NOHTTPS NOAUDIO QUEC_EXT_NVM_SUPPORT SPINOR_SUPPORT NOTLS QUEC_NOTA
	QUEC_TARGET_DFLAGS  += -DQUEC_NVM_IN_EXT_FLASH  -DQUEC_APP_FULL_FOTA -DENABLE_WIFI_SCAN -DQUEC_NOTA
	QUEC_TARGET_DFLAGS  += -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM
	####裁剪IMS相关宏###
	VARIANT_LIST        += NOIMS NOXML
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS  -DREMOVE_CC 
	#####################
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET
	####################

	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		PACKAGE_LIST +=  \
			quectel/lcd \
			quectel/camera \
			quectel/slas

	endif
endif

##OEM FAF 场景2:默认场景,无gps,无audio,有cssms,无wifiscan,无小系统升级,无http,支持ftp app 全量升级
ifneq (,$(findstring FAF,${oem_ver}))
	PMIC_RF_T += NOL1WIFI
	VARIANT_LIST        +=  NOHTTPS NOAUDIO QUEC_EXT_NVM_SUPPORT SPINOR_SUPPORT NOTLS QUEC_NOTA 
	QUEC_TARGET_DFLAGS  += -DQUEC_NVM_IN_EXT_FLASH  -DQUEC_APP_FULL_FOTA  -DQUEC_NOTA
	QUEC_TARGET_DFLAGS  += -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM
	####裁剪IMS相关宏####
	VARIANT_LIST        += NOIMS NOXML
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS  -DREMOVE_CC -UQUEC_NOFTP -DQUEC_FTP_APPFOTA -DQUEC_NOHTTP
	#####################
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET
	####################

	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		PACKAGE_LIST +=  \
			quectel/lcd \
			quectel/camera \
			quectel/slas

	endif
endif

##OEM:NOMINI 场景3:默认场景,无gps,无短信，无wifiscan,无小系统升级
ifneq (,$(findstring NOMINI,${oem_ver}))
	PMIC_RF_T += NOL1WIFI
	VARIANT_LIST        += REMOVE_SMS QUEC_EXT_NVM_SUPPORT NOHTTPS SPINOR_SUPPORT NOHTTPS QUEC_NOTA MP3_DECODE
	QUEC_TARGET_DFLAGS  += -DQUEC_NOFTP -DQUEC_NVM_IN_EXT_FLASH  -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM -DQUEC_NOTA -DQUEC_APP_FULL_FOTA
	####裁剪IMS相关宏####
	VARIANT_LIST        += NOIMS NOXML
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS  -DREMOVE_CC 
	#####################
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET
	####################
	##此宏裁剪更多功能,不常使用的AT
	QUEC_TARGET_DFLAGS  += -DQUEC_LTEONLY_THIN -DNO_EXTEND_MY_Q_AT 
	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		PACKAGE_LIST +=  \
			quectel/lcd \
			quectel/camera \
			quectel/slas
	endif			
endif

##OEM=TRACK 场景4:TRACK行业版本内核,与公版的区别是支持SGS短信,支持WIFISCAN
ifneq (,$(findstring TRACK,${oem_ver}))
	VARIANT_LIST        +=  MP3_DECODE NOHTTPS 
	QUEC_TARGET_DFLAGS  += -DENABLE_WIFI_SCAN -DQUECTEL_MINIFOTA_SUPPORT -DQUEC_APP_FULL_FOTA -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM
	####裁剪IMS相关宏####
	VARIANT_LIST        += NOIMS NOXML
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS  -DREMOVE_CC 
	#####################
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET
	####################
	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		ifneq (,$(findstring HX_GPS,${flash_layout_type}))
			QUEC_TARGET_DFLAGS += -DQUEC_HX_GNSS_SUPPORT
			VARIANT_LIST  += HX_GPS
				PACKAGE_LIST +=  \
			quectel/gnss
		endif
		ifneq (,$(findstring CC1177W_GPS,${flash_layout_type}))
			QUEC_TARGET_DFLAGS += -DQUEC_HX_GNSS_SUPPORT -DHX_GNSS_CC11_SUPPORT
			VARIANT_LIST  += CC1177W_GPS
				PACKAGE_LIST +=  \
			quectel/gnss
		endif
		ifneq (,$(findstring ASR_GPS,${flash_layout_type}))
			QUEC_TARGET_DFLAGS += -DQUEC_GPS_SUPPORT -DQUEC_ASR_GNSS_SUPPORT
			VARIANT_LIST  += ASR_GPS AGPSTP_SUPPORT
				PACKAGE_LIST +=  \
			quectel/gnss
		endif 
	
	endif
endif


##OEM:USBNET 场景5:基于场景一支持USBNET，OPEN的IPC版本，4+2不外挂FLASH
ifneq (,$(findstring USBNET,${oem_ver}))
	
	TARGET_ARMLINK_OPT  += --predefine="-DQUECTEL_USBNET_SUPPORT"
	PMIC_RF_T += NOL1WIFI
	VARIANT_LIST        += REMOVE_SMS NOHTTPS QUEC_EXT_NVM_SUPPORT SPINOR_SUPPORT   
	QUEC_TARGET_DFLAGS  += -DQUECTEL_MINIFOTA_SUPPORT -DQUECTEL_USBNET_SUPPORT -DQUEC_NVM_IN_EXT_FLASH  -DEXTERNAL_SPI_NORFLASH_FS 
	##配置LWIP是否支持USBNET相关协议(DHCP等)
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM
	####裁剪IMS相关宏####
	VARIANT_LIST        += NOIMS NOXML NOAUDIO
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS -DREMOVE_CC 
	
endif

##OEM:ASR_GPS 场景6:TRACK行业版本内核,仅支持ASR5311,不支持Audio,不支持wifiscan
ifneq (,$(findstring ASR_GPS,${oem_ver}))
	PMIC_RF_T += NOL1WIFI
	VARIANT_LIST        += NOHTTPS NOAUDIO
	QUEC_TARGET_DFLAGS  += -DQUECTEL_MINIFOTA_SUPPORT -DNO_AUDIO -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM
	####裁剪IMS相关宏####
	VARIANT_LIST        += NOIMS NOXML
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS -DREMOVE_CC 
	#####################
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET
	####################
	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		ifneq (,$(findstring ASR_GPS,${flash_layout_type}))
			QUEC_TARGET_DFLAGS += -DQUEC_GPS_SUPPORT -DQUEC_ASR_GNSS_SUPPORT
			VARIANT_LIST  += ASR_GPS AGPSTP_SUPPORT
			
			PACKAGE_LIST +=  \
				quectel/gnss
		endif 
	endif
endif

##OEM空 场景7:默认场景,volte gps bt sms 
ifneq (,$(findstring VGBS,${oem_ver}))
	VARIANT_LIST        +=  MP3_DECODE QUEC_EXT_NVM_SUPPORT SPINOR_SUPPORT LCD_CONTROLLER_SUPPORT
	QUEC_TARGET_DFLAGS  += -DQUEC_NVM_IN_EXT_FLASH  -DQUEC_CAMERA_SUPPORT -DQUECTEL_MINIFOTA_SUPPORT -DQUEC_APP_FULL_FOTA -DLWIP_PLAT_LTEONLY_THIN
	QUEC_TARGET_DFLAGS  += -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM -DLWIP_PLAT_NORMAL -DQUEC_IMS_XML_SUPPORT #ims需要这个宏
	
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET
	####################
	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		PACKAGE_LIST +=  \
			quectel/lcd \
			quectel/camera\
			quectel/gnss \
			quectel/slas
			
		QUEC_TARGET_DFLAGS  += -DQUEC_GPS_SUPPORT -DQUEC_ASR_GNSS_SUPPORT
		VARIANT_LIST  += ASR_GPS AGPSTP_SUPPORT
	endif
endif

ifneq (,$(findstring BT,${oem_ver}))
	VARIANT_LIST        += MP3_DECODE REMOVE_SMS NOUSBNET QUEC_EXT_NVM_SUPPORT SPINOR_SUPPORT 
	QUEC_TARGET_DFLAGS  += -DENABLE_WIFI_SCAN -DQUECTEL_MINIFOTA_SUPPORT -DQUEC_NVM_IN_EXT_FLASH -DQUEC_APP_FULL_FOTA 
	QUEC_TARGET_DFLAGS  += -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM 
	####裁剪IMS相关宏####
	VARIANT_LIST        += NOIMS NOXML
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS  -DREMOVE_CC 
	#####################
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET 
	####################	
	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))     	
		VARIANT_LIST  += BT_SUPPORT BT_STACK_SUPPORT 
		QUEC_TARGET_DFLAGS  += -DQUEC_BT_PRODUCIBILITY
	endif

endif



ifneq (,$(findstring QUEC_1607,${oem_ver}))
	VARIANT_LIST        +=  MP3_DECODE NOHTTPS 
	QUEC_TARGET_DFLAGS  += -DENABLE_WIFI_SCAN -DQUECTEL_MINIFOTA_SUPPORT -DQUEC_APP_FULL_FOTA -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM
	####裁剪IMS相关宏####
	VARIANT_LIST        += NOIMS NOXML
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS  -DREMOVE_CC 
	#####################
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET
	####################
    ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		PACKAGE_LIST +=  \
			quectel/lcd 

	endif
	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))

		ifneq (,$(findstring ASR_GPS,${flash_layout_type}))
			QUEC_TARGET_DFLAGS += -DQUEC_GPS_SUPPORT -DQUEC_ASR_GNSS_SUPPORT
			VARIANT_LIST  += ASR_GPS AGPSTP_SUPPORT
				PACKAGE_LIST +=  \
			quectel/gnss
		endif 
	
	endif
endif

##OEM == WIFI 默认场景+WIFI
ifneq (,$(findstring WIFI,${oem_ver}))
	PMIC_RF_T += NOL1WIFI
	VARIANT_LIST        += REMOVE_SMS MP3_DECODE QUEC_EXT_NVM_SUPPORT NOUSBNET SPINOR_SUPPORT 
	QUEC_TARGET_DFLAGS  += -DQUEC_NVM_IN_EXT_FLASH  -DQUEC_CAMERA_SUPPORT -DQUECTEL_MINIFOTA_SUPPORT -DQUEC_APP_FULL_FOTA 
	QUEC_TARGET_DFLAGS  += -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM
	####裁剪IMS相关宏####
	VARIANT_LIST        += NOIMS NOXML
	QUEC_TARGET_DFLAGS  += -DMORE_ATC_SPACE_OPT -DLWIP_PLAT_NOIMS  -DREMOVE_CC 
	#####################
	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET
	####################	
	此宏用于使能音频降噪算法
	QUEC_TARGET_DFLAGS  += -DQUEC_AUDIO_DENOISE
	#########################
	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		PACKAGE_LIST +=  \
			quectel/lcd \
			quectel/camera \
			quectel/slas

	endif
	ifneq (,$(findstring CRANELR,${ASR_PLATFORM}))
		#1602 1605(LRH)
		QUEC_TARGET_DFLAGS  += -DQUEC_FGM842D_WIFI_SUPPORT
		TARGET_ARMLINK_OPT  += --predefine="-DQUEC_FGM842D_WIFI_SUPPORT"
	endif
endif

##OEM == TM : TRACKER+单开IMS短信+no custom_fs
ifneq (,$(findstring TM,${oem_ver}))
	VARIANT_LIST        +=  MP3_DECODE NOHTTPS 
	QUEC_TARGET_DFLAGS  += -DENABLE_WIFI_SCAN -DQUECTEL_MINIFOTA_SUPPORT -DQUEC_APP_FULL_FOTA -DEXTERNAL_SPI_NORFLASH_FS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM
	QUEC_TARGET_DFLAGS  += -DQUEC_IMS_XML_SUPPORT -DLWIP_PLAT_NORMAL

	####裁剪usbnet#######
	VARIANT_LIST        += NOUSBNET 
	QUEC_TARGET_DFLAGS  += -DLWIP_PLAT_SOCKET
	####################
	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		ifneq (,$(findstring HX_GPS,${flash_layout_type}))
			QUEC_TARGET_DFLAGS += -DQUEC_HX_GNSS_SUPPORT
			VARIANT_LIST  += HX_GPS
				PACKAGE_LIST +=  \
			quectel/gnss
		endif
		ifneq (,$(findstring CC1177W_GPS,${flash_layout_type}))
			QUEC_TARGET_DFLAGS += -DQUEC_HX_GNSS_SUPPORT -DHX_GNSS_CC11_SUPPORT
			VARIANT_LIST  += CC1177W_GPS
				PACKAGE_LIST +=  \
			quectel/gnss
		endif
		ifneq (,$(findstring ASR_GPS,${flash_layout_type}))
			QUEC_TARGET_DFLAGS += -DQUEC_GPS_SUPPORT -DQUEC_ASR_GNSS_SUPPORT
			VARIANT_LIST  += ASR_GPS AGPSTP_SUPPORT
				PACKAGE_LIST +=  \
			quectel/gnss
		endif 
	
	endif
endif

else
########################################################################
##
##STD项目功能控制
########################################################################





endif


TARGET_DFLAGS += ${QUEC_TARGET_DFLAGS}

