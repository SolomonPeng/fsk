

#include "Connect.h"


#define MAXBUF	8192 //����socket��buf���ֵ
char						g_PlatformIP[32];
int							g_PlatformPort;
int							g_PlatformTimeout;


void* CTSISendRecvData(void* lpv)
{	
	//wrLog("�߳�");
	
	//wrLog("thread address[%d]",(int *)lpv);

	ctsimsg ch;
	
	memcpy(&ch, (pctsimsg)lpv, sizeof(ctsimsg));
	
	//wrLog("length[%d]",ch.length);

	int sd = ch.sd;

	//wrLog("socket[%d]",sd);

	int send_len = ch.length;

	BYTE * send_buff = (BYTE *)malloc(sizeof(BYTE)*send_len);

	memcpy(send_buff,ch.buff,send_len);

	wrHex("����CTSI����",send_buff,send_len);
	
	BYTE * rev_buff = (BYTE *)malloc(sizeof(BYTE)*LEN_1024);

	memset(rev_buff,0x00,LEN_1024);

	int iRet = HOSTSendRecvData(sd,(char *)send_buff,send_len,(char *)rev_buff,1024);
	
	//wrHex("�յ���������",rev_buff,strlen((char *)rev_buff));

	//wrLog("send success[%d]",iRet);
	
	free(send_buff);
	//free(rev_buff);
	
	//pctsimsg temp = (pctsimsg)lpv;

	if(iRet > 0)
	{
		memcpy(((pctsimsg)lpv)->buff,rev_buff,iRet);
		//wrLog("buff address[%d]",(int *)((pctsimsg)lpv)->buff);
		//wrHex("�̷߳�������",((pctsimsg)lpv)->buff,iRet);
		((pctsimsg)lpv)->length = iRet;
		((pctsimsg)lpv)->flag = 1;
		//wrLog("�߳�return 0");
		free(rev_buff);
	}
	else{
		((pctsimsg)lpv)->flag = -1;
		//wrLog("�߳�return -1");
	}

	//pthread_exit((void*)&sd);
}

void* CTSISendFailData(void* lpv)
{	
	//wrLog("�߳�");
	
	//wrLog("thread address[%d]",(int *)lpv);

	ctsimsg ch;
	
	memcpy(&ch, (pctsimsg)lpv, sizeof(ctsimsg));
	
	//wrLog("length[%d]",ch.length);

	int sd = ch.sd;

	//wrLog("socket[%d]",sd);

	int send_len = ch.length;

	BYTE * send_buff = (BYTE *)malloc(sizeof(BYTE)*send_len);

	memcpy(send_buff,ch.buff,send_len);

	wrHex("����CTSI����",send_buff,send_len);

	int iRet = sndMsg(sd,(char *)send_buff,send_len);

	//wrLog("send result[%d]",iRet);
	
	free(send_buff);

	close(sd);

	//pthread_exit((void*)&sd);
}

//---------------------------------------------------------------------------
//������:       HostSendRecvData
//����������    ���������ͽ������󣬲�����Ӧ����Ϣ
//���룺	sendData�����ͱ������ݣ�sendLen�����ͱ��ĳ��ȣ�recvLen�����ձ��ĳ���
//�����	recvData�����ձ���
//���أ�	0���ɹ�������������
//����SOCK��ͬ�������ӷ�ʽ������ͨѶ��������

int HOSTSendRecvData(int sd, char *sendData,int sendLen,char * recvData,int recvLen)
{
    int iRet;
	char len_buff[4+1];
	long rev_len;
	
	iRet = sndMsg(sd,sendData,sendLen);
	if(iRet == -1)
		return -1;
	else if(iRet == -2)
		return -2;

	iRet = rcvMsg(sd,recvData,1024);
	//close(sd);
	if(iRet == -1)
		return -1;
	else if(iRet == -3)
		return -3;

	//bcd_to_asc( (BYTE *)len_buff, (BYTE *)recvData, 4, 0);
	//rev_len =atol(len_buff);
	//if(rev_len != (iRet - 2))	//�յ��������İ� 
	//	return -1;
	return iRet;
}

int sndMsg(int sd,char *sendData,int sendLen)
{
	int iRet;
	fd_set  fdW;
	struct timeval timeout;
	timeout.tv_sec = g_PlatformTimeout;	//���ó�ʱ�˳�ʱ��Ϊ40��
	timeout.tv_usec = 0;
	FD_ZERO(&fdW);
	FD_SET(sd, &fdW);
	
	switch (select(sd+1, NULL, &fdW, NULL,&timeout))
	{
	case -1: 
		{
			wrLog("����ʧ��");
			close(sd);
			return -1;//error handled by u; 
		}
	case 0: 
		{
			wrLog("���ͳ�ʱ");
			close(sd);
			return -2;//timeout hanled by u; 
		}
	default: 
		if( FD_ISSET(sd, &fdW) )
		{
			iRet=send( sd, sendData, sendLen, 0 ) ;
			wrLog("��������[%d]",iRet);
			return iRet;
		}
		
	}
}

int rcvMsg(int sd,char *recvData,int recvLen)
{
	int iRet;
	fd_set  fdR;
	struct timeval timeout;
	timeout.tv_sec = g_PlatformTimeout;	//���ó�ʱ�˳�ʱ��(��λ����)
	timeout.tv_usec = 0;
	FD_ZERO(&fdR);
	FD_SET(sd, &fdR);

	switch (select(sd+1, &fdR, NULL, NULL,&timeout)) 
	{ 
	case -1: 
		{
			wrLog("����ʧ��");
			return -1;//error handled by u;
		}
	case 0: 
		{
			wrLog("���ճ�ʱ");
			return -3;//timeout hanled by u; 
		}
	default: 
		if (  FD_ISSET(sd, &fdR) )
		{
			iRet = recv( sd, recvData, recvLen, 0 );
			wrLog("���ճ���[%d]",iRet);
			wrHex("���ذ�����",(BYTE*)recvData,iRet);
			return iRet;
		}
		
	}
}



int ConToHost()
{
	int sd;
	struct sockaddr_in their_addr;
	struct hostent *hostEnt = NULL;

	//WSADATA   wsaData;
	//WORD   version   =   MAKEWORD(2,0);
	//int   ret   =   WSAStartup(version,&wsaData);
	//if(ret   !=   0)
	//{
	//	 wrLog("��ʼ��ʧ��");
	//  return -1;
	//}

    wrLog("��������%s,%d����",g_PlatformIP,g_PlatformPort);
	
    //��������
	if ( (	sd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP )) == INVALID_SOCKET )
	{
        wrLog("socket��ʼ��ʧ��");
        return -1;
	}
	
	
	if ((hostEnt = gethostbyname(g_PlatformIP)) == NULL)
	{ /* get the host info */
		wrLog("��ȡ��������Ϣ����");
		return -1;
	}
	
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons((u_short)g_PlatformPort);
	their_addr.sin_addr = *((struct in_addr *)hostEnt->h_addr);
	memset( their_addr.sin_zero , 0, 8 );//sizeof(their_addr.sin_zero)

	if ( connect(sd, (struct sockaddr *)&their_addr,sizeof(struct sockaddr) ) == -1) 
	{
		wrLog("���ӷ���������");
		return -1;
	}
	wrLog("Connect OK!");
	return sd;
}

