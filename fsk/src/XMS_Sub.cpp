
#include "DJAcsDataDef.h"
#include "DJAcsAPIDef.h"
#include "DJAcsDevState.h"

#include "XMS_Sub.h"
#include "XMS_Event.h"
#include "XMS_String.h"

#include "tools.h"
#include "axmscurses.h"
#include "WrLog.h"
#include "Connect.h"
#include "FSK_Sub.h"

#include <unistd.h>


// -----------------------------------------------------------------------
// variable declaration

char			cfg_IniName[] = "./XMS_Fsk.ini";

ServerID_t		cfg_ServerID;
char			cfg_VocPath[128];
int				cfg_iDispChnl;
int				cfg_iVoiceRule;

int				cfg_iPartWork;
int				cfg_iPartWorkModuleID;

int				cfg_s32DebugOn;

// var about work
ACSHandle_t		g_acsHandle = 0;
DJ_U8			g_u8UnitID = 2;

// var in XMS_Event.CPP
extern TYPE_XMS_DSP_DEVICE_RES_DEMO	AllDeviceRes[MAX_DSP_MODULE_NUMBER_OF_XMS];

extern int						g_iTotalPcm;
extern int						g_iTotalPcmOpened;
extern TYPE_CHANNEL_MAP_TABLE	MapTable_Pcm[MAX_PCM_NUM_IN_THIS_DEMO];

extern int						g_iTotalTrunk;
extern int						g_iTotalTrunkOpened;
extern TYPE_CHANNEL_MAP_TABLE	MapTable_Trunk[MAX_TRUNK_NUM_IN_THIS_DEMO];

extern int						g_iTotalVoice;
extern int						g_iTotalVoiceOpened;
extern int						g_iTotalVoiceFree;
extern TYPE_CHANNEL_MAP_TABLE	MapTable_Voice[MAX_TRUNK_NUM_IN_THIS_DEMO];

extern int						g_iTotalModule;
extern DJ_S8					MapTable_Module[MAX_DSP_MODULE_NUMBER_OF_XMS];

// var in about Platform
extern char							g_PlatformIP[32];
extern	int							g_PlatformPort;
extern	int							g_PlatformTimeout;
char								g_CallerCode[20];

// -----------------------------------------------------------------------
// local function decalration
void	ReadFromConfig(void);


TIni g_cfg;
int  windownum = 15;
int  colnum = 9;
int  g_ymove;

// -----------------------------------------------------------------------
void	ReadFromConfig(void)
{		
	g_cfg.OpenIniFile(cfg_IniName);

	//GetPrivateProfileString ( "ConfigInfo", "IpAddr", "192.168.0.8", cfg_ServerID.m_s8ServerIp, sizeof(cfg_ServerID.m_s8ServerIp), cfg_IniName);
	read_string(g_cfg, "ConfigInfo", "IpAddr",  cfg_ServerID.m_s8ServerIp, sizeof(cfg_ServerID.m_s8ServerIp));

	//cfg_ServerID.m_u32ServerPort = GetPrivateProfileInt ( "ConfigInfo", "Port", 9001, cfg_IniName);
	read_int(g_cfg, "ConfigInfo", "Port",(int&)cfg_ServerID.m_u32ServerPort );

	read_string(g_cfg,"ConfigInfo","UserName",cfg_ServerID.m_s8UserName,sizeof(cfg_ServerID.m_s8UserName));
	read_string(g_cfg,"ConfigInfo","PassWord",cfg_ServerID.m_s8UserPwd,sizeof(cfg_ServerID.m_s8UserPwd));

	read_string(g_cfg, "ConfigInfo", "VocPath", cfg_VocPath, sizeof(cfg_VocPath));

	read_int(g_cfg, "ConfigInfo", "DispChnl", (int&)cfg_iDispChnl);
	read_int(g_cfg, "ConfigInfo", "VoiceRule",(int&)cfg_iVoiceRule );
	read_int(g_cfg, "ConfigInfo", "PartWork", (int&)cfg_iPartWork );
	read_int(g_cfg, "ConfigInfo", "PartWorkModuleID",(int&)cfg_iPartWorkModuleID );
	read_int(g_cfg, "ConfigInfo", "DebugOn",(int&)cfg_s32DebugOn );

	read_string(g_cfg, "Platform", "IpAddr", g_PlatformIP, sizeof(g_PlatformIP));
	read_string(g_cfg, "Platform", "LocalTelNum", g_CallerCode, sizeof(g_CallerCode));
	read_int(g_cfg, "Platform", "Port",(int&)g_PlatformPort );
	read_int(g_cfg, "Platform", "TimeOut",(int&)g_PlatformTimeout );
}



void AddMsg ( char *str)
{
	wrLog(str);
}

// --------------------------------------------------------------------------------

int		CalDispRow ( int iSeqID )
{
	int	iRet;

	if ( cfg_iDispChnl == 30 )
	{
		if ( (iSeqID % 16) == 0 )
			return -1;
	
		iRet = (iSeqID / 32) * 30 + (iSeqID % 32);
		iRet --;
		if ( (iSeqID % 32) > 16 )
			iRet --;
		return iRet;

	}

	if ( cfg_iDispChnl == 31 )
	{
		if ( (iSeqID % 32) == 0 )
			return -1;

		iRet = (iSeqID / 32) * 31 + (iSeqID % 32);
		iRet --;
		return iRet;
	}

	return iSeqID;
}

void DrawMain_LineState( TRUNK_STRUCT *pOneTrunk )
{
	int	 iDispRow;

	iDispRow = CalDispRow(pOneTrunk->iSeqID); 
	if ( iDispRow < 0 )	return;

	if (   iDispRow >= g_ymove 
	    && iDispRow <= (g_ymove+windownum -1))
	    draw_main_linestate(windownum,iDispRow,g_ymove,pOneTrunk);
}

void DrawMain_State( TRUNK_STRUCT *pOneTrunk )
{
	int	 iDispRow;

	iDispRow = CalDispRow(pOneTrunk->iSeqID); 
	if ( iDispRow < 0 )	return;
	
	if (   iDispRow >= g_ymove 
	    && iDispRow <= (g_ymove+windownum -1))
		draw_main_trunkstate(windownum,iDispRow,g_ymove,pOneTrunk);
}

