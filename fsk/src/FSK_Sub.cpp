#include "FSK_Sub.h"
#include "adapter.h"

void wrChannel(char *str,TRUNK_STRUCT *pOneTrunk)
{
	char * msg = "通道信息";
	wrLog("%s[%d]：[%s]",msg,pOneTrunk->deviceID.m_s16ChannelID,str);
}

void ReleaseCall(TRUNK_STRUCT *pOneTrunk)
{	
	if (pOneTrunk->State != TRK_FREE)
	{
		ClearCall(pOneTrunk);

		Change_State ( pOneTrunk, TRK_HANGON );

		if(pOneTrunk->msg->sd != INVALID_SOCKET)
		{			
			close(pOneTrunk->msg->sd);
		}

	}
}

void ResetTimer(TRUNK_STRUCT *pOneTrunk)
{
	pOneTrunk->loop = 0;
}

void Send_LinkPack(TRUNK_STRUCT *pOneTrunk)
{
	if(pOneTrunk->loop >= SEND_MAX_COUNT)
	{
		wrLog("通道信息[%d]：[建链失败，主动挂机]",pOneTrunk->deviceID.m_s16ChannelID);
		ReleaseCall(pOneTrunk);
		return;
	}

	int len;
	len = Construct_LinkPack(pOneTrunk->szSendbyFSKBuf,pOneTrunk->szRandom);
	wrHex("发送建链报文",pOneTrunk->szSendbyFSKBuf,len);

	if(SendFSK(pOneTrunk,len))
	{
		Change_State ( pOneTrunk, TRK_LINK_REQUEST );
	}
	else
	{
		pOneTrunk->loop++;
		return;
	}
}

void Send_FSKPack(TRUNK_STRUCT *pOneTrunk)
{
	if(pOneTrunk->loop >= SEND_MAX_COUNT)
	{
		/*wrChannel("发送FSK报文无响应，挂机",pOneTrunk);
		WriteMonitorLog(NORECEIVEDONE,pOneTrunk);*/
		wrLog("通道信息[%d]：[发送FSK报文无响应，挂机]",pOneTrunk->deviceID.m_s16ChannelID);
		ReleaseCall(pOneTrunk);
		return;
	}
	if(SendFSK(pOneTrunk,pOneTrunk->packLength))
	{
		Change_State ( pOneTrunk, TRK_SENDFSK );
	}
	else
	{
		pOneTrunk->loop++;
		return;
	}	
}

void Send_AckPack(TRUNK_STRUCT *pOneTrunk)
{
	if(pOneTrunk->loop >= SEND_MAX_COUNT)
	{
		/*wrChannel("发送FSK报文失败，挂机",pOneTrunk);
		WriteMonitorLog(SENDFSKFAULT,pOneTrunk);*/
		wrLog("通道信息[%d]：[发送FSK报文失败，挂机]",pOneTrunk->deviceID.m_s16ChannelID);
		ReleaseCall(pOneTrunk);
		return;
	}
	int len;
	len = Construct_AckPack(pOneTrunk->szSendbyFSKBuf,pOneTrunk->szRandom,pOneTrunk->syncSeqNo);
	wrHex("发送接收完成报文",pOneTrunk->szSendbyFSKBuf,len);
	if(SendFSK(pOneTrunk,len))
	{
		Change_State ( pOneTrunk, TRK_SENDFSKACK );
	}
	else
	{
		pOneTrunk->loop++;
		return;
	}
}

void RecvCTSI(TRUNK_STRUCT *pOneTrunk)
{
	if(pOneTrunk->msg->flag > 0)
	{
		memset(pOneTrunk->FskBuf,0,LEN_1024);
		int len = getPackLen(pOneTrunk->msg->buff);
		
		if(len != ((pOneTrunk->msg->length)-2))
		{			
			wrLog("通道信息[%d]：[CTSI返回数据不合法，挂机]",pOneTrunk->deviceID.m_s16ChannelID);
			ReleaseCall(pOneTrunk);
			return;
		}
		memcpy(pOneTrunk->FskBuf,pOneTrunk->msg->buff+2,len);
		wrHex("FskBuf报文",pOneTrunk->FskBuf,len);
		addSeqNo(pOneTrunk->syncSeqNo);

		pOneTrunk->packLength = Construct_FSKPack(pOneTrunk->szSendbyFSKBuf,pOneTrunk->szRandom,pOneTrunk->syncSeqNo,pOneTrunk->FskBuf,len);
		pOneTrunk->loop = 0;
		wrHex("FSK报文",pOneTrunk->szSendbyFSKBuf,pOneTrunk->packLength);
		Change_State ( pOneTrunk, TRK_CONNECT_SENDFSK);
		Send_FSKPack(pOneTrunk);
		
	}
	else if(pOneTrunk->msg->flag <= 0)
	{
		/*wrChannel("CTSI无返回，挂机",pOneTrunk);
		WriteMonitorLog(NOCTSIRESPONSE,pOneTrunk);*/
		wrLog("通道信息[%d]：[CTSI无返回，挂机]",pOneTrunk->deviceID.m_s16ChannelID);
		ReleaseCall(pOneTrunk);
	}

}

