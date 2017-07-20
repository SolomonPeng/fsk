#ifndef __ADAPTER_H__
#define __ADAPTER_H__
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "DataDefine.h"

class CAdapter
{
public:
	CAdapter(void);
	~CAdapter(void);
	static void asc_to_bcd(unsigned char *bcd_buf, unsigned char *ascii_buf, int conv_len, unsigned char type);
	static void bcd_to_asc(unsigned char *ascii_buf, unsigned char *bcd_buf, int conv_len, unsigned char type);
	static int tobyte (const char *instr, BYTE *outstr);
	static void byte_to_hex (const BYTE *in_buff, BYTE *out_buff, int len);
};
#endif // __TERM_ADAPTER_H__
