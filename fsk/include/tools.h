#ifndef __TOOLS_H__
#define __TOOLS_H__

#include "cini.h"
#include <stdlib.h>

int read_string(TIni g_cfg,const char* domain,const char* name,char* value,int bufflen);
int read_int(TIni g_cfg,const char* domain,const char* name,int & value);

#endif

