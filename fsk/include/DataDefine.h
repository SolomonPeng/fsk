#ifndef __DATADEFINE_H__
#define __DATADEFINE_H__

#include "DJAcsDataDef.h"

typedef		unsigned long           DWORD;
typedef		unsigned short          WORD;
typedef		unsigned char           BYTE;
typedef     bool					BOOL;

#define bbzero(m)        memset((m),0,sizeof(m))

#define	MAX_SEND_IODATA_DTMF_LEN	16


#define SEND_MAX_COUNT 10

#define	MAX_FILE_NAME_LEN		256
#define FSK_CODE_FLAG           0xAAAA
#define FSK_CODE_LEN            256
#define FSK_CONTENT_FLAG_LEN    1
#define CLEAR_CALL_FLAG         3
//#define MAX_FSK_BUF_LEN         1024

#ifndef	WIN32
#define INVALID_SOCKET	-1
#endif

enum
{
	NUMBERLEN_MAX = 128,
	LEN_10 = 10,
	LEN_100 = 100,
	LEN_1024 = 1024,
};

typedef struct{
	int flag;
	int sd;
	int length;
	BYTE buff[1024];
}ctsimsg, *pctsimsg;

//typedef struct{
//	char msg_type=0x02;                             //报文类型
//	char end_flag;									//结束标识
//	char program_ver[2]={0x20,0x01};				//程序版本号
//	char applycation_ver[4]={0x20,0x07,0x01,0x01};	//应用版本号
//	char calldisp_flag;								//来电显示标识
//	char tsam_no[8];								//TSAM卡号
//	char sysdate[4];								//系统日期
//	char systime[3];								//系统时间
//	char seq[3];									//交易流水号
//	char trade_code[3];								//交易代码
//	char flow_code[10];								//流程代码
//	char length[2];									//有效数据长度
//	char* data;										//有效数据域
//	char mac[8];									//MAC
//}fskmsg,*pfskmsg;


enum TRUNK_STATE {
	TRK_WAITOPEN,
	TRK_FREE,			
	TRK_WAIT_ALERTCALL,
	TRK_SENDLINK,
	TRK_LINK_REQUEST,
	TRK_RECVLINKACK,
	TRK_SENDFSK,		
	TRK_RECVFSKACK,
	TRK_RECVFSK,
	TRK_SENDFSKACK,	
	TRK_SENDCTSI,
	TRK_RECVCTSI,
	TRK_CONNECT_SENDFSK,
	TRK_CONNECT_SENDFSKACK,
	TRK_HANGON,

	// new add for XMS
	TRK_WAIT_ANSWERCALL,
	TRK_WAIT_LINKOK,

	TRK_WAIT_REMOVE,

};

typedef struct
{
	// ----------------
	DeviceID_t	deviceID;
	int			iSeqID;
	int			iModSeqID;
	int			iLineState;

	DeviceID_t	VocDevID;

	// -----------------
	TRUNK_STATE	State;

	int		DtmfCount;
	char	DtmfBuf[32];

	char CallerCode[20];
	char CalleeCode[20];

	//int  FskCount;
	int loop;
	BYTE FskBuf[LEN_1024];

	BYTE syncSeqNo;
	BYTE szRandom[5];
	BYTE szSendbyFSKBuf[LEN_1024];
	BYTE szRecvbyFSKBuf[LEN_1024];
	int packLength;
	pctsimsg msg;

} TRUNK_STRUCT;

enum VOICE_STATE {
	VOC_WAITOPEN,
	VOC_FREE,
	VOC_USED,

	VOC_WAIT_REMOVE
} ;

typedef struct
{
	// ----------------
	DeviceID_t	deviceID;
	int			iSeqID;

	DeviceID_t	UsedDevID;

	// ----------------
	VOICE_STATE	State;


} VOICE_STRUCT;

typedef struct
{
	// ----------------
	DeviceID_t	deviceID;
	int			iSeqID;
	bool		bOpenFlag;		// OpenDevice成功的标志

	//
	DJ_U8		u8E1Type;
	DJ_S32		s32AlarmVal;
} PCM_STRUCT;


enum
{
	EPOS_TERMCALL = 0x00,
	EPOS_CALLOUT = 0x10,
};


enum
{
	MAIN_TIMER = 10000,
	MAIN_INTERVAL = 10,
}; 

//------------FSK报文信息类型------------------
enum 
{
	MTYPE_LINK_REQUEST = 0x81,
	MTYPE_LINK_ANSWER = 0x82,
	MTYPE_RECEIVE_DONE = 0x83,
	MTYPE_TRADE_SERVER = 0x84,
	MTYPE_TRADE_TREM = 0x87,
}; 

enum
{
	SYNCDATA = 0x55, //同步载波数据内容
	SYNCSEQNO_LINK = 0x00,  //建链请求同步码
};

enum
{
	MENUREQUEST = 0x00,  //下发菜单
	BILLREQUEST = 0x01,  //支付菜单
};

enum
{
	TERMMESSAGE = 0x00,  //终端信息
	FSKMESSAGE = 0x01,   //FSK信息
};
#endif
