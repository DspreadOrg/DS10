#ifndef _RES_H_
#define _RES_H_

typedef struct Download_header {
uint32_t magic_number; /* magic number 魔数 YMZN*/
uint32_t version; /* version: 0.0.0.0 文件版本号*/
uint16_t header_chksum; /* header checksum 头校验和   (64个字节 chksum)*/
uint16_t data_chksum; /* data checksum 数据域校验值 （文件内容校验)*/
uint32_t data_size; /* data size 文件长度，不含头部*/
uint16_t attribute; /* attribute  0为APP，1为语音资源，2为TTS库，3为OTA APP*/
uint16_t index; /*项目内建存储位置索引号。默认为FFFF，此时以文件名索引*/
int next_addr; /*下一文件寻址，相对当前头部的偏移量，为0则当前文件后无后续文件，后续可能增加校验或其它信息，所以可能大于等于头部长度与文件体长度之和。*/ 
uint32_t Resv[2]; /* section ID 保留*/
uint32_t FileName[8]; /* private data 文件名，索引号为FFFF时存在*/
}download_header_t;


#define FILE_HEADER 0
#define FILE_DATA  1

#define	RESPRG	0
#define	RESWAV	1

struct rev_status {
	uint8_t revDataStatus;
    uint8_t revLen;
	uint32_t offset;
	uint32_t writeAddr;
	uint32_t BaseAddr;
	uint16_t checkSum;
};	

#define HEADER_SIZE  sizeof(struct Download_header)

extern int getFileDataAddrFromFlash(char const *content,uint32_t *fileLen);

extern int getFileDataAddrFromTable(char const *content,uint32_t *fileLen);

extern uint32_t getFileDataFromFlash(uint32_t addr,char *data,uint32_t len);

extern int getFileDataPathFromFlash(char const *content,char *buff, uint32_t buffsize);

extern struct rev_status rev_s;
extern struct Download_header download_header;
extern int IdxRangChk(void);
extern int ResTabChk(void);
extern uint32_t getResVer(void);
extern int getResCount(uint16_t type);
extern int check_wav_file( char const *gbk_content, char *buff, int bufsize );

#endif // _YM_RESOURCE_H_
