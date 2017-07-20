/*------------------------------------------------------------------------
**	Copyright (C) 2002 - 2005 GuangDong Tisson Techonlogy Co. Ltd.
**	File name:			Ciso8583.hpp
**	Version:			1.0
**	Date:				2005-08-03
**	Description:		封装iso8583打包解包的类
**	Others:       
**	Revision history:      
**		1:
**		Date: 
**		Author: 
**		Modifiacation: 
**		2：……
------------------------------------------------------------------------*/
//#include	<STRING>

#ifndef _Ciso8583_H
#define _Ciso8583_H

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "page.h"
#include "common.h"

using namespace std;

#define CLASS_DEBUG_LEVEL 1

const int HEX_BITMAP_LEN = 32;
const int P_CODE_POS = 3;
const int NMIC_POS = 70;
const int ON = 1;
const int OFF = 0;

//const char* const NULL_P_CODE = "XXXXXX";
const char* const NULL_P_CODE = "000000";	//lxd
const char* const NULL_NMIC = "XXX";
const char* const ZERO_P_CODE = "000000";
const char* const HANDLE_PARTY_GROUP = "PO";	//lxd
//const char* const HANDLE_PARTY_GROUP = "JF";
const char* const MTI_DEF_FILENAME = "mti.txt";
const char* const BIT_DEF_FILENAME = "bit_def_mv.txt";
//const char* const MECH_CODE_DEF_FILENAME = "mech_code.txt";

class Ciso8583
{

private:
	MTI_PAGE		mv_mti_page[NUM_MTI_PAGES];
	MTI_TABLE 	mv_mti_table[NUM_MTI_PAGES][NUM_BITS_128 + 1];
	BIT_TABLE 	mv_bit_table;
	MTI_PAGE		*mv_page_ptr;

	mech_table_stru	mv_mech_table[NUM_MECH_CODE];
	mac_table_stru	mv_mac_table[NUM_CB_PARTY];

	char	mv_packet[10240];

	int 	mf_init_mti_page(const char *pv_in_party_group); 
	int		mf_init_bit_def_tbl(BIT_DEF *pv_out_def_tbl);
	int		mf_init_mac_table();
	int		mf_init_mech_table();
	int		mf_extract_msg_identifier(char *hex_data,char *bitmap,char *nmic,char *mti_code,char *p_code,char *s_mti_code);
	int		mf_const_bitmap(MTI_PAGE *pv_in_page_ptr, int pv_in_num_bits, int pv_in_direction, char *pv_out_bitmap);
	int		mf_const_char_bitmap(char *pv_in_hex_buff, char *pv_out_bitmap);
	int		mf_validate_bitmap(char *pv_in_bit_map, int pv_in_flag);
	int		mf_set_mti_page_bit_on(char *pv_in_bitmap, int pv_in_flag);
	int		mf_decode_data(char *pv_in_hex_data, int pv_in_flag);
	
	int		mf_read_var_stru(PG_txn_stru &pv_in_var_stru);
	int		mf_write_stru(PG_txn_stru &pv_out_var_stru);

	
	MTI_PAGE *mf_search_mti_page(const char *prty,const char *mti_c, char *p_c, char *nm);

	char * mf_construct_mac_input(MTI_TABLE* pv_in_tbl,int pv_in_flag);	
	char * mf_cal_mac(char  *pv_in_mac_data,char *pv_in_mac_key);	
	void 	mf_display_mti_page(int pv_in_page_num);
	int		mf_display_bit_content(int pv_in_flag);
	int		mf_request_hdlr(const char* pv_in_mti_code, PG_txn_stru &pv_in_var_stru);
public:
	Ciso8583();
	//~Ciso8583();
	int		mf_init();
	int		mf_extra(char* pv_out_mti_code, PG_txn_stru &pv_out_var_stru,const char *pv_in_8583_packet);
	int		mf_const(const char* pv_in_mti_code, PG_txn_stru &pv_in_var_stru,char *pv_out_8583_packet);
	
	void	mf_display_stru(const PG_txn_stru &pv_in_var_stru);
	void	mf_init_var_stru(PG_txn_stru &pv_in_var_stru);
};


#endif _Ciso8583_H