//---------------校验收到的链接响应包，连接中心----------------------
void RecvLinkACK(TRUNK_STRUCT *pOneTrunk,int length)
{
	int ackState = 0;	
	if(length <= 0)
	{
		return;
	}
	wrHex("收到FSK报文",pOneTrunk->szRecvbyFSKBuf,length);
	ackState = LinkAckPackCheck(pOneTrunk->szRecvbyFSKBuf,length,pOneTrunk->szRandom);

	if(ackState == 0)
	{
		pOneTrunk->loop = 0;
		wrChannel("接受建链响应包成功",pOneTrunk);
		if(pOneTrunk->msg->sd == INVALID_SOCKET)
		{
			if((pOneTrunk->msg->sd=ConToHost()) == -1)
			{
				wrChannel("连接中心失败，挂机",pOneTrunk);
				ReleaseCall(pOneTrunk);
				return;
			}
		}
		Change_State ( pOneTrunk, TRK_RECVFSK );
	}
	else if(ackState == 1)
	{
		wrChannel("检验失败",pOneTrunk);
		pOneTrunk->loop++;

		Change_State ( pOneTrunk, TRK_LINK_REQUEST );
		//sleep(100);
		Send_LinkPack(pOneTrunk);
		return;

	}
	else if(ackState == -1)
	{
		if(checkPackComplete(length,pOneTrunk->szRecvbyFSKBuf))
		{
			if(PackCheck(length,pOneTrunk->szRecvbyFSKBuf,pOneTrunk->szRandom,pOneTrunk->syncSeqNo))
			{
				wrChannel("无终端的响应报文确认，继续交易",pOneTrunk);
				pOneTrunk->loop = 0;
				addSeqNo(pOneTrunk->syncSeqNo);
				pOneTrunk->packLength = length-10;//新终端用到
				Send_AckPack(pOneTrunk);
				return;
			}
		}
		if(length > 600)
		{
			memset(pOneTrunk->szRecvbyFSKBuf,0,sizeof(pOneTrunk->szRecvbyFSKBuf));
		}
	}
}


//---------------校验收到的FSK响应包----------------------
void RecvFSKACK(TRUNK_STRUCT *pOneTrunk,int length)
{	
	int ackState = 0;	
	if(length <= 0)
	{
		return;
	}
	wrHex("收到FSK报文",pOneTrunk->szRecvbyFSKBuf,length);	

	
	ackState = AckPackCheck(pOneTrunk->szRecvbyFSKBuf,length,pOneTrunk->syncSeqNo,pOneTrunk->szRandom);
	if(ackState == 0)
	{
		pOneTrunk->loop = 0;
		memset(pOneTrunk->szRecvbyFSKBuf,0,1024);
		Change_State ( pOneTrunk, TRK_RECVFSK );

	}
	else if(ackState == 1)
	{
		wrChannel("检验失败",pOneTrunk);
		pOneTrunk->loop++;

		Send_FSKPack(pOneTrunk);

	}
	else if(ackState == -1)
	{
		if(checkPackComplete(length,pOneTrunk->szRecvbyFSKBuf))
		{
			if(PackCheck(length,pOneTrunk->szRecvbyFSKBuf,pOneTrunk->szRandom,pOneTrunk->syncSeqNo))
			{
				wrChannel("无终端的响应报文确认，继续交易",pOneTrunk);
				pOneTrunk->loop = 0;
				addSeqNo(pOneTrunk->syncSeqNo);
				pOneTrunk->packLength = length-10;//新终端用到
				Send_AckPack(pOneTrunk);
				return;
			}
		}
		if(length > 600)
		{
			memset(pOneTrunk->szRecvbyFSKBuf,0,sizeof(pOneTrunk->szRecvbyFSKBuf));
		}
	}
}


int RecvFSK(TRUNK_STRUCT *pOneTrunk,int iLength)
{
	if ( iLength != 0)
	{
		wrHex("收到FSK报文",pOneTrunk->szRecvbyFSKBuf,iLength);
		if(!checkPackComplete(iLength,pOneTrunk->szRecvbyFSKBuf))
		{
			if(iLength > 600)
			{
				memset(pOneTrunk->szRecvbyFSKBuf,0,sizeof(pOneTrunk->szRecvbyFSKBuf));
			}
			return 1;
		}

		if(PackCheck(iLength,pOneTrunk->szRecvbyFSKBuf,pOneTrunk->szRandom,pOneTrunk->syncSeqNo))
		{
			addSeqNo(pOneTrunk->syncSeqNo);
			pOneTrunk->packLength = iLength-11;//新终端用到
			
			return 0;
		}
		else
			return -1;
	}
	return 1;
}

