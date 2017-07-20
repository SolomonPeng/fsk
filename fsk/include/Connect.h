// Connect.h: interface for the CConnect class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __CONNECT_H__
#define __CONNECT_H__

#include <stdlib.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
//#include <pthread.h>
#include <string.h>
#include "FSK_Data.h"

void* CTSISendRecvData(void* lpv);
void* CTSISendFailData(void* lpv);
int sndMsg(int sd,char *sendData,int sendLen);
int rcvMsg(int sd,char *recvData,int recvLen);
int HOSTSendRecvData(int sd, char *sendData,int sendLen,char * recvData,int recvLen);

//by ps
int ConToHost();
//int ConToHost(bool eposFlag,int &isd);



#endif // __CONNECT_H__