void DrawMain_CallInfo( TRUNK_STRUCT *pOneTrunk )
{
	int	 iDispRow;

	iDispRow = CalDispRow(pOneTrunk->iSeqID); 
	if ( iDispRow < 0 )	return;

	if (   iDispRow >= g_ymove 
	    && iDispRow <= (g_ymove+windownum -1))
		draw_main_called(windownum,iDispRow,g_ymove,pOneTrunk);

	if (   iDispRow >= g_ymove 
	    && iDispRow <= (g_ymove+windownum -1))
		draw_main_calling(windownum,iDispRow,g_ymove,pOneTrunk);
}

void DrawMain_DTMF( TRUNK_STRUCT *pOneTrunk )
{
	int	 iDispRow;

	iDispRow = CalDispRow(pOneTrunk->iSeqID); 
	if ( iDispRow < 0 )	return;

	if (   iDispRow >= g_ymove 
	    && iDispRow <= (g_ymove+windownum -1))
		draw_main_dtmf(windownum,iDispRow,g_ymove,pOneTrunk); 

}

void DrawMain_VocInfo( TRUNK_STRUCT *pOneTrunk )
{
	int	 iDispRow;
	DeviceID_t	*pDev;

	iDispRow = CalDispRow(pOneTrunk->iSeqID); 
	if ( iDispRow < 0 )	return;

	if (   iDispRow >= g_ymove 
	    && iDispRow <= (g_ymove+windownum -1))
	    draw_main_vocinfo(windownum,iDispRow,g_ymove,pOneTrunk);

}




// -------------------------------------------------------------------------------------------------
void	ReDrawAll (void)
{
	char	TmpStr[256];
	int		i, iDispRow;
	TRUNK_STRUCT	*pOneTrunk;

	for ( i = 0; i < g_iTotalTrunk; i ++ )
	{
		iDispRow = CalDispRow ( i ); 
		if ( iDispRow < 0 )	continue;

		pOneTrunk = &M_OneTrunk(MapTable_Trunk[i]);
		
		// 固定不变的内容
		sprintf ( TmpStr, "%3d", iDispRow );
		draw_main_id(windownum,i,g_ymove,pOneTrunk);

		sprintf ( TmpStr, "%d,%d", MapTable_Trunk[i].m_s8ModuleID, MapTable_Trunk[i].m_s16ChannelID );
		draw_main_modch(windownum,i,g_ymove,pOneTrunk);

		
		draw_main_type(windownum,i,g_ymove,pOneTrunk);

		// 可能改变的内容
		
		draw_main_linestate(windownum,i,g_ymove,pOneTrunk);
		draw_main_trunkstate(windownum,i,g_ymove,pOneTrunk);
		draw_main_called(windownum,i,g_ymove,pOneTrunk);
		draw_main_calling(windownum,i,g_ymove,pOneTrunk);
		draw_main_dtmf(windownum,i,g_ymove,pOneTrunk);
	}
	
}

// --------------------------------------------------------------------------------
bool	InitSystem()
{
	RetCode_t	r;
	char		MsgStr[160];

	ReadFromConfig();
	
	r = XMS_acsDllInit();	
	
	if( r< 0 )
	{
		sprintf(MsgStr,"XMS_acsDllInit failed! ret = %d\n",r);
		AddMsg(MsgStr);
	}
	else
	{
		sprintf(MsgStr,"XMS_acsDllInit OK!\n");
		AddMsg(MsgStr);
	}
	
	/*调用acsOpenStream 建立与ACS服务器的连接*/
	r = XMS_acsOpenStream ( &g_acsHandle, 
						&cfg_ServerID,
						g_u8UnitID,		// App Unit ID 
						32,				// sendQSize, in K Bytes
						32,				// recvQSize, in K Bytes
						cfg_s32DebugOn,	// Debug On/Off
						NULL);

	if ( r < 0 )
	{
		sprintf ( MsgStr, "XMS_acsOpenStream(IP Addr : %s, port : %d) FAIL. ret = %d", 
			cfg_ServerID.m_s8ServerIp, cfg_ServerID.m_u32ServerPort, r );

		//MessageBox(NULL, MsgStr, "Init System", MB_OK ) ;
		AddMsg ( MsgStr );
		return false;
	}
	else
	{
		sprintf ( MsgStr, "XMS_acsOpenStream(%s,%d) OK!", cfg_ServerID.m_s8ServerIp, cfg_ServerID.m_u32ServerPort );
		AddMsg ( MsgStr );
	}
        printf(MsgStr);
        printf("\n");
	r = XMS_acsSetESR ( g_acsHandle, (EsrFunc)EvtHandler, 0, 1 );
	if ( r < 0 )
	{
		sprintf ( MsgStr, "XMS_acsSetESR() FAIL! ret = %d", r );
		AddMsg ( MsgStr );
		return false;
	}
	else
	{
		sprintf ( MsgStr, "XMS_acsSetESR() OK!" );
		AddMsg ( MsgStr );
	}
        printf(MsgStr);
        printf("\n");

	InitAllDeviceRes ();

//	r = XMS_acsSetTimer (g_acsHandle, 3000);
//  	printf ("XMS_acsSetTimer r = %d\n",r);
  
	r = XMS_acsGetDeviceList ( g_acsHandle, NULL );
	if ( r < 0 )
	{
		sprintf ( MsgStr, "XMS_acsGetDeviceList() FAIL! ret = %d", r );
		AddMsg ( MsgStr );
		return false;
	}
	else
	{
		sprintf ( MsgStr, "XMS_acsGetDeviceList() OK!" );
		AddMsg ( MsgStr );
	}
	

	return true;
}

void	ExitSystem() 
{
	RetCode_t	r;
	int			i;

	// close all device
	for ( i = 0; i < g_iTotalModule; i ++ )
	{
		CloseAllDevice_Dsp ( MapTable_Module[i] );
	}

	r = XMS_acsCloseStream ( g_acsHandle, NULL );

	FreeAllDeviceRes ();

}

// --------------------------------------------------------------------------------
void	OpenTrunkDevice ( TRUNK_STRUCT *pOneTrunk )
{
	RetCode_t	r;
	
	if ( pOneTrunk->State == TRK_WAITOPEN )		// 还没有Open
	{
		r = XMS_ctsOpenDevice ( g_acsHandle, &pOneTrunk->deviceID, NULL );
		if ( r < 0 )
		{
			AddMsg ( "XMS_ctsOpenDevice Fail in OpenTrunkDevice()!" );
		}
	}
}

