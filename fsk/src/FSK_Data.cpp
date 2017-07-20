#include "FSK_Data.h"
#include "adapter.h"

BYTE verify(BYTE *buff, int length)
{
	int result = 0;
	for(int i=0;i<length;i++)
		result += (int)buff[i];
	result = result % 256;
	result = 256 - result;
	/*wrLog("校验位:%s",&result);
	wrLog("校验位:%02x",result);*/
	return (BYTE)result;
}

void verifyRandom(BYTE* szRandom,BYTE *random)
{
	BYTE key[4] = {0x37, 0x58, 0x57, 0x01};
	for(int i=0; i<4; i++)
		*(random+i) = (*(szRandom+i))^key[i];
}

void Construct_msgPack(BYTE *fsk, int length,pctsimsg msg)
{
	msg->flag = 0;
	//msg->sd = sd;
	msg->length = length;
	memcpy(msg->buff,fsk,length);
}

int getPackLen(BYTE *lenBuff)
{
	int packLen;
	packLen = (int)lenBuff[1];
	packLen += ((int)lenBuff[0])*256;
	return packLen;
}

void getRandomHex(BYTE *random)
{
	srand((unsigned)time(NULL));
	sprintf((char *)random,"%x",(rand()<<16)+rand());
}

void Construct_NumberPack(BYTE* szBuf,char* CalleeCode,pctsimsg msg,BYTE* szContent)
{
	//*szBuf = FSKMESSAGE;
	//*(szBuf+1) = 0x00;
	//*(szBuf+3) = 0x11;
	//int iLinkAckLen = getPackLen(szContent+1) - 5;
	//memcpy(szBuf+5,szContent+8,iLinkAckLen);
	//int length = strlen(CalleeCode);
	//*(szBuf+iLinkAckLen+5) = length;
	//*(szBuf+4) = (BYTE)(iLinkAckLen+1+length);
	//*(szBuf+2) = iLinkAckLen+3+length;
	//memcpy(szBuf+iLinkAckLen+6,CalleeCode,length);
	//Construct_msgPack(szBuf,iLinkAckLen+length+6,msg);
	char s[1024];
	strcpy(s,"003a600008801108002020040000c2000096000000134500223031303032373138303030303030303030303030303030001030323030303030303031");
	int len=strlen(s);
	memcpy(szBuf,s,len);
	int len2 = CAdapter::tobyte(s,szBuf);
	wrLog("CTSI报文:%s",szBuf);
	Construct_msgPack(szBuf,len2,msg);
}

//---------------构造发送给终端的FSK报文------------------------------
int Construct_FSKPack(BYTE* szBuf,BYTE* szRandom,BYTE syncSeqNo,BYTE* szContent,int length)
{
	int ret = 0;
	/*BYTE tmpstr[11+length];
	memset(tmpstr,0,sizeof(tmpstr));
	for (int i=0;i<8;i++)
	{
		*(szBuf+i) = SYNCDATA;
	}*/
	szBuf[0] = MTYPE_TRADE_SERVER;	
	szBuf[1] = (BYTE)((length+7)/256);
	szBuf[2] = (BYTE)((length+7)%256);
	memcpy(szBuf+3,szRandom,4);
	szBuf[7] = syncSeqNo;
	szBuf[8] = (BYTE)((length)/256);
	szBuf[9] = (BYTE)((length)%256);
	memcpy(szBuf+10,szContent,length);
	szBuf[10+length] = verify(szBuf,10+length);//校验位
	
	ret += length+10+1;
	return ret;
}

//---------------构造发送给中心的FSK报文------------------------------
void Construct_CTSIPack(BYTE* szBuf,int length,pctsimsg msg,BYTE* szContent)
{	
	int ret = length;
	/*szBuf[0] = (BYTE)((ret)/(256*256*256));
	szBuf[1] = (BYTE)((ret)/(256*256));*/
	szBuf[0] = (BYTE)((ret)/(256));
	szBuf[1] = (BYTE)((ret)%(256));
	////十位TPDU：3000000000
	//szBuf[4] = 0x30;
	//for (int i=0;i<4;++i)
	//{
	//	szBuf[5+i] = 0x00;
	//}
	//复制FSK报文实际内容
	int szlen = getPackLen(szContent+8);
	if (length < szlen)
	{
		wrLog("内容长度检验错误:recv:%d,save:%d",szlen,length);		
	}
	memcpy(szBuf+2,szContent+10,szlen);
	Construct_msgPack(szBuf,length+2,msg);
}

//---------------构造建链报文------------------------------
int Construct_LinkPack(BYTE* szBuf,BYTE *szrandom)
{
	int ret = 0;
	BYTE random[5];	
	memset(random,0,sizeof(random));
	/*for (int i=0;i<8;i++)
	{
		*(szBuf+i) = SYNCDATA;
	}*/
	
	szBuf[0] = MTYPE_LINK_REQUEST;
	szBuf[1] = 0x00;
	szBuf[2] = 0x05;
	getRandomHex(random);
	wrHex("随机数",random,4);
	memcpy(szBuf+3,random,4);
	//verifyRandom(random,szrandom);
	memset(szrandom,0,sizeof(szrandom));
	memcpy(szrandom,random,4);
	szBuf[7] = SYNCSEQNO_LINK;
	szBuf[8] = verify(szBuf,8);//校验位
	//memcpy(szBuf+8,tmpstr,sizeof(tmpstr));
	ret = 9;
	//wrHex("建链报文:",szBuf,17);
	return ret;
}

