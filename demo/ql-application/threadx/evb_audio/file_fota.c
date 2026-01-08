#include <stdio.h>
#include "ql_rtos.h"
#include "ql_power.h"
#include "ql_data_call.h"
#include "ql_application.h"
#include "ql_fs.h"
#include "ql_fota.h"
#include "public_api_interface.h"
#include "HTTPClient/HTTPCUsr_api.h"
#include "prj_common.h"

#define DFOTA_PKG_PATH "U:/dfota.bin"
#define FILE_READ_BUF_LEN 1024

#define fota_exam_log usb_log_printf

extern bool audio_pause_play;
int Ext_Wifi_AppOta(char * url)
{
	fota_exam_log("========== fota start ==========\r\n");
    qlFotaImgProcCtxPtr ctx = NULL;
    //QFILE * fp = NULL;
    char *file_read_buf = NULL;
    int ret;
    int filesize = 0;
    int file_read_total_len = 0;
    int per=-1;

    uint8_t eof;
    unsigned int recSize;
    HTTPParameters *clientParams = NULL;

    ret = -1;
    file_read_buf = malloc( FILE_READ_BUF_LEN );
    if ( !file_read_buf )
    {
        fota_exam_log( "*** heap memory is not enough ***\r\n" );
        goto exit;
    }

    clientParams = malloc( sizeof( *clientParams ) );
    if ( !clientParams )
        goto exit;
    memset( clientParams, 0, sizeof(HTTPParameters) );
    strcpy( clientParams->Uri, url );

    if( LFS_ERR_OK == ql_access( AUDIO_RESOURCE_ROOT_PATH"FotaFile.bin", 0 ) )
    {
        if (LFS_ERR_OK!=ql_remove(AUDIO_RESOURCE_ROOT_PATH"FotaFile.bin"))
        	fota_exam_log("%s line %d,FotaFile.bin removed fail!\r\n",__func__,__LINE__);
        else
        	fota_exam_log("%s line %d,FotaFile.bin removed!\r\n",__func__,__LINE__);
    }

    ctx = ql_fota_init( );
    if ( !ctx )
    {

        fota_exam_log( "*** fota init fail ***\r\n" );
        goto exit;
    }

    audio_pause_play = true;

    eof = 0;
    while ( 1 )
    {
        recSize=0;
        ret = HTTPC_get( clientParams, (CHAR*) file_read_buf, (INT32) FILE_READ_BUF_LEN, (INT32*) &recSize );
        if ( ret == HTTP_CLIENT_SUCCESS )
            eof = 0;
        else if ( ret == HTTP_CLIENT_EOS )
            eof = 1;
        else
        {
            ret = -1;			
			break;
        }
        if ( recSize )
        {
            file_read_total_len += recSize;
            ret = ql_fota_image_write( ctx, (void*) file_read_buf, recSize );
            if ( ret )
            {
                fota_exam_log( "*** fota image write fail ***\r\n" );
                ret = -2;
                goto exit;
            }
			if (filesize==0)
			{
				HTTP_PARAM      HTTPParam;                                  // A generic pointer\length parameter for parsing
				if (HTTPIntrnHeadersFind((P_HTTP_SESSION)clientParams->pHTTP,"content-length",&HTTPParam,TRUE,0)==HTTP_CLIENT_SUCCESS)
				{
					//char buf[30];
					//snprintf(buf,sizeof(buf),"%.*s",(int)HTTPParam.nLength,HTTPParam.pParam);
					fota_exam_log("found headin:%.*s\r\n",(int)HTTPParam.nLength,HTTPParam.pParam);
					sscanf(HTTPParam.pParam,"%*[^0-9]%d",&filesize);
				}
				else
				{
					printf("content-length not found!\r\n");
					filesize=700*1024;
				}
				fota_exam_log("filesize:%d\r\n",filesize);
			}

			if (per!=file_read_total_len * 100/filesize)
			{
				per=file_read_total_len * 100/filesize;
				if (per>100) per=100;
				disp_set_ota_state(per);
				ql_rtos_task_sleep_ms(20);//����ʾ
			}
        }

        if ( eof )
        {
            fota_exam_log( "%s_%d =========eof ok\n", __func__, __LINE__ );
            break;
        }
    }

    fota_exam_log( "%s_%d =========%d\n", __func__, __LINE__, file_read_total_len );
    if ( ( eof ) && ( file_read_total_len ) )
    {
        ret = ql_fota_image_flush( ctx );
        if ( ret )
        {
			fota_exam_log( "*** fota image flush fail ***\r\n" );
            ret = -3;
            goto exit;
        }

        disp_set_ota_state(100);
        ql_rtos_task_sleep_ms(100);
        fota_exam_log( "fota image write done, verifing ...\r\n" );

        ret = ql_fota_image_verify( ctx );
        if ( ret )
        {
            fota_exam_log( "*** fota image verify fail ***\r\n" );
            ret = -3;
            goto exit;
        }

        fota_exam_log( "fota image verify done, will restart to update ...\r\n" );

    }

    if ( ret < 0 )
    {
        fota_exam_log( "*** dfota pkg file read fail ***\r\n" );
        goto exit;
    }
	
	
exit:
    audio_pause_play = false;
    HTTPC_close( clientParams );
	if(ctx) ql_fota_deinit(ctx);
	if(file_read_buf) free(file_read_buf);
	if(clientParams) free(clientParams);
	fota_exam_log("========== fota end ==========\r\n");
	return ret;
}