void	OpenVoiceDevice ( VOICE_STRUCT *pOneVoice )
{
	RetCode_t	r;

	if ( pOneVoice->State == VOC_WAITOPEN )		// 还没有Open
	{
		r = XMS_ctsOpenDevice ( g_acsHandle, &pOneVoice->deviceID, NULL );
		if ( r < 0 )
		{
			AddMsg ( "XMS_ctsOpenDevice Fail in OpenVoiceDevice()!" );
		}
	}
}

void	OpenPcmDevice ( PCM_STRUCT *pOnePcm )
{
	RetCode_t	r;

	if ( pOnePcm->bOpenFlag == false )		// 还没有Open
	{
		r = XMS_ctsOpenDevice ( g_acsHandle, &pOnePcm->deviceID, NULL );
		if ( r < 0 )
		{
			AddMsg ( "XMS_ctsOpenDevice Fail in OpenPcmDevice()!" );
		}
	}
}

void	OpenBoardDevice (  DJ_S8 s8DspModID )
{
	RetCode_t	r;

	if ( AllDeviceRes[s8DspModID].bOpenFlag == false )	// 还没有Open
	{
		r = XMS_ctsOpenDevice ( g_acsHandle, &AllDeviceRes[s8DspModID].deviceID, NULL );

		if ( r < 0 )
		{
			AddMsg ( "XMS_ctsOpenDevice Fail in OpenBoardDevice()!" );
		}
	}
}

void	OpenAllDevice_Dsp ( DJ_S8 s8DspModID )
{
	int			 i;

	AllDeviceRes[s8DspModID].bErrFlag = false;
	AllDeviceRes[s8DspModID].RemoveState = DSP_REMOVE_STATE_NONE;
	//DrawCount_RemoveState (s8DspModID);

	// Open Board
	OpenBoardDevice ( s8DspModID );

	// pVoice
	for ( i = 0; i < AllDeviceRes[s8DspModID].lVocNum; i++ )
	{
		OpenVoiceDevice ( &AllDeviceRes[s8DspModID].pVoice[i] );
	}

	// pPcm
	for ( i = 0; i < AllDeviceRes[s8DspModID].lPcmNum; i++ )
	{
		OpenPcmDevice ( &AllDeviceRes[s8DspModID].pPcm[i] );
	}

	// pTrunk
	for ( i = 0; i < AllDeviceRes[s8DspModID].lTrunkNum; i++ )
	{
		OpenTrunkDevice ( &AllDeviceRes[s8DspModID].pTrunk[i] );
	}
}

void	ReOpen_AllDevice (void)
{
	int		i;

	for ( i = 0; i < g_iTotalModule; i ++ )
	{
		OpenAllDevice_Dsp ( MapTable_Module[i] );
	}
}

void	OpenDeviceOK ( DeviceID_t *pDevice )
{
	TRUNK_STRUCT	*pOneTrunk;

	if ( pDevice->m_s16DeviceMain == XMS_DEVMAIN_BOARD )
	{
		AllDeviceRes[pDevice->m_s8ModuleID].deviceID.m_CallID = pDevice->m_CallID;		// this line is very important, must before all operation
		AllDeviceRes[pDevice->m_s8ModuleID].bOpenFlag = true;
	}

	if ( pDevice->m_s16DeviceMain == XMS_DEVMAIN_INTERFACE_CH )
	{
		pOneTrunk = &M_OneTrunk(*pDevice);

		pOneTrunk->deviceID.m_CallID = pDevice->m_CallID;		// this line is very important, must before all operation

		// init this Device: Trunk
		InitTrunkChannel ( pOneTrunk );

		XMS_ctsResetDevice ( g_acsHandle, pDevice, NULL );
		XMS_ctsGetDevState ( g_acsHandle, pDevice, NULL );

		// modify the count
		g_iTotalTrunkOpened ++;
		
		AllDeviceRes[pDevice->m_s8ModuleID].lTrunkOpened ++;

		if ( pOneTrunk->deviceID.m_s16DeviceSub == XMS_DEVSUB_ANALOG_TRUNK )
		{

			// Set AnalogTrunk
			void* p = NULL;
			DeviceID_t FreeVocDeviceID;
			CmdParamData_AnalogTrunk_t cmdAnalogTrunk;
			DJ_U16 u16ParamType = ANALOGTRUNK_PARAM_UNIPARAM ;
			DJ_U16 u16ParamSize = sizeof(CmdParamData_AnalogTrunk_t);

			memset(&cmdAnalogTrunk,0,sizeof(cmdAnalogTrunk));
			cmdAnalogTrunk.m_u16CallInRingCount = 1;
			cmdAnalogTrunk.m_u16CallInRingTimeOut = 6000;
                        //cmdAnalogTrunk.m_u16Rfu1 = ;
			//cmdAnalogTrunk.m_u8DialtoneDetect = XMS_ANALOG_TRUNK_DIALTONE_DETECT;
			//cmdAnalogTrunk.m_u8CallOutMethod = XMS_ANALOG_TRUNK_CALLOUT_NORMAL_PROGRESS; //XMS_ANALOG_TRUNK_CALLOUT_POLARITY_REVERSE;
			//cmdAnalogTrunk.m_u16Rfu2 = ;
			
			p = (void*)&cmdAnalogTrunk;

			RetCode_t r ;
		//	if ( SearchOneFreeVoice ( pOneTrunk,  &FreeVocDeviceID ) >= 0 )
		//	{
		//		cmdAnalogTrunk.m_VocDevID = FreeVocDeviceID;

		//		//指定绑定关系
		//		pOneTrunk->VocDevID = FreeVocDeviceID;

		//		M_OneVoice(FreeVocDeviceID).UsedDevID = pOneTrunk->deviceID; 

		//		DrawMain_VocInfo ( pOneTrunk );

				r = XMS_ctsSetParam( g_acsHandle, & pOneTrunk->deviceID, u16ParamType, u16ParamSize, (DJ_Void *)p );
				char szbuffer[1024];
				memset(szbuffer,0,sizeof(szbuffer));
				sprintf(szbuffer,"Set AnalogTrunk  ret = %d\n",r);
				AddMsg ( szbuffer );
		//	}
		//	PrepareForCallerID(pOneTrunk);
			}
	}

	if ( pDevice->m_s16DeviceMain == XMS_DEVMAIN_VOICE )
	{
		M_OneVoice(*pDevice).deviceID.m_CallID = pDevice->m_CallID;		// this is very important

		// init this Device: Voice
		Change_Voc_State ( &M_OneVoice(*pDevice), VOC_FREE);

		XMS_ctsResetDevice ( g_acsHandle, pDevice, NULL );
		XMS_ctsGetDevState ( g_acsHandle, pDevice, NULL );

		// Build Index
		/*if ( g_iTotalVoiceOpened == 0 )
		{
			My_BuildIndex ( pDevice );
		}*/

		// modify the count
		g_iTotalVoiceOpened ++;
		g_iTotalVoiceFree ++;
		AllDeviceRes[pDevice->m_s8ModuleID].lVocOpened ++;
		AllDeviceRes[pDevice->m_s8ModuleID].lVocFreeNum ++;

	}

	if ( pDevice->m_s16DeviceMain == XMS_DEVMAIN_DIGITAL_PORT )
	{
		M_OnePcm(*pDevice).deviceID.m_CallID = pDevice->m_CallID;		// this is very important
		M_OnePcm(*pDevice).bOpenFlag = true;

		// init the Device: Pcm
		XMS_ctsResetDevice ( g_acsHandle, pDevice, NULL );
		XMS_ctsGetDevState ( g_acsHandle, pDevice, NULL );

		// moidfy the count
		g_iTotalPcmOpened ++;
		AllDeviceRes[pDevice->m_s8ModuleID].lPcmOpened ++;
	}
}

