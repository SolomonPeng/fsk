#ifndef __FSK_SUB_H__
#define __FSK_SUB_H__
#include "Connect.h"
#include "DataDefine.h"
#include "XMS_Sub.h"
#include "FSK_Data.h"

void wrChannel(char *str,TRUNK_STRUCT *pOneTrunk);
void ReleaseCall(TRUNK_STRUCT *pOneTrunk);
void ResetTimer(TRUNK_STRUCT *pOneTrunk);
void Send_LinkPack(TRUNK_STRUCT *pOneTrunk);
void Send_FSKPack(TRUNK_STRUCT *pOneTrunk);
void Send_AckPack(TRUNK_STRUCT *pOneTrunk);
void RecvCTSI(TRUNK_STRUCT *pOneTrunk);
void RecvFSKACK(TRUNK_STRUCT *pOneTrunk,int length);
void RecvLinkACK(TRUNK_STRUCT *pOneTrunk,int length);
int RecvFSK(TRUNK_STRUCT *pOneTrunk,int iLength);
#endif
