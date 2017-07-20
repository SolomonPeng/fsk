#ifndef _XMS_STRING_H_
#define _XMS_STRING_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *GetString_EventType ( EventType_t EvtType );
char *GetString_DeviceMain ( DJ_S32	s32DeviceType );
char *GetString_DeviceSub ( DJ_S32	s32DeviceSub );
char *GetString_ErrorCode ( DJ_S32	s32ErrorCode );

void	GetString_LineState ( char *StateStr, int iLineState );
char	*GetString_PcmType ( int iPcmType );
char* GetString_TrunkState(int State);

#endif