// --------------------------------------------------------------------------------
void	CloseDeviceOK ( DeviceID_t *pDevice )
{
	TRUNK_STRUCT	*pOneTrunk;

	AllDeviceRes[pDevice->m_s8ModuleID].bErrFlag = true;

	if ( pDevice->m_s16DeviceMain == XMS_DEVMAIN_BOARD )
	{
		AllDeviceRes[pDevice->m_s8ModuleID].bOpenFlag = false;
	}

	if ( pDevice->m_s16DeviceMain == XMS_DEVMAIN_INTERFACE_CH )
	{
		pOneTrunk = &M_OneTrunk(*pDevice);

		Change_State ( pOneTrunk, TRK_WAITOPEN );

		// modify the count
		g_iTotalTrunkOpened --;
		
		AllDeviceRes[pDevice->m_s8ModuleID].lTrunkOpened --;
	}

	if ( pDevice->m_s16DeviceMain == XMS_DEVMAIN_VOICE )
	{
		// init this Device: Voice
		Change_Voc_State ( &M_OneVoice(*pDevice), VOC_WAITOPEN);

		// modify the count
		g_iTotalVoiceOpened --;
		g_iTotalVoiceFree --;
		AllDeviceRes[pDevice->m_s8ModuleID].lVocOpened --;
		AllDeviceRes[pDevice->m_s8ModuleID].lVocFreeNum --;

	}

	if ( pDevice->m_s16DeviceMain == XMS_DEVMAIN_DIGITAL_PORT )
	{
		M_OnePcm(*pDevice).bOpenFlag = false;

		// moidfy the count
		g_iTotalPcmOpened --;
		AllDeviceRes[pDevice->m_s8ModuleID].lPcmOpened --;
	}
}

// --------------------------------------------------------------------------------
void	CloseTrunkDevice ( TRUNK_STRUCT *pOneTrunk )
{
	RetCode_t	r;
	
	pOneTrunk->State= TRK_WAITOPEN;

	r = XMS_ctsCloseDevice ( g_acsHandle, &pOneTrunk->deviceID, NULL );
	if ( r < 0 )
	{
		AddMsg ( "XMS_ctsCloseDevice Fail in CloseTrunkDevice()!" );
	}
}

void	CloseVoiceDevice ( VOICE_STRUCT *pOneVoice )
{
	RetCode_t	r;
	
	r = XMS_ctsCloseDevice ( g_acsHandle, &pOneVoice->deviceID, NULL );
	if ( r < 0 )
	{
		AddMsg ( "XMS_ctsCloseDevice Fail in CloseVoiceDevice()!" );
	}
}

void	ClosePcmDevice ( PCM_STRUCT *pOnePcm )
{
	RetCode_t	r;
	
	r = XMS_ctsCloseDevice ( g_acsHandle, &pOnePcm->deviceID, NULL );
	if ( r < 0 )
	{
		AddMsg ( "XMS_ctsCloseDevice Fail in ClosePcmDevice()!" );
	}
}

void	CloseBoardDevice ( DeviceID_t *pBoardDevID )
{
	RetCode_t	r;

	r = XMS_ctsCloseDevice ( g_acsHandle, pBoardDevID, NULL );

	if ( r < 0 )
	{
		AddMsg ( "XMS_ctsCloseDevice Fail in CloseBoardDevice()!" );
	}
}

void	CloseAllDevice_Dsp ( DJ_S8 s8DspModID )
{
	int			 i;

	// close Board
	CloseBoardDevice ( &AllDeviceRes[s8DspModID].deviceID );

	// pTrunk
	for ( i = 0; i < AllDeviceRes[s8DspModID].lTrunkNum; i++ )
	{
		CloseTrunkDevice ( &AllDeviceRes[s8DspModID].pTrunk[i] );
	}

	// pVoice
	for ( i = 0; i < AllDeviceRes[s8DspModID].lVocNum; i++ )
	{
		CloseVoiceDevice ( &AllDeviceRes[s8DspModID].pVoice[i] );
	}

	// pPcm
	for ( i = 0; i < AllDeviceRes[s8DspModID].lPcmNum; i++ )
	{
		ClosePcmDevice ( &AllDeviceRes[s8DspModID].pPcm[i] );
	}
}

// -------------------------------------------------------------------------------------------------
void	HandleDevState ( Acs_Evt_t *pAcsEvt )
{
	TRUNK_STRUCT	*pOneTrunk;
	PCM_STRUCT		*pOnePcm;
	Acs_GeneralProc_Data *pGeneralData = NULL;

	pGeneralData = (Acs_GeneralProc_Data *)FetchEventData(pAcsEvt);
	if ( pAcsEvt->m_DeviceID.m_s16DeviceMain == XMS_DEVMAIN_INTERFACE_CH )
	{
		pOneTrunk = &M_OneTrunk(pAcsEvt->m_DeviceID);

		pOneTrunk->iLineState = pGeneralData->m_s32DeviceState;
		DrawMain_LineState( pOneTrunk );
	}

	if ( pAcsEvt->m_DeviceID.m_s16DeviceMain == XMS_DEVMAIN_DIGITAL_PORT )
	{
		pOnePcm = &M_OnePcm(pAcsEvt->m_DeviceID);

		pOnePcm->u8E1Type = (pGeneralData->m_s32DeviceState >> 16) & 0xFF;
		pOnePcm->s32AlarmVal = (pGeneralData->m_s32DeviceState & 0xFFFF);

	}

}