//---------------构造接收响应报文------------------------------
int Construct_AckPack(BYTE* szBuf,BYTE* random,BYTE syncSeqNo)
{
	int ret = 0;
	/*BYTE tmpstr[9];
	memset(tmpstr,0,sizeof(tmpstr));*/
	/*for (int i=0;i<8;i++)
	{
		*(szBuf+i) = SYNCDATA;
	}*/
	szBuf[0] = MTYPE_RECEIVE_DONE;
	szBuf[1] = 0x00;
	szBuf[2] = 0x05;
	memcpy(szBuf+3,random,4);
	szBuf[7] = syncSeqNo;
	szBuf[8] = verify(szBuf,8);//校验位
	
	ret = 9;
	return ret;
}


//------------------删除载波数据------------------------
bool checkPackComplete(int &length,BYTE* szBuf)
{
	int i,packLen;
	//index = 4;
	for(i=0; i<length; i++)
	{
		if(*(szBuf+i) == MTYPE_TRADE_TREM)
			break;
	}
	if(length < i)
	{
		return false;
	}
	packLen = getPackLen(szBuf+i+1);
	if(length < (packLen+i))
	{
		wrLog("长度检验错误:recv:%d,save:%d",packLen+i,length);
		return false;
	}
	length = packLen+4;
	BYTE tempbuff[1024];
	memcpy(tempbuff, szBuf+i, length);
	memcpy(szBuf,tempbuff,length);
	return true;
}

//---------------检查FSK报文合法性----------------------
bool PackCheck(int length,BYTE* szBuf,BYTE* szrandom,BYTE syncSeqNo)
{
	//wrHex("接收FSK数据",szBuf,length);
	int syncIndex = 2;
	BYTE random[5];
	memcpy(random,szBuf+3,4);
	if(memcmp(random, szrandom, 4)!=0)
	{
		wrLog("not the same random! recv:%s,save:%s",random,szrandom);
		return false;
	}
	syncIndex = 7;
	BYTE verifyValue= verify(szBuf,length);
	wrHex("校验值",&verifyValue,1);
	if(szBuf[0] != MTYPE_TRADE_TREM)
	{
		wrLog("not the term msg");
		return false;
	}
	if(szBuf[syncIndex] != (BYTE)(syncSeqNo + 1))
	{
		wrLog("not the right sync");
		return false;
	}
	//if(*(szBuf+length-1) != verify(szBuf,length-1)) //新协议长度不包括校验位
	if(szBuf[length] != verifyValue)
	{	//包检验位
		wrLog("verify fault");
		return false;
	}
	wrLog("fsk check true");
	return true;
}

//---------------检查建链响应报文合法性----------------------
//返回：1.错包 -1.包不完整 0.正确的包

int LinkAckPackCheck(BYTE* szBuf,int length,BYTE* szrandom)
{
	int i = 0;
	for(i=0; i<length; i++)
	{
		if(szBuf[i] == MTYPE_LINK_ANSWER)
			break;
	}
	if(length < (i+4))
	{
		return -1;
	}
	int packLen = getPackLen(szBuf+i+1);
	if(length < (packLen+4+i))
	{
		return -1;
	}
	length = packLen+4;
	BYTE tempbuff[1024];
	memcpy(tempbuff, szBuf+i, length);
	memcpy(szBuf,tempbuff,length);
	wrHex("接收完整的FSK数据",szBuf,length);
	if(szBuf[length] != verify(szBuf,length))
	{
		//wrChannel("verify fault",pOneTrunk);
		wrLog("校验位错误!");
		return 1;
	}
	if(memcmp(szrandom,szBuf+3,4) != 0)
	{
		//wrChannel("随机数错误",pOneTrunk);
		wrLog("随机数错误!");
		return 1;
	}
	//memcpy(m_random,&m_pRecvbyFSKBuf[3],4);//不验证随机数
	if(szBuf[0] != MTYPE_LINK_ANSWER)
	{
		return 1;
	}
	if(szBuf[7] != 0x00)
	{
		//wrChannel("同步码错误",pOneTrunk);
		wrLog("同步码错误!");
		return 1;
	}

	return 0;
}

//---------------检查FSK响应报文合法性----------------------
//返回：1.错包 -1.包不完整 0.正确的包
int AckPackCheck(BYTE* szBuf,int length,BYTE syncSeqNo,BYTE* szrandom)
{
	int i = 0;
	while(1)
	{
		for(; i<length; i++)
		{
			if(szBuf[i] == MTYPE_RECEIVE_DONE)
				break;
		}
		if(length < (i+4))
		{
			return -1;
		}
		int packLen = getPackLen(szBuf+i+1);
		if(packLen>6)
		{
			i++;
			continue;
		}
		if(length < (packLen+4+i))
			return -1;
		length = packLen+4;
		//BYTE tempbuff[1024];
		//memcpy(tempbuff, &m_pRecvbyFSKBuf[i], length);
		//memcpy(m_pRecvbyFSKBuf,tempbuff,length);
		wrHex("接收完整的FSK数据",szBuf+i,length);
		BYTE random[5];
		memcpy(random,szBuf+i+3,4);
		if(szBuf[i+length] != verify(szBuf+i,length))
		{
			//wrChannel("verify fault",pOneTrunk);
			return 1;
		}
		if(szBuf[i] != MTYPE_RECEIVE_DONE)
		{
			return 1;
		}
		if(szBuf[i+7] != syncSeqNo)
		{
			wrLog("seqno[%d]",(int)syncSeqNo);
			return 1;
		}
		if(memcmp(random, szrandom, 4)!=0)
		{
			wrHex("rece random",random,4);
			wrHex("save random",szrandom,4);
			return 1;
		}
		return 0;
	}
	return 1;
}

//----------------增加同步随机数------------------------------
void addSeqNo(BYTE &syncSeqNo)
{
	syncSeqNo = (BYTE)((int)syncSeqNo+1);
}
