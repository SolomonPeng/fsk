#ifndef __WRLOG_H__
#define __WRLOG_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "DataDefine.h"

void wrLog(const char *format,...);
void wrHex(char *name,BYTE *hex,int hex_len);

void get_date8 (char *cDate);	//取8位日期，e.g 20060809
void get_time8(char *cTime)	;	//取8位时间，e.g 15:56:24
#endif // __WRLOG_H__