// -------------------------------------------------------------------------------------------------
void	CheckRemoveReady ( DJ_S8 s8DspModID )
{
	int			i;

	// check device : INTERFACE_CH
	for ( i = 0; i < AllDeviceRes[s8DspModID].lTrunkNum; i ++ )
	{
		if ( (AllDeviceRes[s8DspModID].pTrunk[i].State != TRK_WAITOPEN)
			&& (AllDeviceRes[s8DspModID].pTrunk[i].State != TRK_WAIT_REMOVE) )
		{
			return;
		}
	}

	// check device : VOICE
	for ( i = 0; i < AllDeviceRes[s8DspModID].lVocNum; i ++ )
	{
		if ( (AllDeviceRes[s8DspModID].pVoice[i].State != VOC_WAITOPEN)
			&& (AllDeviceRes[s8DspModID].pVoice[i].State != VOC_WAIT_REMOVE) )
		{
			return;
		}
	}

	// all device in this DSP is ready for remove 
	AllDeviceRes[s8DspModID].RemoveState = DSP_REMOVE_STATE_READY;
	//DrawCount_RemoveState ( s8DspModID );
}



void My_InitDtmfBuf ( TRUNK_STRUCT *pOneTrunk )
{
	pOneTrunk->DtmfCount = 0;
	pOneTrunk->DtmfBuf[0] = 0; 
}

void My_AddDtmfBuf ( TRUNK_STRUCT *pOneTrunk, char TmpDtmf )
{
	if ( pOneTrunk->DtmfCount < (32-1) )
	{
		pOneTrunk->DtmfBuf[pOneTrunk->DtmfCount] = TmpDtmf; 
		pOneTrunk->DtmfBuf[pOneTrunk->DtmfCount+1] = 0; 
		pOneTrunk->DtmfCount ++;
	}
}

int My_GetDtmfCode ( Acs_Evt_t *pAcsEvt ,char *&data)
{
	Acs_IO_Data				*pIOData = NULL;
	if ( pAcsEvt->m_s32EventType == XMS_EVT_RECVIODATA )	/*IO Data事件*/
	{
		pIOData = (Acs_IO_Data *)FetchEventData(pAcsEvt);

		if ( ( pIOData->m_u16IoType== XMS_IO_TYPE_DTMF ) //XMS_IO_TYPE_GTG )
			&& ( pIOData->m_u16IoDataLen > 0 ) )
		{
			data = (char *)FetchIOData(pAcsEvt);
			AddMsg(strcat(data,"\0"));

			return pIOData->m_u16IoDataLen;
		}
		/*else if( ( pIOData->m_u16IoType== XMS_IO_TYPE_FSK ) 
			&& ( pIOData->m_u16IoDataLen > 0 ) )
		{
			data = (char *)FetchIOData(pAcsEvt);
			AddMsg(strcat(data,"\0"));

			return pIOData->m_u16IoDataLen;
		}*/
		else if( ( pIOData->m_u16IoType== XMS_IO_TYPE_GTG ) 
			&& ( pIOData->m_u16IoDataLen > 0 ) )
		{
			data = (char *)FetchIOData(pAcsEvt);
			AddMsg(strcat(data,"\0"));

			return pIOData->m_u16IoDataLen;
		}
	}

	return -1;	// not a good DTMF
}

int My_GetFskCode ( Acs_Evt_t *pAcsEvt ,char *&data)
{
	Acs_IO_Data				*pIOData = NULL;	

	if ( pAcsEvt->m_s32EventType == XMS_EVT_RECVIODATA )	/*IO Data事件*/
	{
		pIOData = (Acs_IO_Data *)FetchEventData(pAcsEvt);

		if ( ( pIOData->m_u16IoType== XMS_IO_TYPE_FSK ) //XMS_IO_TYPE_GTG )
			&& ( pIOData->m_u16IoDataLen > 0 ) )
		{
			data = (char *)FetchIOData(pAcsEvt);

			return pIOData->m_u16IoDataLen;
		}
	}

	return -1;	// not a good FSK
}

void	My_DualLink ( DeviceID_t *pDev1, DeviceID_t *pDev2 )
{
	XMS_ctsLinkDevice ( g_acsHandle, pDev1, pDev2, NULL ); 
	XMS_ctsLinkDevice ( g_acsHandle, pDev2, pDev1, NULL ); 
}

void	My_DualUnlink ( DeviceID_t *pDev1, DeviceID_t *pDev2 )
{
	XMS_ctsUnlinkDevice ( g_acsHandle, pDev1, pDev2, NULL ); 
	XMS_ctsUnlinkDevice ( g_acsHandle, pDev2, pDev1, NULL ); 
}

