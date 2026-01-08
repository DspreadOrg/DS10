/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#ifndef _QL_API_MAP_COMMON_H
#define _QL_API_MAP_COMMON_H

#ifdef __SDK_API_DEBUG__
#define api_notsup "%s() is not supported\r\n"
#define SDK_API_DEBUG_NOTSUP() ql_trace(api_notsup, (char*)__FUNCTION__)
#else
#define SDK_API_DEBUG_NOTSUP()
#endif
typedef enum QL_API_MAP_PARAMETER
{
    QL_API_MAP_GET_KERNEL_VERSION     = 0,
    QL_API_MAP_GET_KERNEL_BUILD_TIME     = 1,

    QL_API_MAP_PARAMETER_MAX          = 100,
}QL_API_MAP_PARAMETER_E;
extern void *ql_api_map_caller(QL_API_MAP_PARAMETER_E index, char *buffer);
extern int ym_get_config_current_device_type(void);
#endif

