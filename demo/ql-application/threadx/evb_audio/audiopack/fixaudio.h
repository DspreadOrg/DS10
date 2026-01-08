#ifndef __FIXAUDIO_H__
#define __FIXAUDIO_H__


////#define PlayBuildinAudio
typedef struct{
	char const * Data;
	unsigned int dlen;
}AudioArrayInfo;

typedef struct {
	const unsigned int Idx;
	const char * TTSstr;
	const AudioArrayInfo * Array;
	const char *filePinyinName;
}AudioResInfo;

#define VOICE_PATH_PREFIX       "B:/"

extern const AudioResInfo *GetRecInfo(int Idx);

extern const AudioArrayInfo AudioGcmsInfo;
extern const AudioArrayInfo AudioYlcsInfo;
extern const AudioArrayInfo AudioZhengzgjInfo;
extern const AudioArrayInfo AudioKeyBeepInfo;
extern const AudioArrayInfo AudioWelcomeInfo;
extern const AudioArrayInfo AudioInputErrInfo;
extern const AudioArrayInfo AudioCswcInfo;
extern const AudioArrayInfo AudioMP3_1KHZInfo;

#define AudioKeyErrBeepInfo AudioKeyBeepInfo

typedef enum
{
	AUD_ID_GROUP_IDX,
	AUD_ID_SALE_ID_INFO,
	AUD_ID_SALE_STR_INFO,
	AUD_ID_RECODE_PLAY,
	
	AUD_ID_FIX_AUDIO_BASE=2000,

	AUD_ID_FAC_TEST_MODE,
	AUD_ID_FAC_1KHZ,
	AUD_ID_FAC_TEST_AUDIO,
	//0
	AUD_ID_NUM_0,
	//1
	AUD_ID_NUM_1,
	//2
	AUD_ID_NUM_2,
	//3
	AUD_ID_NUM_3,
	//4
	AUD_ID_NUM_4,
	//5
	AUD_ID_NUM_5,
	//6
	AUD_ID_NUM_6,
	//7
	AUD_ID_NUM_7,
	//8
	AUD_ID_NUM_8,
	//9
	AUD_ID_NUM_9,
	//TTS - ʮ
	AUD_ID_NUM_10,
	//TTS - ��
	AUD_ID_NUM_11,
	AUD_ID_NUM_12,
	AUD_ID_NUM_13,
	AUD_ID_NUM_14,
	AUD_ID_NUM_15,
	AUD_ID_NUM_16,
	AUD_ID_NUM_17,
	AUD_ID_NUM_18,
	AUD_ID_NUM_19,
	AUD_ID_NUM_20,
	AUD_ID_NUM_30,
	AUD_ID_NUM_40,
	AUD_ID_NUM_50,
	AUD_ID_NUM_60,
	AUD_ID_NUM_70,
	AUD_ID_NUM_80,
	AUD_ID_NUM_90,
	AUD_ID_HUNDRED,
	//TTS - 
	AUD_ID_THOUSAND,
	//TTS - 
	AUD_ID_MILLION,
	//TTS - 
	AUD_ID_DIAN,
	//TTS - Ԫ
	AUD_ID_AND,

	AUD_ID_DOT,
	AUD_ID_UNIT,
	AUD_ID_PAYMENT,
	////TTS - 
	AUD_ID_LAKALA_PAY,
	AUD_ID_WEIXIN_PAY,
	AUD_ID_ZHIFUBAO_PAY,
	AUD_ID_YUNSHANFU_PAY,
	AUD_ID_REFUND,
	//TTS - 
	AUD_ID_RECORD_DI,
	//TTS - 
	AUD_ID_RECORD_BI,

	AUD_ID_WELCOME,
	AUD_ID_WELCOME_1,
	AUD_ID_WISH_BUSINESS,
	AUD_ID_KEY_BEEP,
	AUD_ID_KEY_ERR_BEEP,
	AUD_ID_LOG_MODE_USB,
	AUD_ID_OR,
	AUD_ID_PLEASE_WAITING,
	AUD_ID_SIM_NOT_FOUND,
	AUD_ID_SIM_INSERT_REQ,
	AUD_ID_NET_FAULT,
	AUD_ID_NET_CONNECTING,
	AUD_ID_NET_CONNECT_FAIL,
	AUD_ID_NET_CONNECT_SUCESS,
	AUD_ID_NET_NOT_CONNECT,
	AUD_ID_NET_CONNECT,
	AUD_ID_NET_MODE_GPRS,
	AUD_ID_NET_MODE_WIFI,
	AUD_ID_NET_MODE_WIFI_AIRKISS,
	AUD_ID_NET_MODE_WIFI_AP,
	AUD_ID_MQTT_CONNECT_SUCESS,
	AUD_ID_MQTT_CONNECT_FAIL,
	AUD_ID_HOW_GPRS_TO_WIFI,
	AUD_ID_HOW_WIFI_TO_GPRS,
	AUD_ID_HOW_WIFI_TO_AP,
	AUD_ID_HOW_WIFI_TO_AIRKISS,
	AUD_ID_EXIT_WIFI_SET,
	AUD_ID_PWR_LOWBAT,
	AUD_ID_PWR_OFF,
	AUD_ID_PWR_LOWBATOFF,
	AUD_ID_VOL_MAX,
	AUD_ID_VOL_MIN,
	AUD_ID_RECODE_MODE,
	AUD_ID_EXIT_RECODE,
	AUD_ID_RECODE_NOT_FOUND,
	AUD_ID_RECODE_FIRST,
	AUD_ID_RECODE_LAST,
	AUD_ID_RECOVER_FACTORY,
	AUD_ID_PARAM_EMPTY,
	AUD_ID_ACTIVE_FAIL,
	AUD_ID_ACTIVE_SUCESS,
	AUD_ID_CHARGE_IN,
	AUD_ID_CHARGE_OUT,
	AUD_ID_CHARGE_FULL,
	AUD_ID_UPDATE_START,
	AUD_ID_UPDATE_SUCESS,
	AUD_ID_UPDATE_FAIL,

	AUD_ID_MAX
}AudioPlayIdxDef;