int		SearchOneFreeVoice (  TRUNK_STRUCT *pOneTrunk, DeviceID_t *pFreeVocDeviceID )
{
	DJ_S8			s8ModID;
	DJ_S16			s16ChID;
	int				i;
	static	int		iLoopStart = 0;
	VOICE_STRUCT	*pOneVoice;
	DJ_S8			s8SearchModID;
	long			lNowMostFreeNum;

	s8ModID = pOneTrunk->deviceID.m_s8ModuleID;

	if ( cfg_iVoiceRule == 0 )			// Fix relationship between Trunk & Voice
	{
		s16ChID = pOneTrunk->deviceID.m_s16ChannelID;
		
		i = pOneTrunk->iModSeqID;

		if ( i < AllDeviceRes[s8ModID].lVocNum )
		{
			pOneVoice = &AllDeviceRes[s8ModID].pVoice[i];
			if ( pOneVoice->State != VOC_WAITOPEN )
			{
				*pFreeVocDeviceID = pOneVoice->deviceID;

				// 占用该voice device 
				Change_Voc_State ( pOneVoice, VOC_USED);
				AllDeviceRes[s8ModID].lVocFreeNum--;
				g_iTotalVoiceFree --;
				return i;
			}
		}

		return -1;
	}

	if ( cfg_iVoiceRule == 1 )			// Search in Same Module
	{
		s8SearchModID = s8ModID;
	}
	else		// Search in Most free resource module
	{
		s8SearchModID = -1;
		lNowMostFreeNum = -1;
		for ( i = 0; i < g_iTotalModule; i ++ )
		{
			if ( AllDeviceRes[MapTable_Module[i]].lVocFreeNum > lNowMostFreeNum )
			{
				s8SearchModID = MapTable_Module[i];
				lNowMostFreeNum = AllDeviceRes[MapTable_Module[i]].lVocFreeNum;
			}
		}
	}

	for ( i = 0; i < AllDeviceRes[s8SearchModID].lVocNum; i ++ )
	{
		pOneVoice = &AllDeviceRes[s8SearchModID].pVoice[i];
		if ( pOneVoice->State == VOC_FREE )
		{
			*pFreeVocDeviceID = pOneVoice->deviceID;

			// 占用该voice device 
			Change_Voc_State ( pOneVoice, VOC_USED);
			AllDeviceRes[s8SearchModID].lVocFreeNum--;
			g_iTotalVoiceFree --;
			return i;
		}
	}

	return -1;
}

int		FreeOneFreeVoice (  DeviceID_t *pFreeVocDeviceID )
{
	DJ_S8	s8ModID;

	s8ModID = pFreeVocDeviceID->m_s8ModuleID;
	if ( AllDeviceRes[s8ModID].lFlag == 1 )
	{
		Change_Voc_State ( &M_OneVoice(*pFreeVocDeviceID), VOC_FREE);

		AllDeviceRes[s8ModID].lVocFreeNum++;
		g_iTotalVoiceFree ++;
		return	0;		// OK
	}

	return -1;			// invalid VocDeviceID
}

void	Change_Voc_State ( VOICE_STRUCT *pOneVoice, VOICE_STATE NewState )
{
	pOneVoice->State = NewState;

	// 检查是否准备开始删除DSP单元
	if ( (AllDeviceRes[pOneVoice->deviceID.m_s8ModuleID].RemoveState == DSP_REMOVE_STATE_START)
		&& (NewState == VOC_FREE) )
	{
		pOneVoice->State = VOC_WAIT_REMOVE;

		CheckRemoveReady ( pOneVoice->deviceID.m_s8ModuleID );
	}
}

void	Change_State ( TRUNK_STRUCT *pOneTrunk, TRUNK_STATE NewState )
{
	pOneTrunk->State = NewState;

	DrawMain_State ( pOneTrunk );

	// 检查是否准备开始删除DSP单元
	if ( (AllDeviceRes[pOneTrunk->deviceID.m_s8ModuleID].RemoveState == DSP_REMOVE_STATE_START)
		&& (NewState == TRK_FREE) )
	{
		pOneTrunk->State = TRK_WAIT_REMOVE;

		DrawMain_State ( pOneTrunk );

		CheckRemoveReady ( pOneTrunk->deviceID.m_s8ModuleID );
	}
}

void	InitTrunkChannel ( TRUNK_STRUCT *pOneTrunk )
{
	Change_State ( pOneTrunk, TRK_FREE );

	pOneTrunk->CallerCode[0]=0;
	pOneTrunk->CalleeCode[0]=0;
	pOneTrunk->loop = 0;
	pOneTrunk->syncSeqNo = SYNCSEQNO_LINK;
	pOneTrunk->msg = (ctsimsg *) malloc (sizeof(ctsimsg)); 
	pOneTrunk->msg->sd = INVALID_SOCKET;
	pOneTrunk->msg->flag = 0;
	memset(pOneTrunk->szRandom,0,sizeof(pOneTrunk->szRandom));
	memset(pOneTrunk->FskBuf,0,sizeof(pOneTrunk->FskBuf));
	memset(pOneTrunk->szRecvbyFSKBuf,0,sizeof(pOneTrunk->szRecvbyFSKBuf));
	memset(pOneTrunk->szSendbyFSKBuf,0,sizeof(pOneTrunk->szSendbyFSKBuf));

	My_InitDtmfBuf ( pOneTrunk );
	DrawMain_DTMF ( pOneTrunk );

	memset ( &pOneTrunk->VocDevID, 0, sizeof(DeviceID_t) );		// -1表示还没有分配Voc Device
	DrawMain_VocInfo ( pOneTrunk );
}

void ResetTrunk ( TRUNK_STRUCT *pOneTrunk, Acs_Evt_t *pAcsEvt )
{
	/*StopPlayFile ( &pOneTrunk->VocDevID );*/

	// free the used Voice Resource
	if ( pOneTrunk->VocDevID.m_s16DeviceMain != 0 )
	{
		My_DualUnlink ( &pOneTrunk->VocDevID, &pOneTrunk->deviceID );

		FreeOneFreeVoice (  &pOneTrunk->VocDevID );

		memset ( &M_OneVoice(pOneTrunk->VocDevID).UsedDevID,	0, sizeof(DeviceID_t) );		// 0表示还没有分配 Device 
		memset ( &pOneTrunk->VocDevID, 0, sizeof(DeviceID_t) );		// 0表示还没有分配 Device
		DrawMain_VocInfo ( pOneTrunk );
	}

	InitTrunkChannel ( pOneTrunk );
}


void SetGtD(DeviceID_t* pDevId)
{
	//========Set GTG Begin========
				
				CmdParamData_Voice_t cmdVoc;
				memset(&cmdVoc,0,sizeof(cmdVoc));
				cmdVoc.m_u8GtdCtrlValid = 1 ; //Enable GTD
				cmdVoc.m_VocGtdControl.m_u8ChannelEnable = 1;//Enable Gtd channel
				cmdVoc.m_VocGtdControl.m_u8DTMFEnable = 1;
				cmdVoc.m_VocGtdControl.m_u8MR2FEnable = 1;
				cmdVoc.m_VocGtdControl.m_u8MR2BEnable = 1;
				cmdVoc.m_VocGtdControl.m_u8GTDEnable = 1;
				cmdVoc.m_VocGtdControl.m_u8FSKEnable = 1;
				strcpy((char*)&cmdVoc.m_VocGtdControl.m_u8GTDID[0],"GHIJK");

				DJ_U16 u16ParamType = VOC_PARAM_UNIPARAM;
				DJ_U16 u16ParamSize = sizeof(cmdVoc);
				void* p = (void*) &cmdVoc;

				int r = XMS_ctsSetParam( g_acsHandle,pDevId,u16ParamType,u16ParamSize,(void*)p);
				char szbuffer[1024];
				memset(szbuffer,0,sizeof(szbuffer));
				sprintf(szbuffer,"Set GTD ret = %d\n",r);
				AddMsg ( szbuffer );

				//========Set GTG End  ========
}

