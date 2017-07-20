#ifndef __FSK_DATA_H__
#define __FSK_DATA_H__
#include "WrLog.h"
#include "DataDefine.h"

BYTE verify(BYTE *buff, int length);
void verifyRandom(BYTE* szRandom,BYTE *random);
void addSeqNo(BYTE &syncSeqNo);
int AckPackCheck(BYTE* szBuf,int length,BYTE syncSeqNo,BYTE* szrandom);
int LinkAckPackCheck(BYTE* szBuf,int length,BYTE* szrandom);
bool PackCheck(int length,BYTE* szBuf,BYTE* szrandom,BYTE syncSeqNo);
bool checkPackComplete(int &length,BYTE* szBuf);
void Construct_msgPack(BYTE *fsk, int length,pctsimsg msg);
int Construct_AckPack(BYTE* szBuf,BYTE* random,BYTE syncSeqNo);
int Construct_LinkPack(BYTE* szBuf,BYTE* random);
void Construct_CTSIPack(BYTE* szBuf,int length,pctsimsg msg,BYTE* szContent);
int Construct_FSKPack(BYTE* szBuf,BYTE* szRandom,BYTE syncSeqNo,BYTE* szContent,int length);
void Construct_NumberPack(BYTE* szBuf,char* CalleeCode,pctsimsg msg,BYTE* szContent);
void getRandomHex(BYTE *random);
int getPackLen(BYTE *lenBuff);
//void Unzip_PEPack(BYTE* szBuf,BYTE* szContent);

#endif