#define STR_ID_WEIXIN_PAY			"΢���տ�"
#define STR_ID_ZHIFUBAO_PAY			"֧�����տ�"
#define STR_ID_YUNSHANFU_PAY		"�������տ�"
#define STR_ID_REFUND		        "�˿�"
#define STR_ID_TTS_PLAY_PAY         "TTS����"


#define STR_ID_FAC_TEST_MODE							"����ģʽ"
#define STR_ID_FAC_TEST_COMPLETE					"�������"
#define STR_ID_FAC_TEST_AUDIO							"��������"
//#define STR_ID_FAC_TEST_TTS								"TTS��������"
#define STR_ID_FAC_TEST_PWROFF						"���ڹػ�"
#define STR_ID_FAC_TEST_GET_VESION_FAIL		"��ȡ�汾ʧ��"
#define STR_ID_FAC_TEST_SIM_NOT_FOUND			"�����SIM��"
#define STR_ID_FAC_TEST_GET_LEV_FAIL			"��ȡ�ź�ǿ��ʧ��"
#define STR_ID_FAC_TEST_TTS_ACTIVEOK			"TTS����ʧ��"

#define STR_ID_CUR_SIGNAL					"��ǰ�ź�ֵ"

#define STR_ID_SALE_CANCLE		       "�û�ȡ��֧��"
#define STR_ID_NUM_0					"0"
#define STR_ID_NUM_1					"1"	
#define STR_ID_NUM_2					"2"	
#define STR_ID_NUM_3					"3"
#define STR_ID_NUM_4					"4"
#define STR_ID_NUM_5					"5"
#define STR_ID_NUM_6					"6"
#define STR_ID_NUM_7					"7"
#define STR_ID_NUM_8					"8"
#define STR_ID_NUM_9					"9"
#define STR_ID_SHI						"ʮ"
#define STR_ID_BAI						"��"
#define STR_ID_QIAN						"ǧ"
#define STR_ID_WAN						"��"
#define STR_ID_DIAN						"��"
#define STR_ID_YUAN						"Ԫ"

#define  STR_ID_TTS_PLAY_PAY          "tts����"

#define STR_ID_LAKALA_PAY			"����������"
#define STR_ID_WEIXIN_PAY			"΢�ŵ���"
#define STR_ID_ZHIFUBAO_PAY			"֧��������"
#define STR_ID_SUCESS_PAY			"�տ�ɹ�"
#define STR_ID_YSF_PAY			    "����������"

#define STR_ID_RECORD_DI			"��"
#define STR_ID_RECORD_BI			"��"

#define STR_ID_KEY_BEEP					"��"
#define STR_ID_KEY_ERR_BEEP			"�"
#define STR_ID_OR								"��"
#define STR_ID_LOG_MODE_USB			"USB��־ģʽ���ȴ�����"
#define STR_ID_WELCOME					"��ӭʹ��������"
#define STR_ID_WISH_BUSINESS		"ף��������¡"
#define STR_ID_PLEASE_WAITING		"���Ժ�"

#define STR_ID_SIM_INSERT				"SIM���Ѳ���"
#define STR_ID_SIM_REMOVED			"SIM�����Ƴ�"
#define STR_ID_SIM_NOT_FOUND		"δ��⵽SIM��"
#define STR_ID_SIM_INSERT_REQ		"�����SIM��"