void ClearCall(TRUNK_STRUCT *pOneTrunk)
{
	wrLog("通道信息[%d]：[挂机]",pOneTrunk->deviceID.m_s16ChannelID);
	XMS_ctsClearCall ( g_acsHandle, &pOneTrunk->deviceID, 0, NULL );	
}

DJ_S32	SendFSK ( TRUNK_STRUCT *pOneTrunk, DJ_U16 data_size)
{	
	/*BYTE pbyBuf[LEN_1024];
	memset(pbyBuf, 0, sizeof(pbyBuf));
	if(pOneTrunk->isEpos)
	{
		memset(pbyBuf, 0x55, 8);
		pbyBuf[8] = 0xFF;
		memcpy(&pbyBuf[9], pOneTrunk->szSendbyFSKBuf, data_size);
		data_size += 9;
	}
	else
	{
		memcpy(pbyBuf, pOneTrunk->szSendbyFSKBuf, data_size);
	}
	memcpy(pbyBuf, pOneTrunk->szSendbyFSKBuf, data_size);*/
	
	if (XMS_ctsSendIOData ( g_acsHandle, &pOneTrunk->VocDevID, XMS_IO_TYPE_FSK, data_size, pOneTrunk->szSendbyFSKBuf))
	{	
		return 1;
	}
	else
	{
		wrLog("Send Fail");
		pOneTrunk->loop++;
		return 0;
	}
}

void TrunkWork ( TRUNK_STRUCT *pOneTrunk, Acs_Evt_t *pAcsEvt )
{
	Acs_CallControl_Data *	pCallControl = NULL;
	DeviceID_t				FreeVocDeviceID;
	char					FileName[256];
	char					TmpDtmf;


	if ( pAcsEvt->m_s32EventType == XMS_EVT_CLEARCALL )	/*拆线事件*/
	{
		wrLog("通道信息[%d]：[拆线]",pOneTrunk->deviceID.m_s16ChannelID);
		if ( (pOneTrunk->State != TRK_FREE) && (pOneTrunk->State != TRK_WAIT_REMOVE) )
		{
			ResetTrunk ( pOneTrunk, pAcsEvt );
			return ; 
		}
	}

	if ( pAcsEvt->m_s32EventType == XMS_EVT_RECVIODATA )	/*忙音事件*/
	{
		Acs_IO_Data				*pIOData = NULL;
		char		*p;
				pIOData = (Acs_IO_Data *)FetchEventData(pAcsEvt);

		if ( ( pIOData->m_u16IoType == XMS_IO_TYPE_GTG ) //
			&& ( pIOData->m_u16IoDataLen > 0 ) )
		{
			p = (char *)FetchIOData(pAcsEvt);

		 if ( *p == 'I' || *p == 'J' || *p == 'K' )//忙音
			{
				wrLog("通道信息[%d]：[忙音]",pOneTrunk->deviceID.m_s16ChannelID);
				XMS_ctsResetDevice(g_acsHandle,&pOneTrunk->deviceID,NULL);
				XMS_ctsClearCall(g_acsHandle,&pOneTrunk->deviceID,0,NULL);
				ResetTrunk(pOneTrunk,pAcsEvt);
			}

		}

	}

	switch(pOneTrunk->State)
	{
	case TRK_FREE:  

		/*振铃*/
		if ( pAcsEvt->m_s32EventType == XMS_EVT_CALLIN )	/*呼入事件，开始振铃*/
		{
			pCallControl = (Acs_CallControl_Data *)FetchEventData(pAcsEvt);

			XMS_ctsAlertCall ( g_acsHandle, &pOneTrunk->deviceID, NULL );

			XMS_ctsAnswerCallIn ( g_acsHandle, &pOneTrunk->deviceID, NULL );

			sprintf ( pOneTrunk->CalleeCode, pCallControl->m_s8CalledNum );
			sprintf ( pOneTrunk->CallerCode, pCallControl->m_s8CallingNum );
			DrawMain_CallInfo( pOneTrunk );

			Change_State ( pOneTrunk, TRK_WAIT_ANSWERCALL );//TRK_WAIT_ALERTCALL
		}
		break;

	case TRK_WAIT_ANSWERCALL:

		if ( pAcsEvt->m_s32EventType == XMS_EVT_ANSWERCALL )	/*应答呼入完成事件*/
		{
			pCallControl = (Acs_CallControl_Data *)FetchEventData(pAcsEvt);

			if ( SearchOneFreeVoice ( pOneTrunk,  &FreeVocDeviceID ) >= 0 )
			{				

				pOneTrunk->VocDevID = FreeVocDeviceID;

				M_OneVoice(FreeVocDeviceID).UsedDevID = pOneTrunk->deviceID; 

				DrawMain_VocInfo ( pOneTrunk );

				My_DualLink ( &FreeVocDeviceID, &pOneTrunk->deviceID ); 

				SetGtD(&pOneTrunk->VocDevID);


				Change_State ( pOneTrunk, TRK_WAIT_LINKOK );

				wrLog("通道信息[%d]：[振铃成功，连接语音设备]",pOneTrunk->deviceID.m_s16ChannelID);
			}
			else
			{
				ResetTrunk(pOneTrunk,pAcsEvt);
				Change_State ( pOneTrunk, TRK_FREE);
				wrLog("通道信息[%d]：[无空闲语音资源]",pOneTrunk->deviceID.m_s16ChannelID);
			}
		}
		break;

	case TRK_WAIT_LINKOK:
		if ( pAcsEvt->m_s32EventType == XMS_EVT_LINKDEVICE )	/*LinkDevice完成*/
		{
			Change_State ( pOneTrunk, TRK_SENDLINK );
		}
		break;	

	case TRK_SENDLINK:
		if ( pAcsEvt->m_s32EventType == XMS_EVT_SETPARAM )	/*设置GTD参数事件*/
		{
			Send_LinkPack(pOneTrunk);
		}

		break;

	case TRK_LINK_REQUEST:
		if ( pAcsEvt->m_s32EventType == XMS_EVT_SENDIODATA )
		{
			Change_State ( pOneTrunk, TRK_RECVLINKACK );
		}
		break;

	case TRK_RECVLINKACK:
		if ( pAcsEvt->m_s32EventType == XMS_EVT_RECVIODATA )
		{
			wrLog("通道信息[%d]：[收到FSK链接响应报文]",pOneTrunk->deviceID.m_s16ChannelID);
			char * buf;
			int len = My_GetFskCode(pAcsEvt,buf);			
			if (len>0)
			{
				memset(pOneTrunk->szRecvbyFSKBuf,0,LEN_1024);
				memcpy(pOneTrunk->szRecvbyFSKBuf,buf,len);
				RecvLinkACK(pOneTrunk,len);
			}			
		}
		break;

	case TRK_SENDFSK:
		if ( pAcsEvt->m_s32EventType == XMS_EVT_SENDIODATA )
		{	
			Change_State ( pOneTrunk, TRK_RECVFSKACK );
		}
		break;

	case TRK_RECVFSKACK:
		if ( pAcsEvt->m_s32EventType == XMS_EVT_RECVIODATA )
		{
			char * buf;
			int len = My_GetFskCode(pAcsEvt,buf);
			if (len>0)
			{
				memset(pOneTrunk->szRecvbyFSKBuf,0,LEN_1024);
				memcpy(pOneTrunk->szRecvbyFSKBuf,buf,len);
				RecvFSKACK(pOneTrunk,len);
			}
		}		
		break;

	case TRK_RECVFSK:
		if ( pAcsEvt->m_s32EventType == XMS_EVT_RECVIODATA )
		{
			Acs_IO_Data				*pIOData = NULL;
			int ret = 0,iLen,r;

			pIOData = (Acs_IO_Data *)FetchEventData(pAcsEvt);

			if ( ( pIOData->m_u16IoType == XMS_IO_TYPE_FSK )    /*FSK 事件*/
				&& ( pIOData->m_u16IoDataLen > 0 ) )
			{
				char * buf;
				memset(pOneTrunk->szRecvbyFSKBuf,0,LEN_1024);
				buf = (char *)FetchIOData(pAcsEvt);
				memcpy(pOneTrunk->szRecvbyFSKBuf,buf,pIOData->m_u16IoDataLen);
				int rret = RecvFSK(pOneTrunk,pIOData->m_u16IoDataLen);
				if(rret == 0)
				{
					Send_AckPack(pOneTrunk);
				}
			}
		}
		break;

	case TRK_SENDFSKACK:
		if ( pAcsEvt->m_s32EventType == XMS_EVT_SENDIODATA )
		{
			pOneTrunk->loop = 0;
			memset(pOneTrunk->szSendbyFSKBuf,0,1024);
			
			Construct_CTSIPack(pOneTrunk->szSendbyFSKBuf,pOneTrunk->packLength,pOneTrunk->msg,pOneTrunk->szRecvbyFSKBuf);

			Change_State ( pOneTrunk, TRK_SENDCTSI );

			/*HANDLE hHandle= CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)CTSISendRecvData, pOneTrunk->msg, 0, NULL);
			CloseHandle(hHandle);*/
			/*int res;
			pthread_t a_thread;
			res = pthread_create(&a_thread, NULL, CTSISendRecvData, (void *)pOneTrunk->msg);			
			res = pthread_join(a_thread, NULL);*/

			CTSISendRecvData((void *)pOneTrunk->msg);
			Change_State ( pOneTrunk, TRK_RECVCTSI );

			//sleep(1);//延迟FOR RecvCTSI
			RecvCTSI(pOneTrunk);
		}
	}
}

