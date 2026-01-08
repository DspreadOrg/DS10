/**
  ******************************************************************************
  * @file    voice_wav_mix.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/10/29
  * @brief   wav mix func file for C header
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VOICE_WAV_MIX_H__
#define __VOICE_WAV_MIX_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
typedef struct
{
    /* chunk "riff" */
    char ChunkID[4]; /* "RIFF" */
    /* sub-chunk-size */
    uint32_t ChunkSize; /* 36 + Subchunk2Size */
    /* sub-chunk-data */
    char Format[4]; /* "WAVE" */
} RIFF_t;

typedef struct
{
    /* sub-chunk "fmt" */
    char Subchunk1ID[4]; /* "fmt " */
    /* sub-chunk-size */
    uint32_t Subchunk1Size; /* 16 for PCM */
    /* sub-chunk-data */
    uint16_t AudioFormat; /* PCM = 1*/
    uint16_t NumChannels; /* Mono = 1, Stereo = 2, etc. */
    uint32_t SampleRate; /* 8000, 44100, etc. */
    uint32_t ByteRate; /* = SampleRate * NumChannels * BitsPerSample/8 */
    uint16_t BlockAlign; /* = NumChannels * BitsPerSample/8 */
    uint16_t BitsPerSample; /* 8bits, 16bits, etc. */
} FMT_t;

typedef struct
{
    /* sub-chunk "data" */
    char Subchunk2ID[4]; /* "data" */
    /* sub-chunk-size */
    uint32_t Subchunk2Size; /* data size */
    /* sub-chunk-data */
    //    Data_block_t block;
    uint32_t DataOffset;
} WavData_t;

//typedef struct WAV_data_block {
//} Data_block_t;

typedef struct WAV_fotmat
{
    RIFF_t riff;
    FMT_t fmt;
    WavData_t data;
} Wav_t;
/* Exported define -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief  get wav info
 * @param  filename - wav file
 * @param  pinfo - output wav info
 * @retval 0 if success,otherwise error code
 * @note   none
 */
int get_wav_info(const char *filename,Wav_t *pinfo);


/**
 * @brief  set mix duration in ms
 * @param  duration_ms - duration in ms
 * @retval none
 * @note   none
 */
void wav_set_mix_duration(int duration_ms);


/**
 * @brief  play wav list by mix
 * @param  playlist - wav file list
 * @retval none
 * @note   none
 */
//void wav_mix_play_list(char **playlist);
int wav_mix_play_list(int size ,short const * group);


/**
 * @brief  create new playlist
 * @param  none
 * @retval buff of playlist
 * @note   none
 */
char *wav_mix_new_playlist(void);

/**
 * @brief  del playlist
 * @param  playlist - list to be remove
 * @retval none
 * @note   none
 */
void wav_mix_del_playlist(char *playlist);

/**
 * @brief  add an item to the end of list
 * @param  buff - list buff
 * @param  itemname - item name
 * @retval error < 0,otherwise return the insert position in playlist
 * @note   none
 */
int wav_mix_playlist_append(char *buff,char *itemname);

/**
 * @brief  start play list
 * @param  playlist - list to be play
 * @retval none
 * @note   none
 */
int wav_mix_playlist_start(char *buff);


#ifdef __cplusplus
}
#endif

#endif /* __VOICE_WAV_MIX_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
