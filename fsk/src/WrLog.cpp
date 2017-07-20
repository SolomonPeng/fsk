#include "WrLog.h"
#include "adapter.h"


void wrHex(char *name,BYTE *hex,int hex_len)		//bcd转成字符串后写入log文件
{
	BYTE str[8192];
	memset(str,0,sizeof(str));
	CAdapter::byte_to_hex(hex,str,hex_len);

	wrLog("%s[%d]：[%s]",name,hex_len*2,str);
}

//写log
void wrLog(const char *format,...)
{
	/*if(FskDataLog == 0)
		return 0;*/
	
	va_list ap;
	int n;
	char buff[8192];
	char strTime[12+1];
	char strDay[8+1];
	char temp[64];

	memset(buff,0,sizeof(buff));
	va_start (ap,format);
	vsprintf(buff,format,ap);
	va_end(ap);

	n = strlen(buff);
	memset( strDay, 0, sizeof(strDay) );
	get_date8(strDay);
	memset( strTime, 0, sizeof(strTime) );
	get_time8(strTime);

	memset( temp, 0, sizeof(temp) );
	//sprintf( temp, "./log/%s",strDay);	
	FILE *fp;
	strcat(temp, "./log/TFSK");
	strcat(temp, strDay);
	strcat(temp, ".log");
	//printf("temp:%s\n",temp);
	fp = fopen( temp, "a");

	if( fp == NULL)
		fp = fopen( temp, "w+");

	memset( temp, 0, sizeof(temp) );
	sprintf(temp, "[%s %s] ", strDay, strTime);
	fputs(temp, fp);
	fputs(buff, fp);
	fputs("\n", fp);
	fclose(fp);
}

void get_date8 (char *cDate)	//取8位日期，e.g 20060809
{
	static char buff[8+1];
	struct tm     *tm_now;
	time_t        time_now;
	
	time_now = time (NULL);
	tm_now = localtime (&time_now);
	sprintf (buff, "%04d%02d%02d", tm_now->tm_year + 1900,
		tm_now->tm_mon + 1, tm_now->tm_mday);
	strcpy(cDate,buff);
}

void get_time8(char *cTime)		//取8位时间，e.g 15:56:24
{
	static char buff[8+1];
	time_t t;
	struct tm tm;
	time(&t);
	memcpy(&tm, localtime(&t), sizeof(tm));
	sprintf(buff, "%.2d:%.2d:%.2d",tm.tm_hour,tm.tm_min,tm.tm_sec);
	strcpy(cTime, buff);
}