bool	IsTrunk ( DJ_S16 s16DevSub )
{

	if ( (s16DevSub == XMS_DEVSUB_E1_PCM)
		|| (s16DevSub == XMS_DEVSUB_E1_CAS)
		|| (s16DevSub == XMS_DEVSUB_E1_DSS1)
		|| (s16DevSub == XMS_DEVSUB_E1_SS7_TUP)
		|| (s16DevSub == XMS_DEVSUB_E1_SS7_ISUP) 
		|| (s16DevSub == XMS_DEVSUB_ANALOG_TRUNK) 
		)
	{
		return true;
	}

	return false;
}

bool	IsDigitTrunk ( DJ_S16 s16DevSub )
{

	if ( (s16DevSub == XMS_DEVSUB_E1_PCM)
		|| (s16DevSub == XMS_DEVSUB_E1_CAS)
		|| (s16DevSub == XMS_DEVSUB_E1_DSS1)
		|| (s16DevSub == XMS_DEVSUB_E1_SS7_TUP)
		|| (s16DevSub == XMS_DEVSUB_E1_SS7_ISUP) 
		)
	{
		return true;
	}

	return false;
}

int	 SearchOneFreeTrunk ( DeviceID_t *pFreeTrkDeviceID )
{
	int		i, iTmp;
	DJ_S8	s8ModID;
	DJ_S16	s16ChID;
	static	int	iLoopStart = 0;
	
	for ( i = 0; i < g_iTotalTrunk; i ++ )
	{
		s8ModID = MapTable_Trunk[iLoopStart].m_s8ModuleID;
		s16ChID = MapTable_Trunk[iLoopStart].m_s16ChannelID;

		if ( ( AllDeviceRes[s8ModID].pTrunk[s16ChID].iLineState == DCS_FREE )
			&& IsTrunk(AllDeviceRes[s8ModID].pTrunk[s16ChID].deviceID.m_s16DeviceSub) )
		{
			*pFreeTrkDeviceID = AllDeviceRes[s8ModID].pTrunk[s16ChID].deviceID;
			iTmp = iLoopStart;
			iLoopStart = (iLoopStart + 1 ) % g_iTotalTrunk;
			return iTmp;
		}
		iLoopStart = (iLoopStart + 1 ) % g_iTotalTrunk;
	}

	return -1;
}


int main(int argc,char** argv)
{

	printf("XMS Demo Program\n");
	
	InitSystem();	
	
	mainwork(windownum,colnum,130,0,"TISSON APMS FSK","'q' to Exit; Up,Down to Scroll Window");
	
	return 0;
}
