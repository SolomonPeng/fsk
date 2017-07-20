#include "FSK_Sub.h"
#include "adapter.h"

void wrChannel(char *str,TRUNK_STRUCT *pOneTrunk)
{
	char * msg = "ͨ����Ϣ";
	wrLog("%s[%d]��[%s]",msg,pOneTrunk->deviceID.m_s16ChannelID,str);
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
		wrLog("ͨ����Ϣ[%d]��[����ʧ�ܣ������һ�]",pOneTrunk->deviceID.m_s16ChannelID);
		ReleaseCall(pOneTrunk);
		return;
	}

	int len;
	len = Construct_LinkPack(pOneTrunk->szSendbyFSKBuf,pOneTrunk->szRandom);
	wrHex("���ͽ�������",pOneTrunk->szSendbyFSKBuf,len);

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
		/*wrChannel("����FSK��������Ӧ���һ�",pOneTrunk);
		WriteMonitorLog(NORECEIVEDONE,pOneTrunk);*/
		wrLog("ͨ����Ϣ[%d]��[����FSK��������Ӧ���һ�]",pOneTrunk->deviceID.m_s16ChannelID);
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
		/*wrChannel("����FSK����ʧ�ܣ��һ�",pOneTrunk);
		WriteMonitorLog(SENDFSKFAULT,pOneTrunk);*/
		wrLog("ͨ����Ϣ[%d]��[����FSK����ʧ�ܣ��һ�]",pOneTrunk->deviceID.m_s16ChannelID);
		ReleaseCall(pOneTrunk);
		return;
	}
	int len;
	len = Construct_AckPack(pOneTrunk->szSendbyFSKBuf,pOneTrunk->szRandom,pOneTrunk->syncSeqNo);
	wrHex("���ͽ�����ɱ���",pOneTrunk->szSendbyFSKBuf,len);
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
			wrLog("ͨ����Ϣ[%d]��[CTSI�������ݲ��Ϸ����һ�]",pOneTrunk->deviceID.m_s16ChannelID);
			ReleaseCall(pOneTrunk);
			return;
		}
		memcpy(pOneTrunk->FskBuf,pOneTrunk->msg->buff+2,len);
		wrHex("FskBuf����",pOneTrunk->FskBuf,len);
		addSeqNo(pOneTrunk->syncSeqNo);

		pOneTrunk->packLength = Construct_FSKPack(pOneTrunk->szSendbyFSKBuf,pOneTrunk->szRandom,pOneTrunk->syncSeqNo,pOneTrunk->FskBuf,len);
		pOneTrunk->loop = 0;
		wrHex("FSK����",pOneTrunk->szSendbyFSKBuf,pOneTrunk->packLength);
		Change_State ( pOneTrunk, TRK_CONNECT_SENDFSK);
		Send_FSKPack(pOneTrunk);
		
	}
	else if(pOneTrunk->msg->flag <= 0)
	{
		/*wrChannel("CTSI�޷��أ��һ�",pOneTrunk);
		WriteMonitorLog(NOCTSIRESPONSE,pOneTrunk);*/
		wrLog("ͨ����Ϣ[%d]��[CTSI�޷��أ��һ�]",pOneTrunk->deviceID.m_s16ChannelID);
		ReleaseCall(pOneTrunk);
	}

}

//---------------У���յ���������Ӧ������������----------------------
void RecvLinkACK(TRUNK_STRUCT *pOneTrunk,int length)
{
	int ackState = 0;	
	if(length <= 0)
	{
		return;
	}
	wrHex("�յ�FSK����",pOneTrunk->szRecvbyFSKBuf,length);
	ackState = LinkAckPackCheck(pOneTrunk->szRecvbyFSKBuf,length,pOneTrunk->szRandom);

	if(ackState == 0)
	{
		pOneTrunk->loop = 0;
		wrChannel("���ܽ�����Ӧ���ɹ�",pOneTrunk);
		if(pOneTrunk->msg->sd == INVALID_SOCKET)
		{
			if((pOneTrunk->msg->sd=ConToHost()) == -1)
			{
				wrChannel("��������ʧ�ܣ��һ�",pOneTrunk);
				ReleaseCall(pOneTrunk);
				return;
			}
		}
		Change_State ( pOneTrunk, TRK_RECVFSK );
	}
	else if(ackState == 1)
	{
		wrChannel("����ʧ��",pOneTrunk);
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
				wrChannel("���ն˵���Ӧ����ȷ�ϣ���������",pOneTrunk);
				pOneTrunk->loop = 0;
				addSeqNo(pOneTrunk->syncSeqNo);
				pOneTrunk->packLength = length-10;//���ն��õ�
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


//---------------У���յ���FSK��Ӧ��----------------------
void RecvFSKACK(TRUNK_STRUCT *pOneTrunk,int length)
{	
	int ackState = 0;	
	if(length <= 0)
	{
		return;
	}
	wrHex("�յ�FSK����",pOneTrunk->szRecvbyFSKBuf,length);	

	
	ackState = AckPackCheck(pOneTrunk->szRecvbyFSKBuf,length,pOneTrunk->syncSeqNo,pOneTrunk->szRandom);
	if(ackState == 0)
	{
		pOneTrunk->loop = 0;
		memset(pOneTrunk->szRecvbyFSKBuf,0,1024);
		Change_State ( pOneTrunk, TRK_RECVFSK );

	}
	else if(ackState == 1)
	{
		wrChannel("����ʧ��",pOneTrunk);
		pOneTrunk->loop++;

		Send_FSKPack(pOneTrunk);

	}
	else if(ackState == -1)
	{
		if(checkPackComplete(length,pOneTrunk->szRecvbyFSKBuf))
		{
			if(PackCheck(length,pOneTrunk->szRecvbyFSKBuf,pOneTrunk->szRandom,pOneTrunk->syncSeqNo))
			{
				wrChannel("���ն˵���Ӧ����ȷ�ϣ���������",pOneTrunk);
				pOneTrunk->loop = 0;
				addSeqNo(pOneTrunk->syncSeqNo);
				pOneTrunk->packLength = length-10;//���ն��õ�
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
		wrHex("�յ�FSK����",pOneTrunk->szRecvbyFSKBuf,iLength);
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
			pOneTrunk->packLength = iLength-11;//���ն��õ�
			
			return 0;
		}
		else
			return -1;
	}
	return 1;
}

