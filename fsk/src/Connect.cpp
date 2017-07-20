

#include "Connect.h"


#define MAXBUF	8192 //诚联socket的buf最大值
char						g_PlatformIP[32];
int							g_PlatformPort;
int							g_PlatformTimeout;


void* CTSISendRecvData(void* lpv)
{	
	//wrLog("线程");
	
	//wrLog("thread address[%d]",(int *)lpv);

	ctsimsg ch;
	
	memcpy(&ch, (pctsimsg)lpv, sizeof(ctsimsg));
	
	//wrLog("length[%d]",ch.length);

	int sd = ch.sd;

	//wrLog("socket[%d]",sd);

	int send_len = ch.length;

	BYTE * send_buff = (BYTE *)malloc(sizeof(BYTE)*send_len);

	memcpy(send_buff,ch.buff,send_len);

	wrHex("发送CTSI数据",send_buff,send_len);
	
	BYTE * rev_buff = (BYTE *)malloc(sizeof(BYTE)*LEN_1024);

	memset(rev_buff,0x00,LEN_1024);

	int iRet = HOSTSendRecvData(sd,(char *)send_buff,send_len,(char *)rev_buff,1024);
	
	//wrHex("收到返回数据",rev_buff,strlen((char *)rev_buff));

	//wrLog("send success[%d]",iRet);
	
	free(send_buff);
	//free(rev_buff);
	
	//pctsimsg temp = (pctsimsg)lpv;

	if(iRet > 0)
	{
		memcpy(((pctsimsg)lpv)->buff,rev_buff,iRet);
		//wrLog("buff address[%d]",(int *)((pctsimsg)lpv)->buff);
		//wrHex("线程返回数据",((pctsimsg)lpv)->buff,iRet);
		((pctsimsg)lpv)->length = iRet;
		((pctsimsg)lpv)->flag = 1;
		//wrLog("线程return 0");
		free(rev_buff);
	}
	else{
		((pctsimsg)lpv)->flag = -1;
		//wrLog("线程return -1");
	}

	//pthread_exit((void*)&sd);
}

void* CTSISendFailData(void* lpv)
{	
	//wrLog("线程");
	
	//wrLog("thread address[%d]",(int *)lpv);

	ctsimsg ch;
	
	memcpy(&ch, (pctsimsg)lpv, sizeof(ctsimsg));
	
	//wrLog("length[%d]",ch.length);

	int sd = ch.sd;

	//wrLog("socket[%d]",sd);

	int send_len = ch.length;

	BYTE * send_buff = (BYTE *)malloc(sizeof(BYTE)*send_len);

	memcpy(send_buff,ch.buff,send_len);

	wrHex("发送CTSI数据",send_buff,send_len);

	int iRet = sndMsg(sd,(char *)send_buff,send_len);

	//wrLog("send result[%d]",iRet);
	
	free(send_buff);

	close(sd);

	//pthread_exit((void*)&sd);
}

//---------------------------------------------------------------------------
//函数名:       HostSendRecvData
//功能描述：    向主机发送交易请求，并接受应答信息
//输入：	sendData：发送报文内容，sendLen：发送报文长度，recvLen：接收报文长度
//输出：	recvData：接收报文
//返回：	0：成功，其他：错误
//采用SOCK，同步短连接方式与主机通讯交互数据

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
	//if(rev_len != (iRet - 2))	//收到不完整的包 
	//	return -1;
	return iRet;
}

int sndMsg(int sd,char *sendData,int sendLen)
{
	int iRet;
	fd_set  fdW;
	struct timeval timeout;
	timeout.tv_sec = g_PlatformTimeout;	//设置超时退出时间为40秒
	timeout.tv_usec = 0;
	FD_ZERO(&fdW);
	FD_SET(sd, &fdW);
	
	switch (select(sd+1, NULL, &fdW, NULL,&timeout))
	{
	case -1: 
		{
			wrLog("发送失败");
			close(sd);
			return -1;//error handled by u; 
		}
	case 0: 
		{
			wrLog("发送超时");
			close(sd);
			return -2;//timeout hanled by u; 
		}
	default: 
		if( FD_ISSET(sd, &fdW) )
		{
			iRet=send( sd, sendData, sendLen, 0 ) ;
			wrLog("发出数据[%d]",iRet);
			return iRet;
		}
		
	}
}

int rcvMsg(int sd,char *recvData,int recvLen)
{
	int iRet;
	fd_set  fdR;
	struct timeval timeout;
	timeout.tv_sec = g_PlatformTimeout;	//设置超时退出时间(单位：秒)
	timeout.tv_usec = 0;
	FD_ZERO(&fdR);
	FD_SET(sd, &fdR);

	switch (select(sd+1, &fdR, NULL, NULL,&timeout)) 
	{ 
	case -1: 
		{
			wrLog("接收失败");
			return -1;//error handled by u;
		}
	case 0: 
		{
			wrLog("接收超时");
			return -3;//timeout hanled by u; 
		}
	default: 
		if (  FD_ISSET(sd, &fdR) )
		{
			iRet = recv( sd, recvData, recvLen, 0 );
			wrLog("接收长度[%d]",iRet);
			wrHex("返回包内容",(BYTE*)recvData,iRet);
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
	//	 wrLog("初始化失败");
	//  return -1;
	//}

    wrLog("连接主机%s,%d……",g_PlatformIP,g_PlatformPort);
	
    //连接主机
	if ( (	sd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP )) == INVALID_SOCKET )
	{
        wrLog("socket初始化失败");
        return -1;
	}
	
	
	if ((hostEnt = gethostbyname(g_PlatformIP)) == NULL)
	{ /* get the host info */
		wrLog("获取服务器信息错误");
		return -1;
	}
	
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons((u_short)g_PlatformPort);
	their_addr.sin_addr = *((struct in_addr *)hostEnt->h_addr);
	memset( their_addr.sin_zero , 0, 8 );//sizeof(their_addr.sin_zero)

	if ( connect(sd, (struct sockaddr *)&their_addr,sizeof(struct sockaddr) ) == -1) 
	{
		wrLog("连接服务器错误");
		return -1;
	}
	wrLog("Connect OK!");
	return sd;
}

