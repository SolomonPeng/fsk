#include "tools.h"

#define OK 0
#define NOTFOUND 100

#ifndef NULL
#define NULL 0
#endif

int read_string(TIni g_cfg, const char* domain,const char* name,char* value,int bufflen)
{
	int ret = 0;
	if( value == NULL )
	{
		//glog.LogString(LOG_NETWORK,"[[BILLING][CFG][ERROR][%s %s->%s error:INVALID pointer!]",__FUNCTION__,domain,name);
		//return ERROR_INVALID_POINTER;
		return -1;
	}
	memset(value,0,bufflen);
	ret = g_cfg.GetIniString(domain, name, value);
	if( ret != 0 )
	{
		//glog.LogString(LOG_NETWORK,"[BILLING][CFG][ERROR][%s %s->%s error:NOTFOUND!]",__FUNCTION__,domain,name);
		//return NOTFOUND;
		return -2;
	}
	return 0;	
}

int read_int(TIni g_cfg,const char* domain,const char* name,int & value)
{
	char buff[120];
	int ret = 0 ;
	ret = read_string(g_cfg,domain,name,buff,sizeof(buff));
	if( ret == OK )	
	{
		value = atoi( buff );
		//DG(glog.LogString(LOG_NETWORK,"[BILLING][CFG][INFO][%s %s->%s [%s]=>[%d] ]",__FUNCTION__,domain,name,buff,value);)
		return ret;
	}
	return ret;
}


