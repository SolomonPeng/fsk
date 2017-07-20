#ifndef _XMS_SUB_H_
#define _XMS_SUB_H_

#include "DataDefine.h"
#include "DJAcsAPIDef.h"



// ----- decalure function -----
bool	InitSystem(void);
void	ExitSystem(void); 

void	AddMsg ( char *str);

void	OpenAllDevice_Dsp ( DJ_S8 s8DspModID );
void	CloseAllDevice_Dsp ( DJ_S8 s8DspModID );
void	ReOpen_AllDevice (void);

void	HandleDevState ( Acs_Evt_t *pAcsEvt );

void	InitTrunkChannel ( TRUNK_STRUCT *pOneTrunk );
void	ReDrawAll (void);
void	OpenDeviceOK ( DeviceID_t *pDevice );
void	CloseDeviceOK ( DeviceID_t *pDevice );

void	TrunkWork ( TRUNK_STRUCT *pOneTrunk, Acs_Evt_t *pAcsEvt );

void	Change_State ( TRUNK_STRUCT *pOneTrunk, TRUNK_STATE NewState );
void	Change_Voc_State ( VOICE_STRUCT *pOneVoice, VOICE_STATE NewState );

int My_GetDtmfCode ( Acs_Evt_t *pAcsEvt ,char *&data);
int My_GetFskCode ( Acs_Evt_t *pAcsEvt,char *&data);
int  SearchOneFreeVoice (  TRUNK_STRUCT *pOneTrunk, DeviceID_t *pFreeVocDeviceID );
int		CalDispRow ( int iSeqID );
void ClearCall(TRUNK_STRUCT *pOneTrunk);
DJ_S32	SendFSK ( TRUNK_STRUCT *pOneTrunk, DJ_U16 data_size);

#endif


