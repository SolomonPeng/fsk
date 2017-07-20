#include "adapter.h"

CAdapter::CAdapter(void)
{
}

CAdapter::~CAdapter(void)
{
}

void CAdapter::asc_to_bcd(unsigned char *bcd_buf, unsigned char *ascii_buf, int conv_len, unsigned char type)
{
	if (type==1)
	{
		memcpy(bcd_buf,ascii_buf,conv_len);
	}

}

void CAdapter::bcd_to_asc(unsigned char *ascii_buf, unsigned char *bcd_buf, int conv_len, unsigned char type)
{
	if (type==1)
	{
		memcpy(ascii_buf,bcd_buf,conv_len);
	}

}

int CAdapter::tobyte(const char *instr, BYTE *outstr)
{
	register int           i, j;
	register BYTE          ch;

	for (i = j = 0; instr[i]; i++)  {
		ch = instr[i];
		if (ch >= '0' && ch <= '9')
			ch -= '0';
		else
			if (ch >= 'a' && ch <= 'f')
				ch = ch - 'a' + 10;
			else
				if (ch >= 'A' && ch <= 'F')
					ch = ch - 'A' + 10;
				else
					ch = 0;

		if (i & 1)
			outstr[j++] += ch;
		else
			outstr[j] = ch << 4;
	}
	outstr[j] = 0;

	return (j);
}

void CAdapter::byte_to_hex (const BYTE *in_buff, BYTE *out_buff, int len)
{
	int   i, j;
	BYTE  ch;

	for (i = j = 0; i < len; i++)  {
		ch = (in_buff[i] >> 4) & 0x0f;
		out_buff[j++] = ch > 9 ? ch + 'a' - 10 : ch + '0';
		ch = in_buff[i] & 0x0f;
		out_buff[j++] = ch > 9 ? ch + 'a' - 10 : ch + '0';
	}
	out_buff[j] = 0;
}