#define STR_ID_NET_FAULT							"������ϣ���������״̬"
#define STR_ID_NET_CONNECTING					"������������"
#define STR_ID_NET_CONNECT_FAIL				"��������ʧ�ܣ���������"
#define STR_ID_NET_CONNECT_SUCESS			"�������ӳɹ�"
#define STR_ID_NET_NOT_CONNECT				"�����쳣"/*����δ����*/
#define STR_ID_NET_CONNECT						"��������"/*����������*/
#define STR_ID_NET_MODE_GPRS					"������GPRS����ģʽ"
#define STR_ID_NET_MODE_WIFI					"������WIFI����ģʽ"
#define STR_ID_NET_MODE_WIFI_AIRKISS	"������WIFI��������ģʽ"
#define STR_ID_NET_MODE_WIFI_AP				"������WIFI AP����ģʽ"
#define STR_ID_MQTT_CONNECT_SUCESS		"�������ӳɹ�"
#define STR_ID_MQTT_CONNECT_FAIL			"��������ʧ��"

#define STR_ID_HOW_GPRS_TO_WIFI				"�������ܼ��л���WIFI����ģʽ"
#define STR_ID_HOW_WIFI_TO_GPRS				"�������ܼ��л���GPRS����ģʽ"
#define STR_ID_HOW_WIFI_TO_AP					"˫�����ܼ��л���WIFI AP����ģʽ"
#define STR_ID_HOW_WIFI_TO_AIRKISS		"˫�����ܼ��л���WIFI ��������ģʽ"
#define STR_ID_EXIT_WIFI_SET					"���˳�����"

#define STR_ID_PWR_LOWBAT				"�����ͣ�����"
#define STR_ID_PWR_OFF					"���ڹػ�"
#define STR_ID_PWR_LOWBATOFF				"���������ڹػ�"
#define STR_ID_PWR_PER					"��ǰʣ������ٷ�֮"

#define STR_ID_CHARGE_IN				"�����"
#define STR_ID_CHARGE_OUT				"������Ƴ�"
#define STR_ID_CHARGE_FULL			"��������"

#define STR_ID_UPDATE_START			"������������"
#define STR_ID_UPDATE_SUCESS		"�����ɹ�"
#define STR_ID_UPDATE_FAIL			"����ʧ��"

#define STR_ID_ACTIVE_FAIL			"���伤��ʧ�ܣ������˵���鼤���豸"	
#define STR_ID_ACTIVE_SUCESS		"���伤��ɹ�"	

#define STR_ID_VOL_MAX					"�������"	
#define STR_ID_VOL_MIN					"������С"	

#define STR_ID_RECODE_MODE 			"�밴��������ѯ���10�ʽ��ף��������ܼ��˳���ѯ"
#define STR_ID_EXIT_RECODE			"���˳���ѯ"	
#define STR_ID_RECODE_NOT_FOUND	"�����տ��¼"	
#define STR_ID_RECODE_FIRST			"���һ��"
#define STR_ID_RECODE_LAST			"���һ��"
#define STR_ID_RECOVER_FACTORY	"���ڻָ���������"
#define STR_ID_PARAM_EMPTY			"��д���豸����"

#define STR_ID_GET_SIGNAL_FAIL	"��ȡ��ǰ�ź�ֵʧ��"

#define STR_ID_YUENONG_E_PAY	"��ũE֧������"

#define STR_ID_NUM_11					"ʮһ"
#define STR_ID_NUM_12					"ʮ��"
#define STR_ID_NUM_13					"ʮ��"
#define STR_ID_NUM_14					"ʮ��"
#define STR_ID_NUM_15					"ʮ��"
#define STR_ID_NUM_16					"ʮ��"
#define STR_ID_NUM_17					"ʮ��"
#define STR_ID_NUM_18					"ʮ��"
#define STR_ID_NUM_19					"ʮ��"
#define STR_ID_NUM_20					"��ʮ"
#define STR_ID_NUM_30					"��ʮ"
#define STR_ID_NUM_40					"��ʮ"
#define STR_ID_NUM_50					"��ʮ"
#define STR_ID_NUM_60					"��ʮ"
#define STR_ID_NUM_70					"��ʮ"
#define STR_ID_NUM_80					"��ʮ"
#define STR_ID_NUM_90					"��ʮ"
#define STR_ID_AND						"��"
#define STR_ID_CENT						"����"

#define STR_ID_DOT						"dot"
#define STR_ID_UNIT						"dollar"
#define STR_ID_AND			"and"
#define STR_PAY_MSG_SK			"Collection"

#endif