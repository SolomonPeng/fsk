#include <iostream>
#include <string.h>
#include <ctype.h>
#include "Ciso8583.hpp"

BIT_DEF bit_def_tbl[NUM_BITS_128+1];

/*------------------------------------------------------------------------
**	Function:			Ciso8583
**	Description:	    构造函数	
**	Calls:				
**	Called By:			
**	Table Accessed:	    	
**	Table Updated:		
**	Input:
**	Output:				
**	Return:				
**	Others:				
------------------------------------------------------------------------*/
Ciso8583::Ciso8583()
{
	return;
}

/*------------------------------------------------------------------------
**	Function:			mf_init
**	Description:	    初始化函数	
**	Calls:				
**	Called By:			
**	Table Accessed:	    	
**	Table Updated:		
**	Input:
**	Output:				
**	Return:				
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_init()
{
	int ret;
	ret=mf_init_mti_page(HANDLE_PARTY_GROUP);
	if(ret!=0)
		return -1;

	ret=mf_init_bit_def_tbl(bit_def_tbl);
	if(ret!=0)
		return -1;
	
	ret=mf_init_mech_table();
	if(ret!=0)
		return -1;
        
	ret=mf_init_mac_table();
	if(ret!=0)
		return -1;
	printf("ISO8583 init Succ!\n");
	return 0;
}


/*------------------------------------------------------------------------
**	Function:			mf_extra
**	Description:	    解包函数	
**	Calls:				
**	Called By:			
**	Table Accessed:	    	
**	Table Updated:		
**	Input:
**	Output:				
**	Return:				
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_extra(char* pv_out_mti_code, PG_txn_stru &pv_out_var_stru,const char *pv_in_8583_packet)
{
	char	bit_map[NUM_BITS_128+1];
	int	ret;
	char	nmic[3+1], mti_code[4+1], p_code[6+1], s_mti_code[4+1];
	char	hex_buff[1024];
	char	mac_result[8+1];
	char	*mac_ptr;
	int	flag;  // 1 for request,  0 for rely.
	int	i;

	memset(hex_buff, 0, sizeof(hex_buff));
	strcpy(hex_buff, pv_in_8583_packet);
//	bin_to_hex_char(buffer, hex_buff, received_len);

#ifdef CLASS_DEBUG_LEVEL
//	cout << "packet received:\n" << hex_buff << endl;
#endif

	memset(bit_map, 0, sizeof(bit_map));
	mf_const_char_bitmap(hex_buff, bit_map);
#ifdef CLASS_DEBUG_LEVEL
	//cout << "BitMap:[" << bit_map << "]" << endl;
#endif

	// extract mti_code,p_code,nmic from packet.
	ret = mf_extract_msg_identifier(hex_buff, bit_map, nmic, mti_code, p_code, s_mti_code);
	if(ret < 0)
	{
#ifdef CLASS_DEBUG_LEVEL
		cout << "CISO_8583::mf_extract_8583() error: packet data format error!" << endl;
#endif
		return -1;
	}
#ifdef CLASS_DEBUG_LEVEL
	cout << "s_mti_code:[" << s_mti_code << "],  mti_code[" << mti_code << "],  nmic[" << nmic <<"],  p_code[" << p_code << "]" << endl;
#endif


	// search corresponding mti_page 
	mv_page_ptr = mf_search_mti_page(HANDLE_PARTY_GROUP, s_mti_code, p_code, nmic);
	if(mv_page_ptr == NULL)
	{
#ifdef CLASS_DEBUG_LEVEL
		cout << "CISO_8583::mf_extract_8583() error: resolve mti_page error!" << endl;
#endif
		return -2;
	}
#ifdef CLASS_DEBUG_LEVEL
//	cout << "mti_page obtained." << endl;
#endif


	switch(atoi(mti_code))
	{
	case 810:
	case 210:
	case 110:
	case 410:
	case 310:
	case 312:
		flag = 0;
		break;

	case 800:
	case 200:
	case 202:
	case 203:
	case 201:
	case 100:
	case 400:
	case 401:
	case 300:
	case 302:
		flag = 1;
		break;
	}

	// flag=0 for validating bitmap for b_i
	// flag=1 for validating bitmap for b_o
	ret = mf_validate_bitmap(bit_map, flag);
	if(ret > 0)
	{
#ifdef CLASS_DEBUG_LEVEL
		cout << "CISO_8583::mf_extract_8583() error: BIT[" << ret << "] mismatch!" << endl;
#endif
		return -3;
	}


	// 1 to set b_on_off_o,  0 to set b_on_off_i 
	mf_set_mti_page_bit_on(bit_map, flag);

	// Decode incoming packet and store in b_cont of mti_tbl
	ret = mf_decode_data(hex_buff, flag);
	if(ret < 0)
	{
#ifdef CLASS_DEBUG_LEVEL
		cout << "CISO_8583::mf_extract_8583() error: decoding incoming packet error!" << endl;
#endif
		return -4;
	}
	
	////////////////// construct the out_mti_code ///////////////////////////////////
	MTI_TABLE *tbl_ptr;
	tbl_ptr = mv_page_ptr->mti_tbl;
	
	if(mti_code[1]=='8'){
		strcpy(pv_out_mti_code, "8");
		strncat(pv_out_mti_code, tbl_ptr[70].b_cont+1,3);   //construct 8301,8201......
		memset(tbl_ptr[70].b_cont,0,sizeof(tbl_ptr[70].b_cont));
		strncpy(tbl_ptr[70].b_cont, pv_out_mti_code+1,3);      //reduce bit 70 len to 3
	}else{
		strcpy(	pv_out_mti_code, mti_code);
	}
	

#ifdef CLASS_DEBUG_LEVEL
	mf_display_bit_content(flag);
#endif


	////////////////////////// verify mac code ///////////////////////////////

	memset(mac_result, 0, sizeof(mac_result));
/*
	if( (!strncmp(mti_code, "0200", 4)) || (!strncmp(mti_code, "0210", 4)) ||
	    (!strncmp(mti_code, "0400", 4)) || (!strncmp(mti_code, "0401", 4)) ||
	    (!strncmp(mti_code, "0410", 4)) )
	{
		for(i=0; i<NUM_CB_PARTY; i++)
		{
			if(!strcmp(tbl_ptr[100].b_cont+2, mv_mac_table[i].party_code))
			{
				mac_ptr = mf_cal_mac( mf_construct_mac_input(tbl_ptr, flag), mv_mac_table[i].mac_key );
				byte_to_hex(mac_ptr, mac_result, 8);
				break;
			}
		}

		if(i >= NUM_CB_PARTY)
		{
#ifdef CLASS_DEBUG_LEVEL
			cout << "CISO_8583::mf_extract_8583() error: MAC KEY not found!" << endl;
#endif
			return -5;
		}

		if( (strncmp(tbl_ptr[128].b_cont, mac_result, sizeof(mac_result))) )
		{
#ifdef CLASS_DEBUG_LEVEL
			cout << "CISO_8583::mf_extract_8583() error: MAC mismatch!" << endl;
#endif
			return -6;
		}
	}
*/	
	mf_write_stru(pv_out_var_stru);


	return 0;
}

/*------------------------------------------------------------------------
**	Function:			mf_const
**	Description:	    打包函数	
**	Calls:				
**	Called By:			
**	Table Accessed:	    	
**	Table Updated:		
**	Input:
**	Output:				
**	Return:				
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_const(const char* pv_in_mti_code, PG_txn_stru &pv_in_var_stru,char *pv_out_8583_packet)
{
	char bit_map[NUM_BITS_128/4 + 1];
	char mac_result[16+1];
	char *mac_ptr;

	int i;
	int ret;
	int direction; //1 for request, 0 for reply.

	MTI_TABLE *tbl_ptr;

	direction = mf_request_hdlr(pv_in_mti_code, pv_in_var_stru);
	if(direction<0)
	{
#ifdef CLASS_DEBUG_LEVEL
		cout << "CISO_8583::mf_construct_8583() error: request_hdlr fail !" << endl;
#endif
		return -1;
	}

	tbl_ptr = mv_page_ptr->mti_tbl;
/*
	// Calculate MAC
	memset(mac_result, 0, sizeof(mac_result));
	if((!strncmp(pv_in_mti_code, "0200", 4)) || (!strncmp(pv_in_mti_code, "0400", 4)) || (!strncmp(pv_in_mti_code, "0401", 4)))
	{
		for(i=0; i<NUM_CB_PARTY; i++)
		{
			if(!strcmp(pv_in_var_stru.recv_party_code, mv_mac_table[i].party_code))
			{
				mac_ptr = mf_cal_mac( mf_construct_mac_input(tbl_ptr, direction), mv_mac_table[i].mac_key );
				byte_to_hex(mac_ptr, mac_result, 8);
				memset(tbl_ptr[128].b_cont, 0, sizeof(tbl_ptr[128].b_cont));
				strncpy(tbl_ptr[128].b_cont, mac_result, sizeof(mac_result));
				break;
			}
		}
		if(i >= NUM_CB_PARTY)
		{
#ifdef CLASS_DEBUG_LEVEL
			cout << "CISO_8583::mf_construct_8583() error: MAC KEY not found!" << endl;
#endif
			return -1;
		}
#ifdef CLASS_DEBUG_LEVEL
//		cout << "mac_data: " << tbl_ptr[128].b_cont << endl;
#endif
	}
*/
        ret = mf_const_bitmap(mv_page_ptr, NUM_BITS_128, direction, bit_map);
        if(ret < 0)
        {
#ifdef CLASS_DEBUG_LEVEL
                cout << "CISO_8583::mf_const_bitmap() error!" << endl;
#endif
                return -2;
        }
#ifdef CLASS_DEBUG_LEVEL
//	cout << "bit_map: [" << bit_map << "]" << endl;
#endif

	memset(mv_packet, 0, sizeof(mv_packet));
/*
	if( pv_in_mti_code[0] == '8' )
	{
		if(atoi(pv_in_mti_code)==8101 || atoi(pv_in_mti_code)==8201)
		{
			strcpy(mv_packet, "0800");
//			strcat(mv_packet, "--");
		}
		else
		{
			strcpy(mv_packet, "0810");
//			strcat(mv_packet, "--");
		}
	}
	else
	{
		strcpy(mv_packet, pv_in_mti_code);
//		strcat(mv_packet, "--");
	}
*/

	switch( atoi(pv_in_mti_code) )
	{
	case 100:
		strcpy(mv_packet, "0110");
		break;
	case 200:
		strcpy(mv_packet, "0210");
		break;
	case 400:
	case 401:
		strcpy(mv_packet, "0410");
		break;
	default:
		printf("error! unknow MTI\n");
		return -1;
	}

	strcat(mv_packet, bit_map);
//	strcat(mv_packet, "--");

	tbl_ptr = mv_page_ptr->mti_tbl;
        for( i=2; i<=NUM_BITS_128; i++)
	{
        	if (tbl_ptr[i].b_on_off_o)
		{
                	strcat(mv_packet, tbl_ptr[i].b_cont);
//			strcat(mv_packet, "--");
                }
        }

#ifdef CLASS_DEBUG_LEVEL
//	cout << "packet: [" << mv_packet << "]" << endl;
#endif
	mf_display_bit_content(1);
	strcpy(pv_out_8583_packet, mv_packet);

        return  0;
}


/*------------------------------------------------------------------------
**	Function:					mf_init_mti_page
**	Description:	    Fuction to init the mti page	
**	Calls:				
**	Called By:			
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						party group
**	Output:						nil
**	Return:						0: successful,  <0: fail.
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_init_mti_page(const char *pv_party_group)
{
	int i, j;
   
	FILE *fptr;

	char line[80 + 1];
	char *l_ptr;
	char party[3];
	char mti_def_filename[100];

	MTI_TABLE *t_ptr;
	MTI_PAGE *p_ptr;

	int valid_page = 1;  // default invalid
	char b_num[10], b_type[10], b_LLL[10], b_size[10], b_i[10], b_o[10];
	int bit_num;
	int page_no=0;

        // open the mti config file to read in
	memset(mti_def_filename, 0, sizeof(mti_def_filename));
	strcpy(mti_def_filename, PG_DEF_PATH);
	strcat(mti_def_filename, MTI_DEF_FILENAME);
	fptr = fopen(mti_def_filename, "r");
	if (fptr == NULL) {
#ifdef CLASS_DEBUG_LEVEL
		printf("Error open input file[%s]\n", mti_def_filename);
#endif
		return -1;
	}

        /* to clear the current value in mv_mti_page and mti_table */
        for (i=0; i<NUM_MTI_PAGES; i++) {
                memset(mv_mti_page[i].party, 0, 10);
                memset(mv_mti_page[i].mti_code, 0, 10);
                memset(mv_mti_page[i].p_code, 0, 10);
                memset(mv_mti_page[i].nmic, 0, 10);
                memset(mv_mti_page[i].rtn_mti, 0, 10);

                mv_mti_page[i].mti_tbl = mv_mti_table[i];
                t_ptr = mv_mti_page[i].mti_tbl;
      
                for (j=0; j<NUM_BITS+1; j++) {
                        t_ptr[j].b_on_off_i = 0;
                        t_ptr[j].b_on_off_o = 0;
                        t_ptr[j].b_type = 0;
                        t_ptr[j].b_LLL = 0;
                        t_ptr[j].b_size = 0;
                        t_ptr[j].b_i = '-';
                        t_ptr[j].b_o = '-';
                        t_ptr[j].b_cont = NULL;
                }
        }


        while ((feof(fptr) == 0) && (page_no <= NUM_MTI_PAGES)){
                if (fgets(line, sizeof(line), fptr) == NULL) {
                        if (ferror(fptr) != 0) {       //* File stream error detected /
                                printf("Error file input\n");
                                return -2;
                        }
                continue;
                }
      
                l_ptr = line;
                while (*l_ptr == ' ') {          //* Discard write space /
                        l_ptr++;
                }
                if ((*l_ptr == '#') || (*l_ptr == '\n')) {
                        continue;                      //* Discard comments or blank line /
                }

                if (*l_ptr == '!') {
                        l_ptr++;             //* replace the new record indicator - ! /
                        valid_page = 1;
                        sscanf(l_ptr, "%s", party);
                        if (!(strcmp(party, pv_party_group))) {
                                p_ptr = &mv_mti_page[page_no];
                                t_ptr = p_ptr->mti_tbl;
                                page_no++;           //* point to next page /
                                sscanf(l_ptr, "%s%s%s%s%s", p_ptr->party, p_ptr->mti_code, p_ptr->p_code, p_ptr->nmic, p_ptr->rtn_mti);
#ifdef CLASS_DEBUG_LEVEL           
 //                               printf("%s,%s,%s,%s,%s\n", p_ptr->party, p_ptr->mti_code, p_ptr->p_code, p_ptr->nmic, p_ptr->rtn_mti);
#endif
                                valid_page = 0;
                        }
                        continue;
                }
                if (valid_page == 0) {
                        sscanf(l_ptr, "%s%s%s%s%s%s", b_num, b_type, b_LLL, b_size, b_o, b_i);

                        bit_num=atoi(b_num);
                        t_ptr[bit_num].b_type = atoi(b_type);
                        t_ptr[bit_num].b_LLL = atoi(b_LLL);
                        t_ptr[bit_num].b_size = atoi(b_size);
                        t_ptr[bit_num].b_i = *b_i;
                        t_ptr[bit_num].b_o = *b_o;
                }
        }

        if (fclose(fptr)) {
                printf("file error: fclose()\n");
                return -4;
        }

        return 0;
}

/*------------------------------------------------------------------------
**	Function:					mf_init_bit_def_tbl
**	Description:	    Fuction to init the bit table	
**	Calls:				
**	Called By:			
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						define table name
**	Output:						out put define table
**	Return:						0: successful,  <0: fail.
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_init_bit_def_tbl(BIT_DEF *pv_out_def_tbl)
{
	char line_buff[80];
	char bit_def_filename[100];
	FILE *fptr;
	int  bit_num;
	char bit_num_str[10], type_str[10], LLL_str[10], len_str[10];
	int  i;

	memset(bit_def_filename, 0, sizeof(bit_def_filename));
	strcpy(bit_def_filename, PG_DEF_PATH);
	strcat(bit_def_filename, BIT_DEF_FILENAME);
	if ((fptr = fopen(bit_def_filename, "r")) == NULL)
	{
#ifdef CLASS_DEBUG_LEVEL
		cout << "Ciso8583::mf_init_bit_def_tbl() error: BIT DEF file open error!" << endl;
#endif
		return -1;
	}

	// reset bit definition table
	for (i=0; i <= NUM_BITS_128; i++)
	{
		pv_out_def_tbl[i].type = 0;
		pv_out_def_tbl[i].LLL = 0;
		pv_out_def_tbl[i].len = 0;
	}

	while (feof(fptr) == 0)
	{
		memset(line_buff, 0, sizeof(line_buff));
		memset(bit_num_str, 0, sizeof(bit_num_str));
		memset(type_str, 0, sizeof(type_str));
		memset(LLL_str, 0, sizeof(LLL_str));
		memset(len_str, 0, sizeof(len_str));

		if (fgets(line_buff, sizeof(line_buff), fptr) == NULL)
		{
			// File stream error detected
			if (ferror(fptr) != 0)
			{
#ifdef CLASS_DEBUG_LEVEL
				cout << "Ciso8583::mf_init_bit_def_tbl() error: BIT DEF file input!" << endl;
#endif
				return -2;
			}
			// the error is due to EOF
			continue;
		}
		if(line_buff[0] == '#')
			continue;

		sscanf(line_buff, "%s%s%s%s", bit_num_str, type_str, LLL_str, len_str);

		bit_num = atoi(bit_num_str);
		pv_out_def_tbl[bit_num].type = atoi(type_str);
		pv_out_def_tbl[bit_num].LLL = atoi(LLL_str);
		pv_out_def_tbl[bit_num].len = atoi(len_str);
#ifdef CLASS_DEBUG_LEVEL
//		cout << "bit[" << bit_num
//                     << "]:  type[" << pv_out_def_tbl[bit_num].type
//                     << "] ,  LLL[" << pv_out_def_tbl[bit_num].LLL
//                    << "] ,   len["  << pv_out_def_tbl[bit_num].len << "]" << endl;
#endif
	}
	if (fclose(fptr))
	{
#ifdef CLASS_DEBUG_LEVEL
		cout << "Ciso8583::mf_init_bit_def_tbl() error: close BIT DEF file error!" << endl;
#endif
		return -3;
	}

	return 0;
}


/*------------------------------------------------------------------------
**	Function:					mf_init_mac_table
**	Description:	    Fuction to init the mac table
**	Calls:				
**	Called By:			
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						nil
**	Output:						nil
**	Return:						0: successful,  <0: fail.
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_init_mac_table()
{
	return 0;
}

/*------------------------------------------------------------------------
**	Function:					mf_init_mech_table
**	Description:	    Fuction to init the mech code table 
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						nil
**	Output:						nil
**	Return:						0: successful,  <0: fail.
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_init_mech_table()
{
   	return 0;			
}

/*------------------------------------------------------------------------
**	Function:					mf_display_mti_page
**	Description:	    Fuction to display the content of  mti page
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						nil
**	Output:						nil
**	Return:						0: successful,  <0: fail.
**	Others:				
------------------------------------------------------------------------*/
void Ciso8583::mf_display_mti_page(int p_num)
{
   MTI_TABLE *t_ptr;
   int i;

   printf("_______________MTI PAGE [%d]________________\n", p_num);
   printf("[%s] [%s] [%s]", mv_mti_page[p_num].party, mv_mti_page[p_num].mti_code, mv_mti_page[p_num].p_code);
   printf(" [%s] [%s]\n", mv_mti_page[p_num].nmic, mv_mti_page[p_num].rtn_mti);

   t_ptr = mv_mti_page[p_num].mti_tbl;
   for (i=0; i<NUM_BITS + 1; i++) {
      printf("bit %3d: [%d] [%d]", i, t_ptr[i].b_on_off_i, t_ptr[i].b_on_off_o);
      printf(" [%d] [%3d]", t_ptr[i].b_type, t_ptr[i].b_LLL);
      printf(" [%3d] [%c]", t_ptr[i].b_size, t_ptr[i].b_i);
      printf(" [%c] [%d]\n", t_ptr[i].b_o, t_ptr[i].b_cont);
   }
}


/*------------------------------------------------------------------------
**	Function:					mf_display_bit_content
**	Description:	    Fuction to display the contents of bit table
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						nil
**	Output:						nil
**	Return:						0: successful,  <0: fail.
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_display_bit_content(int pv_in_flag)
{
	int i;
        MTI_TABLE *t_ptr;

        t_ptr = mv_page_ptr->mti_tbl;

        if (pv_in_flag)
	{
                printf("\t\tBIT CONTENT RECEIVED:\n");
                for(i=1; i<=NUM_BITS_128; i++) {
                        if (t_ptr[i].b_on_off_o) {
                                printf("BIT [%3d]: [%s]\n", i, t_ptr[i].b_cont);
                        }
                }
        }
        else {
                printf("\t\tBIT CONTENT TO BE SENT:\n");
                for(i=1; i<=NUM_BITS_128; i++) {
                        if (t_ptr[i].b_on_off_i) {
                                printf("BIT [%3d]: [%s]\n", i, t_ptr[i].b_cont);
                        }
                }
        }

        printf("\n");

	return 0;
}


/*------------------------------------------------------------------------
**	Function:					mf_search_mti_page
**	Description:	    Fuction to search the mti page
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						party group , mti code, p_code , nmic
**	Output:						nil
**	Return:						a pointer of the corrsponding mti page
**	Others:				
------------------------------------------------------------------------*/
MTI_PAGE *Ciso8583::mf_search_mti_page(const char *prty,const char *mti_c, char *p_c, char *nm)
{
    int i;
    MTI_TABLE *t_ptr;
  
    for(i=0; i<NUM_MTI_PAGES; i++) {
/*
      printf("\nparty[%d]=[%s],mti_code[%d]=[%s],p_code[%d]=[%s], nmic[%d]=[%s]",i,mv_mti_page[i].party,i,mv_mti_page[i].mti_code,i,
mv_mti_page[i].p_code,i,mv_mti_page[i].nmic);
        printf("\nparty [%s]",prty);
*/
        if (strcmp(mv_mti_page[i].party, prty)) {
         continue;
      }
      if (strcmp(mv_mti_page[i].mti_code, mti_c)) {
         continue;
      }
      if (strcmp(mv_mti_page[i].p_code, p_c)) {
         continue;
      }
      if (strcmp(mv_mti_page[i].nmic, nm)) {
         continue;
      }
  
        t_ptr= mv_mti_page[i].mti_tbl;

        // start to allocate to bit content
        memset(&mv_bit_table, 0, sizeof(BIT_TABLE));
      //  printf("\nNow in allocate_bit_content !");
   
        //for(i=0;i<128;i++)

   //     printf("\nAfter allocate bit content !");
        t_ptr[0].b_cont = mv_bit_table.b0;
        t_ptr[1].b_cont = mv_bit_table.b1;
        t_ptr[2].b_cont = mv_bit_table.b2;
        t_ptr[3].b_cont = mv_bit_table.b3;
        t_ptr[4].b_cont = mv_bit_table.b4;
        t_ptr[5].b_cont = mv_bit_table.b5;
        t_ptr[6].b_cont = mv_bit_table.b6;
        t_ptr[7].b_cont = mv_bit_table.b7;
        t_ptr[8].b_cont = mv_bit_table.b8;
        t_ptr[9].b_cont = mv_bit_table.b9;
        t_ptr[10].b_cont = mv_bit_table.b10;
  
   t_ptr[11].b_cont = mv_bit_table.b11;
   t_ptr[12].b_cont = mv_bit_table.b12;
   t_ptr[13].b_cont = mv_bit_table.b13;
   t_ptr[14].b_cont = mv_bit_table.b14;
   t_ptr[15].b_cont = mv_bit_table.b15;
   t_ptr[16].b_cont = mv_bit_table.b16;
   t_ptr[17].b_cont = mv_bit_table.b17;
   t_ptr[18].b_cont = mv_bit_table.b18;
   t_ptr[19].b_cont = mv_bit_table.b19;
   t_ptr[20].b_cont = mv_bit_table.b20;
   t_ptr[21].b_cont = mv_bit_table.b21;
   t_ptr[22].b_cont = mv_bit_table.b22;
   t_ptr[23].b_cont = mv_bit_table.b23;
   t_ptr[24].b_cont = mv_bit_table.b24;
   t_ptr[25].b_cont = mv_bit_table.b25;
   t_ptr[26].b_cont = mv_bit_table.b26;
   t_ptr[27].b_cont = mv_bit_table.b27;
   t_ptr[28].b_cont = mv_bit_table.b28;
   t_ptr[29].b_cont = mv_bit_table.b29;
   t_ptr[30].b_cont = mv_bit_table.b30;
   t_ptr[31].b_cont = mv_bit_table.b31;
   t_ptr[32].b_cont = mv_bit_table.b32;
   t_ptr[33].b_cont = mv_bit_table.b33;
   t_ptr[34].b_cont = mv_bit_table.b34;
   t_ptr[35].b_cont = mv_bit_table.b35;
   t_ptr[36].b_cont = mv_bit_table.b36;
   t_ptr[37].b_cont = mv_bit_table.b37;
   t_ptr[38].b_cont = mv_bit_table.b38;
   t_ptr[39].b_cont = mv_bit_table.b39;
   t_ptr[40].b_cont = mv_bit_table.b40;
   t_ptr[41].b_cont = mv_bit_table.b41;
   t_ptr[42].b_cont = mv_bit_table.b42;
   t_ptr[43].b_cont = mv_bit_table.b43;
   t_ptr[44].b_cont = mv_bit_table.b44;
   t_ptr[45].b_cont = mv_bit_table.b45;
   t_ptr[46].b_cont = mv_bit_table.b46;
   t_ptr[47].b_cont = mv_bit_table.b47;
   t_ptr[48].b_cont = mv_bit_table.b48;
   t_ptr[49].b_cont = mv_bit_table.b49;
   t_ptr[50].b_cont = mv_bit_table.b50;
   t_ptr[51].b_cont = mv_bit_table.b51;
   t_ptr[52].b_cont = mv_bit_table.b52;
   t_ptr[53].b_cont = mv_bit_table.b53;
   t_ptr[54].b_cont = mv_bit_table.b54;
   t_ptr[55].b_cont = mv_bit_table.b55;
   t_ptr[56].b_cont = mv_bit_table.b56;
   t_ptr[57].b_cont = mv_bit_table.b57;
   t_ptr[58].b_cont = mv_bit_table.b58;
   t_ptr[59].b_cont = mv_bit_table.b59;
   t_ptr[60].b_cont = mv_bit_table.b60;
   t_ptr[61].b_cont = mv_bit_table.b61;
   t_ptr[62].b_cont = mv_bit_table.b62;
   t_ptr[63].b_cont = mv_bit_table.b63;
   t_ptr[64].b_cont = mv_bit_table.b64;

   t_ptr[65].b_cont = mv_bit_table.b65;
   t_ptr[66].b_cont = mv_bit_table.b66;
   t_ptr[67].b_cont = mv_bit_table.b67;
   t_ptr[68].b_cont = mv_bit_table.b68;
   t_ptr[69].b_cont = mv_bit_table.b69;
   t_ptr[70].b_cont = mv_bit_table.b70;
   t_ptr[71].b_cont = mv_bit_table.b71;
   t_ptr[72].b_cont = mv_bit_table.b72;
   t_ptr[73].b_cont = mv_bit_table.b73;
   t_ptr[74].b_cont = mv_bit_table.b74;
   t_ptr[75].b_cont = mv_bit_table.b75;
   t_ptr[76].b_cont = mv_bit_table.b76;
   t_ptr[77].b_cont = mv_bit_table.b77;
   t_ptr[78].b_cont = mv_bit_table.b78;
   t_ptr[79].b_cont = mv_bit_table.b79;
   t_ptr[80].b_cont = mv_bit_table.b80;
   t_ptr[81].b_cont = mv_bit_table.b81;
   t_ptr[82].b_cont = mv_bit_table.b82;
   t_ptr[83].b_cont = mv_bit_table.b83;
   t_ptr[84].b_cont = mv_bit_table.b84;
   t_ptr[85].b_cont = mv_bit_table.b85;
   t_ptr[86].b_cont = mv_bit_table.b86;
   t_ptr[87].b_cont = mv_bit_table.b87;
   t_ptr[88].b_cont = mv_bit_table.b88;
   t_ptr[89].b_cont = mv_bit_table.b89;
   t_ptr[90].b_cont = mv_bit_table.b90;
   t_ptr[91].b_cont = mv_bit_table.b91;
   t_ptr[92].b_cont = mv_bit_table.b92;
   t_ptr[93].b_cont = mv_bit_table.b93;
   t_ptr[94].b_cont = mv_bit_table.b94;
   t_ptr[95].b_cont = mv_bit_table.b95;
   t_ptr[96].b_cont = mv_bit_table.b96;
   t_ptr[97].b_cont = mv_bit_table.b97;
   t_ptr[98].b_cont = mv_bit_table.b98;
   t_ptr[99].b_cont = mv_bit_table.b99;
   t_ptr[100].b_cont = mv_bit_table.b100;
   t_ptr[101].b_cont = mv_bit_table.b101;
   t_ptr[102].b_cont = mv_bit_table.b102;
   t_ptr[103].b_cont = mv_bit_table.b103;
   t_ptr[104].b_cont = mv_bit_table.b104;
   t_ptr[105].b_cont = mv_bit_table.b105;
   t_ptr[106].b_cont = mv_bit_table.b106;
   t_ptr[107].b_cont = mv_bit_table.b107;
   t_ptr[108].b_cont = mv_bit_table.b108;
   t_ptr[109].b_cont = mv_bit_table.b109;
   t_ptr[110].b_cont = mv_bit_table.b110;
   t_ptr[111].b_cont = mv_bit_table.b111;
   t_ptr[112].b_cont = mv_bit_table.b112;
   t_ptr[113].b_cont = mv_bit_table.b113;
   t_ptr[114].b_cont = mv_bit_table.b114;
   t_ptr[115].b_cont = mv_bit_table.b115;
   t_ptr[116].b_cont = mv_bit_table.b116;
   t_ptr[117].b_cont = mv_bit_table.b117;
   t_ptr[118].b_cont = mv_bit_table.b118;
   t_ptr[119].b_cont = mv_bit_table.b119;
   t_ptr[120].b_cont = mv_bit_table.b120;
   t_ptr[121].b_cont = mv_bit_table.b121;
   t_ptr[122].b_cont = mv_bit_table.b122;
   t_ptr[123].b_cont = mv_bit_table.b123;
   t_ptr[124].b_cont = mv_bit_table.b124;
   t_ptr[125].b_cont = mv_bit_table.b125;
   t_ptr[126].b_cont = mv_bit_table.b126;
   t_ptr[127].b_cont = mv_bit_table.b127;
   t_ptr[128].b_cont = mv_bit_table.b128;
   
//      printf("\nbefore return(&mti_p[%d])\n",i);
			mv_page_ptr = &mv_mti_page[i];
		//	mf_display_bit_content(2);
      return(&mv_mti_page[i]);
   }
   return NULL;
}

/*------------------------------------------------------------------------
**	Function:					mf_extract_msg_identifier
**	Description:	    Fuction to extract message identifier. 
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_extract_msg_identifier(char *hex_data,char *bitmap,char *nmic,char *mti_code,char *p_code,char *s_mti_code)
{
	char *h_data_ptr;
	char LLL_char[5];
	int cur_bit_len,cur_LLL_len;
	int i,upto_bit_pos;

 	/* 0 for finacial transaction */
	/* 1 for non fincial transaction */
	int trans_flag;

	h_data_ptr = hex_data;
	mti_code[0] = '\0';
	nmic[0] = '\0';
	p_code[0] = '\0';

	strncat(mti_code,h_data_ptr,4);

	/* skip mti_code,and the first 64-bit bitmap */
	h_data_ptr = h_data_ptr + 4 + 16;

//	printf("Resolved mti:[%s]\n",mti_code);

	/* stream FINACIAL/NON FINACIAL transaction */
	switch (atoi(mti_code)) {
	case 200:
	case 100:
	case 300:
	case 400:
	case 312:
	case 401:
		upto_bit_pos = P_CODE_POS;
		trans_flag = 0;
		break;
	case 800:
	case 810:
		upto_bit_pos = NMIC_POS;
		trans_flag = 1;
		break;
	default:
		fprintf(stderr,"Message received error\n");
		return 1;
	}

	/* For bit i,bitmap position is    i-1; */
	/*            bit_def position is   i    */ 
//	printf("upto_bit_pos:[%d]\n",upto_bit_pos);
	for (i=1; i < upto_bit_pos; i++) {
		if (bitmap[i-1]=='0') {
			continue;
		}
		/* bit_def_tbl must be set for existing bit */
		if (bit_def_tbl[i].type == 0){
			/*printf("bit[%d] bitmap[%c],type[%d]\n",i,bitmap[i-1],bit_def_tbl[i].type);*/
			fprintf(stderr,"Incoming data incomsistent with standard\n");
			return 2;
		}

		/* handle bit with fixed length */
		if (bit_def_tbl[i].LLL == 0) {
			switch(bit_def_tbl[i].type) {
			/* data type: N */
			case 1:
			case 4:
				cur_bit_len = bit_def_tbl[i].len;
				/* the length is odd,0 inserted in the front */
				if (bit_def_tbl[i].len % 2) {
					cur_bit_len++;
				}
				break;
				
			/* data type: AN,ANS */
			case 2:
			case 3:
				cur_bit_len = bit_def_tbl[i].len * 2;
				break;

			/* data type: B */
			case 5:
				cur_bit_len = bit_def_tbl[i].len / 4;
				break;
			}

			/* skip the current bit field */
			h_data_ptr = h_data_ptr + cur_bit_len;
/*
			printf("data:[%s]\n",h_data_ptr);
*/
		}

		/* Handle bit with variable length */
		else if (bitmap[i-1] == '1') {
			memset(LLL_char,0,sizeof(LLL_char));
			LLL_char[0] = '\0';

			switch (bit_def_tbl[i].LLL) {
			case 1:
			case 2:
				cur_LLL_len = 2;
				break;
			case 3:
			case 4:
				cur_LLL_len = 4;
				break;

			default:
				fprintf(stderr,"Error initialized bit_def_tbl\n");
				return 4;
			}

			strncat(LLL_char,h_data_ptr,cur_LLL_len);
			/* skip bit LLL */
/*
			printf("data to[%s]\n",h_data_ptr);
*/
			h_data_ptr = h_data_ptr + cur_LLL_len;
			cur_bit_len = atoi(LLL_char);

/*
			printf("Resolving bit :[%d]\n",i);
*/
			if (cur_bit_len > bit_def_tbl[i].len) {
				fprintf(stderr,"Data inconsistent with bit_def_tbl\n");
				/*printf("bit[%d]:cur_bit_len:[%d],def_bit_len:[%d]\n",i,cur_bit_len,bit_def_tbl[i].len);*/
				return 5;
			}
			switch(bit_def_tbl[i].type) {
			/* data type: N */
			case 1:
			case 4:
				/* the length is odd,0 inserted in the front */
				if (cur_bit_len % 2) {
					cur_bit_len++;
				}
				break;
				
			/* data type: AN,ANS */
			case 2:
			case 3:
				cur_bit_len = cur_bit_len * 2;
				break;

			/* data type: B */
			case 5:
				if (cur_bit_len % 8) {
					fprintf(stderr,"Data size error\n");
					return 5;
				}
				cur_bit_len = cur_bit_len / 4;
				break;
			}

			/* skip the current bit field */
			h_data_ptr = h_data_ptr + cur_bit_len;
/*
			printf("data:[%s]\n",h_data_ptr);
*/
		}
	}

	/* Non finacial transaction */
	if (trans_flag) {
		/* skip 0 add in front of nmic */
		h_data_ptr = h_data_ptr + 1;
		strncat(nmic,h_data_ptr,bit_def_tbl[upto_bit_pos].len);
		strcpy(p_code,NULL_P_CODE);
	}
	/* Finacial transaction */
	else {
		strncat(p_code,"000000",6);		//lxd
		//strncat(p_code,"XXXXXX",6);
		/*strncat(p_code,h_data_ptr,bit_def_tbl[upto_bit_pos].len);*/
		strcpy(nmic,NULL_NMIC);
	}

	/* set s_mti_code for searching mti_page */
	switch (atoi(mti_code)) {
	
	case 100:
		strcpy(s_mti_code,"0100");
		break;
	
	case 200:
		strcpy(s_mti_code,"0200");
		break;

	case 300:
		strcpy(s_mti_code,"0300");
		break;
	
	case 312:
		strcpy(s_mti_code,"0302");
		break;
	
	case 400:
		strcpy(s_mti_code,"0400");
		break;
	
	case 401:
		strcpy(s_mti_code,"0401");
		break;

	case 810:
		strcpy(s_mti_code,"0800");
		break;

	case 800:
		strcpy(s_mti_code,"0800");
	}
	
	return 0;

}

/*------------------------------------------------------------------------
**	Function:					mf_const_bitmap
**	Description:	    Fuction to construct the bit map so as the make the 8583 packet
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						pointer of mti page, num bits(64 or 128), direction ( in or out )
**	Output:						bit map
**	Return:						0:successful , <0: fail.
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_const_bitmap(MTI_PAGE *pv_in_page_ptr, int pv_in_num_bits, int pv_in_direction, char *pv_out_bitmap)
{
	// bit map in hex charactor
//        static char bitmap[NUM_BITS_128/4 + 1];
        MTI_TABLE *tbl_ptr;
        MTI_TABLE *cur_ptr;
        char temp_char[2];
        int bitmap_len;
        int pos_val, digit_val;
        int i, j, base;

        tbl_ptr = pv_in_page_ptr->mti_tbl;

        if (pv_in_num_bits%8)
	{
#ifdef CLASS_DEBUG_LEVEL
                cout << "CISO_8583::mf_const_bitmap() error: number of bits: " << pv_in_num_bits << " is not correct!" << endl;
#endif
                return -1;
        }

        bitmap_len = pv_in_num_bits/4;
        for (i=0; i<bitmap_len; i++)
	{
                digit_val = 0;
                base = i * 4;
                pos_val = 8;

                for (j=0; j<4; j++)
		{
                        /* handle bit 1, extended bit map */
                        if ((i==0) && (j==0))
			{
                                digit_val = digit_val+pos_val;
                                pos_val=pos_val/2;
                                continue;
                        }

                        /* bit number start from 1 */
                        cur_ptr = tbl_ptr + (base+j+1);

                        /* construct bit map for request */
                        if (pv_in_direction)
			{
                                switch ((int)cur_ptr->b_o)
				{
                                case (int)'M':
                                        /* ensure the bit is filled */
                                        if (strlen(cur_ptr->b_cont)>0)
					{
                                                cur_ptr->b_on_off_o=1;
                                                digit_val=digit_val+pos_val;
                                        }
                                        else
					{
#ifdef CLASS_DEBUG_LEVEL
                                                cout << "CISO_8583::mf_const_bitmap() error: BIT " << (base+j+1) << " content missed!" << endl;
#endif
                                                return -1;
                                        }
                                        break;

                                case (int)'C':
                                        if (strlen(cur_ptr->b_cont)>0)
					{
                                                cur_ptr->b_on_off_o=1;
                                                digit_val=digit_val+pos_val;
                                        }
                                        break;

                                default:
                                        cur_ptr->b_on_off_o=0;
                                }
                        }
                        /* construct bit_map for reply */
                        else {
                                switch ((int)cur_ptr->b_i)
				{
                                case (int)'M':
                                        /* ensure the bit is filled */
                                        if (strlen(cur_ptr->b_cont)>0)
					{
                                                cur_ptr->b_on_off_o=1;
                                                digit_val=digit_val+pos_val;
                                        }
                                        else
					{
#ifdef CLASS_DEBUG_LEVEL
                                      cout << "CISO_8583::mf_const_bitmap() error: BIT " << (base+j+1) << " content missed" << endl;
#endif
                                                return -1;
                                        }
                                        break;

                                case (int)'C':
                                        if (strlen(cur_ptr->b_cont)>0)
					{
                                                cur_ptr->b_on_off_o=1;
                                                digit_val=digit_val+pos_val;
                                        }
                                        break;

                                default:
                                        cur_ptr->b_on_off_o=0;
                                }
                        }

                        pos_val=pos_val/2;
                }
                sprintf(temp_char, "%x", digit_val);
                pv_out_bitmap[i]=temp_char[0];
        }
        pv_out_bitmap[i]='\0';
//printf("bitmap[0]=[%c](Char)\n", bitmap[0]);
//printf("bitmap[0]=[%x](Hex)\n", bitmap[0]);
//printf("bitmap=[%s]\n", bitmap);

        return 0;
}


/*------------------------------------------------------------------------
**	Function:					mf_const_char_bitmap
**	Description:	    
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_const_char_bitmap(char *pv_in_hex_buff, char *pv_out_bitmap)
{
	int i;
	char *h_data_ptr;
	char tmp_bitmap[5];
	char value[5];

	// Array for holding bitmap in hex charactor
	char hex_bitmap[HEX_BITMAP_LEN + 1];

	h_data_ptr = pv_in_hex_buff;

	// skip mti_code (N 4)
	h_data_ptr = h_data_ptr + 4;

	memset(hex_bitmap, 0, sizeof(hex_bitmap));
	strncat(hex_bitmap, h_data_ptr, HEX_BITMAP_LEN);

	for(i=0; i<strlen(hex_bitmap); i++)
	{
		memset(value,0,sizeof(value));

		switch(hex_bitmap[i])
		{
		case '1' :
			strncpy(value,"0001",4);
			break;
		case '2' :
			strncpy(value,"0010",4);
			break;
		case '3' :
			strncpy(value,"0011",4);
			break;
		case '4' :
			strncpy(value,"0100",4);
			break;
		case '5' :
			strncpy(value,"0101",4);
			break;
		case '6' :
			strncpy(value,"0110",4);
			break;
		case '7' :
			strncpy(value,"0111",4);
			break;
		case '8' :
			strncpy(value,"1000",4);
			break;
		case '9' :
			strncpy(value,"1001",4);
			break;
		case 'a' :
			strncpy(value,"1010",4);
			break;
		case 'b' :
			strncpy(value,"1011",4);
			break;
		case 'c' :
			strncpy(value,"1100",4);
			break;
		case 'd' :
			strncpy(value,"1101",4);
			break;
		case 'e' :
			strncpy(value,"1110",4);
			break;
		case 'f' :
			strncpy(value,"1111",4);
			break;
		default  :
			strncpy(value,"0000",4);
		}

		strncat(pv_out_bitmap, value, 4);
	}

	return 0;
}

/*------------------------------------------------------------------------
**	Function:					mf_validate_bitmap
**	Description:	    Fuction to validate bitmap
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_validate_bitmap(char *pv_in_bitmap, int pv_in_flag)
{
        int i;
        MTI_TABLE *tbl_ptr;

        // ON if the bit is set, OFF if the bit is not set
        int bitmap_on_off, mtitbl_on_off;

        tbl_ptr = mv_page_ptr->mti_tbl;

        if (pv_in_flag)
	{
                for (i=0; i<NUM_BITS_128; i++)
		{
                        if (pv_in_bitmap[i] == '1')
			{
                                if ((tbl_ptr[i+1].b_o != 'M') && (tbl_ptr[i+1].b_o != 'C'))
				{
#ifdef CLASS_DEBUG_LEVEL
				cout << "bit[" << i+1 << "], bitmap[" << pv_in_bitmap[i]
                                     << "], b_o[" << tbl_ptr[i+1].b_o << "]" << endl;
#endif
				return(i+1);
				}
			}
			if (tbl_ptr[i+1].b_o == 'M')
			{
				if (pv_in_bitmap[i] != '1')
				{
#ifdef CLASS_DEBUG_LEVEL
					cout << "bitmap[" << pv_in_bitmap[i] << "], b_o[" << tbl_ptr[i+1].b_o << "]" << endl;
#endif
					return(i+1);
				}
			}
		}
		return 0;
	}

	for (i=0; i<NUM_BITS_128; i++)
	{
		if (pv_in_bitmap[i] == '1')
		{
			if ((tbl_ptr[i+1].b_i != 'M') && (tbl_ptr[i+1].b_i != 'C'))
			{
#ifdef CLASS_DEBUG_LEVEL
				cout << "bitmap[" << pv_in_bitmap[i] << "], b_i[" << tbl_ptr[i+1].b_i << "]" << endl;
#endif
				return(i+1);
			}
		}
		if (tbl_ptr[i+1].b_i == 'M')
		{
			if (pv_in_bitmap[i] != '1')
			{
#ifdef CLASS_DEBUG_LEVEL
				cout << "bitmap[" << pv_in_bitmap[i] << "], b_i[" << tbl_ptr[i+1].b_i << "]" << endl;
#endif
				return(i+1);
			}
		}
	}
	return 0;
}

/*------------------------------------------------------------------------
**	Function:					mf_set_mti_page_bit_on
**	Description:	    
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_set_mti_page_bit_on(char *pv_in_bitmap, int pv_in_flag)
{
	int i;
	MTI_TABLE *tbl_ptr;

	// REVISE IF IT IS NEEDED TO STORE INCOMING BITMAP
	// set bit[0], the first 32-bit bit map
	tbl_ptr = mv_page_ptr->mti_tbl;
	tbl_ptr[0].b_on_off_i = ON;
	tbl_ptr[0].b_cont = mv_bit_table.b0;

	// for bit i, bitmap position is    i-1
	//            bit_table position is i  
	if (pv_in_flag)
	{
		for (i=0; i < NUM_BITS_128; i++)
		{
			if (pv_in_bitmap[i] == '1')
			{
				tbl_ptr[i+1].b_on_off_o = ON;
			}
			else
			{
				tbl_ptr[i+1].b_on_off_o = OFF;
			}
		}
		return 0;
	}

	/* this might not be valid  as no bitmap for output*/
	for (i=0; i < NUM_BITS_128; i++)
	{
		if (pv_in_bitmap[i] == '1')
		{
			tbl_ptr[i+1].b_on_off_i = ON;
		}
		else
		{
			tbl_ptr[i+1].b_on_off_i = OFF;
		}
	}
	return 0;
}


/*------------------------------------------------------------------------
**	Function:					mf_decode_data
**	Description:	    
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_decode_data(char *pv_in_hex_data, int pv_in_flag)
{
	int i;
	int sub_size, fixed_size, LLL_size, var_len;
	char temp_size[5];
	char *recv_data;

	MTI_TABLE *tbl_ptr;

	recv_data = pv_in_hex_data;
	
	// skip mti_code, and the first 64-bit bitmap
	recv_data = recv_data+4+16;
	tbl_ptr = mv_page_ptr->mti_tbl;

	////////////////////////// decode in message ///////////////////////////////
	if(pv_in_flag == 0)
	for(i=1; i<=NUM_BITS_128; i++)
	{
		if (tbl_ptr[i].b_on_off_i==OFF)
			continue;

		// cope with variable length fields
		if(tbl_ptr[i].b_LLL>0)
		{
			memset(temp_size, 0, sizeof(temp_size));
			LLL_size = 0;

			switch (tbl_ptr[i].b_LLL)
			{
			case 1:
			case 2:
				LLL_size = 2;
				break;
			case 3:
			case 4:
				LLL_size = 4;
				break;
			default:
#ifdef CLASS_DEBUG_LEVEL
				cout << "CISO_8583::mf_decode_data() error: mti_tbl[" << i << "].b_LLL error!" << endl;
#endif
				return -1;
			}
			strncat(temp_size, recv_data, LLL_size);
			var_len = atoi(temp_size);

			// verify variable length
			if (var_len > tbl_ptr[i].b_size)
			{
#ifdef CLASS_DEBUG_LEVEL
//				cout << "var_len: " << var_len << "     ";
//				cout << "bit_size: " << tbl_ptr[i].b_size << endl;
//				cout << "CISO_8583::mf_decode_data() error: BIT[" << i << "] LLL error!" << endl;
#endif
				continue;
			}

			switch (tbl_ptr[i].b_type)
			{
			/* data type N, Z */
			case 1:
			case 4:
				// check if variable length is odd
				if (var_len%2)
				{
					var_len = var_len + 1;
				}
				var_len = var_len + LLL_size;
				memset(tbl_ptr[i].b_cont, 0, var_len+1);
				memcpy(tbl_ptr[i].b_cont, recv_data, var_len);
				recv_data += var_len;
				break;

			// data type AN, ANS
			case 2:
			case 3:
				var_len = var_len*2 + LLL_size;
				memset(tbl_ptr[i].b_cont, 0, var_len+1);
				memcpy(tbl_ptr[i].b_cont, recv_data, var_len);
				recv_data += var_len;
				break;
			case 5:
				var_len = var_len/4 + LLL_size;
				memset(tbl_ptr[i].b_cont, 0, var_len+1);
				memcpy(tbl_ptr[i].b_cont, recv_data, var_len);
				recv_data += var_len;
				break;
			default:
#ifdef CLASS_DEBUG_LEVEL
				cout << "CISO_8583::mf_decode_data() error: BIT[" << i << "] data type error!" << endl;
#endif
				return -2;
			}
			continue;
		}

		// cope with fixed length field
		switch (tbl_ptr[i].b_type)
		{
		// data type N, Z
		case 1:
		case 4:
			fixed_size = tbl_ptr[i].b_size;
			// check if length is odd
			if (fixed_size % 2)
			{
				fixed_size = fixed_size + 1;
			}
			memset(tbl_ptr[i].b_cont, 0, fixed_size+1);
			memcpy(tbl_ptr[i].b_cont, recv_data, fixed_size);
			recv_data += fixed_size;
			break;

		// data type AN, ANS
		case 2:
		case 3:
			fixed_size = tbl_ptr[i].b_size * 2;
			memset(tbl_ptr[i].b_cont, 0, fixed_size+1);
			memcpy(tbl_ptr[i].b_cont, recv_data, fixed_size);
			recv_data += fixed_size;
			break;

		// data type B
		case 5:
			fixed_size = tbl_ptr[i].b_size/4;
			memset(tbl_ptr[i].b_cont, 0, fixed_size+1);
			memcpy(tbl_ptr[i].b_cont, recv_data, fixed_size);
			recv_data += fixed_size;
			break;

		default:
#ifdef CLASS_DEBUG_LEVEL
			cout << "CISO_8583::mf_decode_data() error: BIT[" << i << "] data type error!    ";
			cout << "Error b_type: " << tbl_ptr[i].b_type << endl;
#endif
			return -3;
		}
	}

	////////////////////////// decode out message ///////////////////////////////
        if(pv_in_flag == 1)
        for(i=1; i<=NUM_BITS_128; i++)
        {
                if (tbl_ptr[i].b_on_off_o==OFF)
                        continue;

                // cope with variable length fields
                if(tbl_ptr[i].b_LLL>0)
                {
                        memset(temp_size, 0, sizeof(temp_size));
                        LLL_size = 0;

                        switch (tbl_ptr[i].b_LLL)
                        {
                        case 1:
                        case 2:
                                LLL_size = 2;
                                break;
                        case 3:
                        case 4:
                                LLL_size = 4;
                                break;
                        default:
#ifdef CLASS_DEBUG_LEVEL
                                cout << "CISO_8583::mf_decode_data() error: mti_tbl[" << i << "].b_LLL error!" << endl;
#endif
                                return -1;
                        }
                        strncat(temp_size, recv_data, LLL_size);
                        var_len = atoi(temp_size);

                        // verify variable length
                        if (var_len > tbl_ptr[i].b_size)
                        {
#ifdef CLASS_DEBUG_LEVEL
 //                               cout << "var_len: " << var_len << "     ";
 //                               cout << "bit_size: " << tbl_ptr[i].b_size << endl;
 //                              cout << "CISO_8583::mf_decode_data() error: BIT[" << i << "] LLL error!" << endl;
#endif
                                continue;
                        }

                        switch (tbl_ptr[i].b_type)
                        {
                        /* data type N, Z */
                        case 1:
                        case 4:
                                // check if variable length is odd
                                if (var_len%2)
                                {
                                        var_len = var_len + 1;
                                }
                                var_len = var_len + LLL_size;
                                memset(tbl_ptr[i].b_cont, 0, var_len+1);
                                memcpy(tbl_ptr[i].b_cont, recv_data, var_len);
                                recv_data += var_len;
                                break;

                        // data type AN, ANS
                        case 2:
                        case 3:
                                var_len = var_len*2 + LLL_size;
                                memset(tbl_ptr[i].b_cont, 0, var_len+1);
                                memcpy(tbl_ptr[i].b_cont, recv_data, var_len);
                                recv_data += var_len;
                                break;
                        case 5:
                                var_len = var_len/4 + LLL_size;
                                memset(tbl_ptr[i].b_cont, 0, var_len+1);
                                memcpy(tbl_ptr[i].b_cont, recv_data, var_len);
                                recv_data += var_len;
                                break;
                        default:
#ifdef CLASS_DEBUG_LEVEL
                                cout << "CISO_8583::mf_decode_data() error: BIT[" << i << "] data type error!" << endl;
#endif
                                return -2;
                        }
                        continue;
                }

                // cope with fixed length field
                switch (tbl_ptr[i].b_type)
                {
                // data type N, Z
                case 1:
                case 4:
                        fixed_size = tbl_ptr[i].b_size;
                        // check if length is odd
                        if (fixed_size % 2)
                        {
                                fixed_size = fixed_size + 1;
                        }
                        memset(tbl_ptr[i].b_cont, 0, fixed_size+1);
                        memcpy(tbl_ptr[i].b_cont, recv_data, fixed_size);
                        recv_data += fixed_size;
                        break;

                // data type AN, ANS
                case 2:
                case 3:
                        fixed_size = tbl_ptr[i].b_size * 2;
                        memset(tbl_ptr[i].b_cont, 0, fixed_size+1);
                        memcpy(tbl_ptr[i].b_cont, recv_data, fixed_size);
                        recv_data += fixed_size;
                        break;

                // data type B
                case 5:
                        fixed_size = tbl_ptr[i].b_size/4;
                        memset(tbl_ptr[i].b_cont, 0, fixed_size+1);
                        memcpy(tbl_ptr[i].b_cont, recv_data, fixed_size);
                        recv_data += fixed_size;
                        break;

                default:
#ifdef CLASS_DEBUG_LEVEL
                        cout << "CISO_8583::mf_decode_data() error: BIT[" << i << "] data type error!    ";
                        cout << "Error b_type: " << tbl_ptr[i].b_type << endl;
#endif
                        return -3;
                }
        }

	return 0;

}


/*------------------------------------------------------------------------
**	Function:					mf_cal_mac
**	Description:	    Fuction to calculate the mac with the input mac key  
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						data to be calculated, mac key
**	Output:						
**	Return:						the result mac 
**	Others:				
------------------------------------------------------------------------*/
char *Ciso8583::mf_cal_mac(char  *pv_in_mac_data,char *pv_in_mac_key)
{
        int n,i,index,sum,mac_length,key_length;
        char temp_char,tmp_data_char[2],tmp_key_char[2];
        short tmp_data,tmp_key,tmp_mac;
        char mac_data[MAX_MAC_LENGTH],mac_key[MAX_KEY_LENGTH];
        static char result_mac[MAX_KEY_LENGTH+1];

        /* Get the Input mac data & cal. its Mac length*/
        memset(result_mac, 0, sizeof(result_mac));
        mac_length=strlen(pv_in_mac_data);
        key_length=strlen(pv_in_mac_key);
        index=(int)(mac_length-1)/8;
        strncpy(mac_data,pv_in_mac_data,mac_length);
        strncpy(mac_key,pv_in_mac_key,key_length);

        /* Do Calculation here */
        for (n=0;n<8;n++)
        {
                sum=0;
                for (i=0;i<=index;i++)
                {
                        /* get value from the Field of MAC_DATA*/
                        if ((8*i+n)>=mac_length)
                                tmp_data=0;
                        else
                        {
                                memset(tmp_data_char,0,sizeof(tmp_data_char));
                                tmp_data_char[0]=mac_data[8*i+n];

                                /* assign 1 to all alpha char */
                                if (isalpha(tmp_data_char[0]))
                                        tmp_data=1;
                                else
                                /* convert to int if the char is decimal*/
                                        tmp_data=atoi(tmp_data_char);
                        }
                        /* Get value from the Key */
                        memset(tmp_key_char,0,sizeof(tmp_key_char));
                        tmp_key_char[0]=mac_key[8*(i%2)+n];
                        if (isalpha(tmp_key_char[0])) {
                                tmp_key=1;
                        }
                        else {
                                tmp_key=atoi(tmp_key_char);
                        }

                        sum=sum+tmp_data*tmp_key;
                }

                /* Calc the Mac data & put into result_mac */
                tmp_mac=sum%10;
                sprintf(&temp_char,"%d",tmp_mac);
                result_mac[n]= temp_char;
        }/* for */

        /* return caculated MAC which is defined as string */
        cout<<"MAC result:"<<result_mac<<"\n";
        return result_mac;
}



/*------------------------------------------------------------------------
**	Function:					mf_construct_mac_input
**	Description:	    Fuction to generate data ,bitmap for ooutput file to PEJ 
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						tbl - a pointer to data sturct MTI_TABLE
**										flag - 0 to verify mac for incoming data, b_on_off_i set
**													 1 to verify mac for outing data, b_on_off_o set
**	Output:						
**	Return:						a charactor pointer to involved bits
**										select involved bit, 2, 4, 32, 37, 39, 41, 100
**	Others:				
------------------------------------------------------------------------*/
char* Ciso8583::mf_construct_mac_input(MTI_TABLE* pv_in_tbl,int pv_in_flag)
{
        static char mac_d[300+1];
        char LLL_bit_len[5];
        char LLL_bit_cont[100];
        char ANS_bit_cont[100];
        char *bit_ptr;
        int i;
        int ANS_len;

        memset(mac_d, 0, sizeof(mac_d));

        if (pv_in_flag==1) {
                for(i=2;i<NUM_BITS_128;i++) {
                        memset(LLL_bit_len, 0, sizeof(LLL_bit_len));
                        memset(LLL_bit_cont, 0, sizeof(LLL_bit_cont));

                        if((pv_in_tbl[i].b_o=='M')||(pv_in_tbl[i].b_o=='C')){
                                switch (i) {
                                /* data type LLN */
                                case 2:
                                case 32:
                                case 100:
                                        bit_ptr = pv_in_tbl[i].b_cont;
                                        strncpy(LLL_bit_len, bit_ptr, 2);
                                        bit_ptr=bit_ptr + 2;
                                        strncpy(LLL_bit_cont, bit_ptr, atoi(LLL_bit_len));
                                        strcat(mac_d, LLL_bit_cont);
                                        break;

                                /* data type N */
                                case 4:
                                        strcat(mac_d, pv_in_tbl[i].b_cont);
                                        break;


                                /* data type ANS, AN */
                                case 37:
                                case 39:
                                case 41:
                                        memset(ANS_bit_cont, 0, sizeof(ANS_bit_cont));
                                        ANS_len = tobyte(pv_in_tbl[i].b_cont, ANS_bit_cont);
                                        strncat(mac_d, ANS_bit_cont, ANS_len);
                                        break;

                                default:
                                        break;
                                }
                        }
                }
                cout<<" MAC INPUT DATA:"<<mac_d<<"\n";
                return mac_d;
        }

        for(i=2;i<NUM_BITS_128;i++) {
                memset(LLL_bit_len, 0, sizeof(LLL_bit_len));
                memset(LLL_bit_cont, 0, sizeof(LLL_bit_cont));

                if((pv_in_tbl[i].b_i=='M')||(pv_in_tbl[i].b_i=='C')){
                        switch (i) {
                        /* data type LLN */
                        case 2:
                        case 32:
                        case 100:
                                bit_ptr = pv_in_tbl[i].b_cont;
                                strncpy(LLL_bit_len, bit_ptr, 2);
                                bit_ptr=bit_ptr + 2;
                                strncpy(LLL_bit_cont, bit_ptr, atoi(LLL_bit_len));
                                strcat(mac_d, LLL_bit_cont);
                                break;

                        /* data type N */
                        case 4:

                                strcat(mac_d, pv_in_tbl[i].b_cont);
                                break;


                        /* data type ANS, AN */
                        case 37:
                        case 39:
                        case 41:
                                memset(ANS_bit_cont, 0, sizeof(ANS_bit_cont));
                                ANS_len = tobyte(pv_in_tbl[i].b_cont, ANS_bit_cont);
                                strncat(mac_d, ANS_bit_cont, ANS_len);
                                break;

                        default:
                                break;
                        }
                }
        }
        cout<<" MAC INPUT DATA:"<<mac_d<<"\n";
        return mac_d;
}

/*------------------------------------------------------------------------
**	Function:					mf_write_stru
**	Description:	    
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_write_stru(PG_txn_stru &pv_out_var_stru)
{
	MTI_TABLE *tbl_ptr;
	char temp_LLL[5];
	char *curr_bit_ptr;
	int  curr_bit_len;
	char curr_bit_cont[1024];
	char temp_bit[1024];
	char telnum_len[3];

        tbl_ptr = mv_page_ptr->mti_tbl;
	memset(pv_out_var_stru.payment_type, 0, sizeof(pv_out_var_stru.payment_type));
	memset(pv_out_var_stru.tel_num, 0, sizeof(pv_out_var_stru.tel_num));
	memset(pv_out_var_stru.ori_stan, 0, sizeof(pv_out_var_stru.ori_stan));
	memset(pv_out_var_stru.mac_key_sync_value, 0, sizeof(pv_out_var_stru.mac_key_sync_value));
	memset(pv_out_var_stru.mac_key_value, 0, sizeof(pv_out_var_stru.mac_key_value));

	for(int i=2; i<=NUM_BITS_128; i++)
	{
		// handle variant length
		if(tbl_ptr[i].b_LLL > 0)
		{
			if(tbl_ptr[i].b_LLL == 2)
			{
				curr_bit_ptr = tbl_ptr[i].b_cont;
				memset(temp_LLL, 0, sizeof(temp_LLL));
				strncpy(temp_LLL, curr_bit_ptr, 2);
				curr_bit_ptr += 2;
			}
			if(tbl_ptr[i].b_LLL == 3)
			{
				curr_bit_ptr = tbl_ptr[i].b_cont;
				memset(temp_LLL, 0, sizeof(temp_LLL));
				strncpy(temp_LLL, curr_bit_ptr, 4);
				curr_bit_ptr += 4;
			}
			curr_bit_len = atoi(temp_LLL);
		}
		// handle fixed length
		else
		{
			curr_bit_ptr = tbl_ptr[i].b_cont;
			curr_bit_len = tbl_ptr[i].b_size;
		}
		if( (tbl_ptr[i].b_type==2) || (tbl_ptr[i].b_type==3) )
		{
			curr_bit_len *= 2;
		}
		else if(tbl_ptr[i].b_type==5)
		{
			curr_bit_len /= 4;
		}

		memset(curr_bit_cont, 0, sizeof(curr_bit_cont));
		memcpy(curr_bit_cont, curr_bit_ptr, curr_bit_len);

		memset(tbl_ptr[i].b_cont, 0, sizeof(tbl_ptr[i].b_cont));
		memset(temp_bit, 0, sizeof(temp_bit));

		if( (tbl_ptr[i].b_type==2) || (tbl_ptr[i].b_type==3) )
		{
			tobyte(curr_bit_cont, temp_bit);
			strcpy(tbl_ptr[i].b_cont, temp_bit);
		}
		else
		{
			strcpy(tbl_ptr[i].b_cont, curr_bit_cont);
		}
	}

	strcpy(pv_out_var_stru.stan,			tbl_ptr[37].b_cont);
	strcpy(pv_out_var_stru.account, 		tbl_ptr[2].b_cont);
	strcpy(pv_out_var_stru.process_code,		tbl_ptr[3].b_cont);
	strcpy(pv_out_var_stru.amount, 			tbl_ptr[4].b_cont);
	strcpy(pv_out_var_stru.transfer_date_time, 	tbl_ptr[7].b_cont);
	strcpy(pv_out_var_stru.POS_pipeline_num,	tbl_ptr[11].b_cont);
	strcpy(pv_out_var_stru.POS_txn_time, 		tbl_ptr[12].b_cont);
	strcpy(pv_out_var_stru.POS_txn_date,		tbl_ptr[13].b_cont);
	strcpy(pv_out_var_stru.settle_date, 		tbl_ptr[15].b_cont);
	strcpy(pv_out_var_stru.txn_fee, 		tbl_ptr[28].b_cont);
	strcpy(pv_out_var_stru.PE_party_code, 		tbl_ptr[32].b_cont);
	strcpy(pv_out_var_stru.send_party_code, 	tbl_ptr[33].b_cont);
	strcpy(pv_out_var_stru.second_track, 		tbl_ptr[35].b_cont);
	strcpy(pv_out_var_stru.third_track, 		tbl_ptr[36].b_cont);
	strcpy(pv_out_var_stru.authorized_account, 	tbl_ptr[38].b_cont);
	strcpy(pv_out_var_stru.rsp_code, 		tbl_ptr[39].b_cont);
	strcpy(pv_out_var_stru.term_id, 		tbl_ptr[41].b_cont);
	strcpy(pv_out_var_stru.merchant_code, 		tbl_ptr[42].b_cont);
	strcpy(pv_out_var_stru.password, 		tbl_ptr[52].b_cont);
	strcpy(pv_out_var_stru.attached_amount, 	tbl_ptr[54].b_cont);
	strcpy(pv_out_var_stru.correct_code, 		tbl_ptr[60].b_cont);
	strcpy(pv_out_var_stru.random, 			tbl_ptr[63].b_cont);
	strcpy(pv_out_var_stru.recv_party_code, 	tbl_ptr[100].b_cont);
	strcpy(pv_out_var_stru.account_bit102, 		tbl_ptr[102].b_cont);
	strcpy(pv_out_var_stru.txn_declaration, 	tbl_ptr[104].b_cont);

	if( (!strncmp(mv_page_ptr->mti_code, "0100", 4)) || (!strncmp(mv_page_ptr->mti_code, "0110", 4)) )
	{
		//收费种类
		strncpy(pv_out_var_stru.payment_type,	tbl_ptr[48].b_cont, 4);
		//电话号码长度
		strncpy(telnum_len, tbl_ptr[48].b_cont+4, 2);
		//电话号码
		strncpy(pv_out_var_stru.tel_num, tbl_ptr[48].b_cont+6, atoi(telnum_len) );
		//交费月
		strncpy(pv_out_var_stru.pay_date, tbl_ptr[48].b_cont+6+atoi(telnum_len), 6);
	}
	
	if( (!strncmp(mv_page_ptr->mti_code, "0200", 4)) || (!strncmp(mv_page_ptr->mti_code, "0210", 4)) )
	{
		//收费种类
		strncpy(pv_out_var_stru.payment_type,	tbl_ptr[48].b_cont, 4);
		//电话号码长度
		strncpy(telnum_len, tbl_ptr[48].b_cont+4, 2);
		//电话号码
		strncpy(pv_out_var_stru.tel_num, tbl_ptr[48].b_cont+6, atoi(telnum_len) );
	}

	if( (!strncmp(mv_page_ptr->mti_code, "0400", 4)) || (!strncmp(mv_page_ptr->mti_code, "0401", 4))
	||  (!strncmp(mv_page_ptr->mti_code, "0410", 4)) )
	{
		//收费种类
		strncpy(pv_out_var_stru.payment_type,	tbl_ptr[48].b_cont, 4);
		//冲正流水号
		strncpy(pv_out_var_stru.ori_stan,	tbl_ptr[48].b_cont+4, 12);
		//电话号码长度
		strncpy(telnum_len, tbl_ptr[48].b_cont+16, 2);
		//电话号码
		strncpy(pv_out_var_stru.tel_num, tbl_ptr[48].b_cont+18, atoi(telnum_len) );
	}

	if( ( (!strncmp(mv_page_ptr->mti_code, "0800", 4)) || (!strncmp(mv_page_ptr->mti_code, "0801", 4)) )
	 &&  (!strncmp(mv_page_ptr->nmic, "101", 3)) )
	{
		strncpy(pv_out_var_stru.mac_key_sync_value,  tbl_ptr[48].b_cont,  4);
		strcpy(pv_out_var_stru.mac_key_value,       tbl_ptr[48].b_cont+4);
	}

#ifdef CLASS_DEBUG_LEVEL
//	cout << "PG_txn_variant_stru after extract packet:" << endl;
//	mf_display_stru(pv_out_var_stru);
#endif
	return 0;
}


/*------------------------------------------------------------------------
**	Function:					mf_display_stru
**	Description:	    
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
void Ciso8583::mf_display_stru(const PG_txn_stru &pv_in_var_stru)
{
	cout << "***********************************************************************" << endl;
	cout << "biz_type:["		<< pv_in_var_stru.biz_type			<< "]" << endl;
	cout << "mod_code:["		<< pv_in_var_stru.mod_code			<< "]" << endl;
	cout << "to_gw_mti_code:["		<< pv_in_var_stru.to_gw_mti_code			<< "]" << endl;
	cout << "from_gw_mti_code:["		<< pv_in_var_stru.from_gw_mti_code			<< "]" << endl;
	cout << "proc_code:["		<< pv_in_var_stru.proc_code			<< "]" << endl;
	cout << "att_msg:["		<< pv_in_var_stru.att_msg			<< "]" << endl;

	cout << "stan:["		<< pv_in_var_stru.stan			<< "]" << endl;
	cout << "account:["		<< pv_in_var_stru.account		<< "]" << endl;
	cout << "process_code:["	<< pv_in_var_stru.process_code		<< "]" << endl;
	cout << "amount:["		<< pv_in_var_stru.amount		<< "]" << endl;
	cout << "transfer_date_time:["	<< pv_in_var_stru.transfer_date_time	<< "]" << endl;
	cout << "POS_pipeline_num:["	<< pv_in_var_stru.POS_pipeline_num	<< "]" << endl;
	cout << "POS_txn_time:["	<< pv_in_var_stru.POS_txn_time		<< "]" << endl;
	cout << "POS_txn_date:["	<< pv_in_var_stru.POS_txn_date		<< "]" << endl;
	cout << "settle_date:["		<< pv_in_var_stru.settle_date		<< "]" << endl;
	cout << "txn_fee:["		<< pv_in_var_stru.txn_fee		<< "]" << endl;
	cout << "PE_party_code:["	<< pv_in_var_stru.PE_party_code		<< "]" << endl;
	cout << "send_party_code:["	<< pv_in_var_stru.send_party_code	<< "]" << endl;
	cout << "second_track:["	<< pv_in_var_stru.second_track		<< "]" << endl;
	cout << "third_track:["		<< pv_in_var_stru.third_track		<< "]" << endl;
	cout << "authorized_account:["	<< pv_in_var_stru.authorized_account	<< "]" << endl;
	cout << "respond_code:["	<< pv_in_var_stru.rsp_code		<< "]" << endl;
	cout << "term_id:["		<< pv_in_var_stru.term_id		<< "]" << endl;
	cout << "merchant_code:["	<< pv_in_var_stru.merchant_code		<< "]" << endl;
	cout << "payment_type:["	<< pv_in_var_stru.payment_type		<< "]" << endl;
	cout << "telephone_number:["	<< pv_in_var_stru.tel_num		<< "]" << endl;
	cout << "original_stan:["	<< pv_in_var_stru.ori_stan		<< "]" << endl;
	cout << "sync_mac_key:["	<< pv_in_var_stru.mac_key_sync_value	<< "]" << endl;
	cout << "mac_key:["		<< pv_in_var_stru.mac_key_value		<< "]" << endl;
	cout << "password:["		<< pv_in_var_stru.password		<< "]" << endl;
	cout << "attached_amount:["	<< pv_in_var_stru.attached_amount	<< "]" << endl;
	cout << "correct_code:["	<< pv_in_var_stru.correct_code		<< "]" << endl;
	cout << "random:["		<< pv_in_var_stru.random		<< "]" << endl;
	cout << "recv_party_code:["	<< pv_in_var_stru.recv_party_code	<< "]" << endl;
	cout << "account_bit102:["	<< pv_in_var_stru.account_bit102	<< "]" << endl;
	cout << "txn_declaration:["	<< pv_in_var_stru.txn_declaration	<< "]" << endl;
	cout << "pri125:["	<< pv_in_var_stru.pri125	<< "]" << endl;
	cout << "***********************************************************************" << endl;
}


/*------------------------------------------------------------------------
**	Function:					mf_request_hdlr
**	Description:	    Fuction to handle the request 
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						mti code, variant sturct
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_request_hdlr(const char* pv_in_mti_code, PG_txn_stru &pv_in_var_stru)
{
	MTI_TABLE *tbl_ptr;
	char mech_code[16];
	char *nmic_ptr;
	long  bit_len;
	char bit_len_str[4+1];
	int  b48_len;
	char b48_len_str[4];
	char hex_buff[10240];
	char bit_buff[1024];
	int  direction;

	direction = 0;
	//lxd
	//mv_page_ptr = mf_search_mti_page(HANDLE_PARTY_GROUP, pv_in_mti_code, "XXXXXX", "XXX");
	mv_page_ptr = mf_search_mti_page(HANDLE_PARTY_GROUP, pv_in_mti_code, "000000", "XXX");
	if (mv_page_ptr == NULL)
	{
		cout << "Error searching mti page!" << endl;
		return -1;
	}
	tbl_ptr = mv_page_ptr->mti_tbl;
	mf_read_var_stru(pv_in_var_stru);

	//bit48
	memset(tbl_ptr[48].b_cont, 0, sizeof(tbl_ptr[48].b_cont));
	switch( atoi(pv_in_mti_code) )
	{
		case 100:
		case 200:
        	sprintf( tbl_ptr[48].b_cont, "%s%02d%s%s",
							pv_in_var_stru.payment_type, 
							strlen(pv_in_var_stru.tel_num),
							pv_in_var_stru.tel_num,
							pv_in_var_stru.pay_date);
			break;
		default:
        	strcat(tbl_ptr[48].b_cont,  pv_in_var_stru.payment_type);
			strcat(tbl_ptr[48].b_cont,  pv_in_var_stru.ori_stan);
        	strcat(tbl_ptr[48].b_cont,  pv_in_var_stru.tel_num);
			break;
	}
	//cout << "bit48: [" << tbl_ptr[48].b_cont << "]" << endl;
	
	//bit125
	strcpy(tbl_ptr[125].b_cont, pv_in_var_stru.pri125);

	//bit128
	strcpy(tbl_ptr[128].b_cont, "1111111111111111");


	if(direction==1)
	for(int i=2; i<=NUM_BITS_128; i++)
	{
		if(tbl_ptr[i].b_o == 'M' || tbl_ptr[i].b_o == 'C')
		{
			bit_len = strlen(tbl_ptr[i].b_cont);

			if(bit_len == 0)
				continue;

			if(tbl_ptr[i].b_type==2 || tbl_ptr[i].b_type==3)
			{
				memset(hex_buff, 0, sizeof(hex_buff));
				byte_to_hex(tbl_ptr[i].b_cont, hex_buff, bit_len);
				memset(tbl_ptr[i].b_cont, 0, sizeof(tbl_ptr[i].b_cont));
				strcpy(tbl_ptr[i].b_cont, hex_buff);
			}

			if(tbl_ptr[i].b_LLL != 0)
			{
				memset(bit_buff, 0, sizeof(bit_buff));
				strcpy(bit_buff, tbl_ptr[i].b_cont);

				memset(bit_len_str, 0, sizeof(bit_len_str));
				memset(tbl_ptr[i].b_cont, 0, sizeof(tbl_ptr[i].b_cont));

				if(tbl_ptr[i].b_LLL == 2)
				{
					sprintf(bit_len_str, "%02d", bit_len);
				}
				if(tbl_ptr[i].b_LLL == 3)
                                {
                                	sprintf(bit_len_str, "%04d", bit_len);
				}
                                strcpy(tbl_ptr[i].b_cont, bit_len_str);
                                strcat(tbl_ptr[i].b_cont, bit_buff);
			}
		}
	}

	if(direction==0)
        for(int i=2; i<=NUM_BITS_128; i++)
        {
                if(tbl_ptr[i].b_i == 'M' || tbl_ptr[i].b_i == 'C')
                {
                        bit_len = strlen(tbl_ptr[i].b_cont);

                        if(bit_len == 0)
                                continue;

                        if(tbl_ptr[i].b_type == 2 || tbl_ptr[i].b_type == 3)
                        {
                                memset(hex_buff, 0, sizeof(hex_buff));
                                byte_to_hex(tbl_ptr[i].b_cont, hex_buff, bit_len);
                                memset(tbl_ptr[i].b_cont, 0, sizeof(tbl_ptr[i].b_cont));
                                strcpy(tbl_ptr[i].b_cont, hex_buff);
                        }

                        if(tbl_ptr[i].b_LLL != 0)
                        {
                                memset(bit_buff, 0, sizeof(bit_buff));
                                strcpy(bit_buff, tbl_ptr[i].b_cont);
                                memset(bit_len_str, 0, sizeof(bit_len_str));
                                memset(tbl_ptr[i].b_cont, 0, sizeof(tbl_ptr[i].b_cont));

                                if(tbl_ptr[i].b_LLL == 2)
                                {
                                        sprintf(bit_len_str, "%02d", bit_len);
                                }
                                if(tbl_ptr[i].b_LLL == 3)
                                {
                                        sprintf(bit_len_str, "%04d", bit_len);
                                }
                                strcpy(tbl_ptr[i].b_cont, bit_len_str);
                                strcat(tbl_ptr[i].b_cont, bit_buff);
                        }
                }
        }

	// bit 7 - date time
	char curr_date[10];
	char *date_ptr;
	char curr_time[10];

	memset(tbl_ptr[7].b_cont, 0, sizeof(tbl_ptr[7].b_cont));

	memset(curr_date, 0, sizeof(curr_date));
	strcpy(curr_date, getdate()); //get current date in YYMMDD form.
	date_ptr = curr_date + 2;     //get MMDD.
        strncpy(tbl_ptr[7].b_cont, date_ptr, 4);

	memset(curr_time, 0, sizeof(curr_time));
	strcpy(curr_time, gettime2(NULL));      //get current time in HHMMSS form.
	strncat(tbl_ptr[7].b_cont, curr_time, 6);

#ifdef CLASS_DEBUG_LEVEL
        //cout << "bit7: [" << tbl_ptr[7].b_cont << "]" << endl;
#endif


	return direction;
}

/*------------------------------------------------------------------------
**	Function:					mf_read_var_stru
**	Description:	    Fuction to read out the data from variant struct into the bit content
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						variant struct 
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
int Ciso8583::mf_read_var_stru(PG_txn_stru &pv_in_var_stru)
{
 	MTI_TABLE *tbl_ptr;

	tbl_ptr = mv_page_ptr->mti_tbl;

	strcpy(tbl_ptr[37].b_cont, 	pv_in_var_stru.stan);
	strcpy(tbl_ptr[2].b_cont, 	pv_in_var_stru.account);
	strcpy(tbl_ptr[4].b_cont, 	pv_in_var_stru.amount);
	strcpy(tbl_ptr[7].b_cont, 	pv_in_var_stru.transfer_date_time);
	strcpy(tbl_ptr[11].b_cont, 	pv_in_var_stru.POS_pipeline_num);
	strcpy(tbl_ptr[12].b_cont, 	pv_in_var_stru.POS_txn_time);
	strcpy(tbl_ptr[15].b_cont, 	pv_in_var_stru.settle_date);
	strcpy(tbl_ptr[28].b_cont, 	pv_in_var_stru.txn_fee);
	strcpy(tbl_ptr[32].b_cont, 	pv_in_var_stru.PE_party_code);
	strcpy(tbl_ptr[33].b_cont, 	pv_in_var_stru.send_party_code);
	strcpy(tbl_ptr[35].b_cont, 	pv_in_var_stru.second_track);
	strcpy(tbl_ptr[36].b_cont, 	pv_in_var_stru.third_track);
	strcpy(tbl_ptr[38].b_cont, 	pv_in_var_stru.authorized_account);
	strcpy(tbl_ptr[39].b_cont, 	pv_in_var_stru.rsp_code);
	strcpy(tbl_ptr[41].b_cont, 	pv_in_var_stru.term_id);
	strcpy(tbl_ptr[42].b_cont, 	pv_in_var_stru.merchant_code);
	strcpy(tbl_ptr[52].b_cont, 	pv_in_var_stru.password);
	strcpy(tbl_ptr[54].b_cont, 	pv_in_var_stru.attached_amount);
	strcpy(tbl_ptr[60].b_cont, 	pv_in_var_stru.correct_code);
	strcpy(tbl_ptr[63].b_cont, 	pv_in_var_stru.random);
	strcpy(tbl_ptr[100].b_cont, 	pv_in_var_stru.recv_party_code);
	strcpy(tbl_ptr[102].b_cont, 	pv_in_var_stru.account_bit102);
	strcpy(tbl_ptr[104].b_cont, 	pv_in_var_stru.txn_declaration);

	//POS_txn_date is "YYYYMMDD" form;   bit 13 need "MMDD" form.
	strncpy(tbl_ptr[13].b_cont, &(pv_in_var_stru.POS_txn_date[4]), 4);

	return 0;
}


/*------------------------------------------------------------------------
**	Function:					mf_init_var_stru
**	Description:	    Fuction to read out the data from variant struct into the bit content
**	Calls:						Ciso8583
**	Called By:				
**	Table Accessed:	    	
**	Table Updated:		
**	Input:						variant struct 
**	Output:						
**	Return:						
**	Others:				
------------------------------------------------------------------------*/
void Ciso8583::mf_init_var_stru(PG_txn_stru &pv_in_var_stru)
{
	memset( pv_in_var_stru.stan, 0, sizeof(pv_in_var_stru.stan) );
	memset( pv_in_var_stru.account, 0, sizeof(pv_in_var_stru.account) );
	memset( pv_in_var_stru.amount, 0, sizeof(pv_in_var_stru.amount) );
	memset( pv_in_var_stru.transfer_date_time, 0, sizeof(pv_in_var_stru.transfer_date_time) );
	memset( pv_in_var_stru.POS_pipeline_num, 0, sizeof(pv_in_var_stru.POS_pipeline_num) );
	memset( pv_in_var_stru.POS_txn_time, 0, sizeof(pv_in_var_stru.POS_txn_time) );
	memset( pv_in_var_stru.settle_date, 0, sizeof(pv_in_var_stru.settle_date) );
	memset( pv_in_var_stru.txn_fee, 0, sizeof(pv_in_var_stru.txn_fee) );
	memset( pv_in_var_stru.PE_party_code, 0, sizeof(pv_in_var_stru.PE_party_code) );
	memset( pv_in_var_stru.send_party_code, 0, sizeof(pv_in_var_stru.send_party_code) );
	memset( pv_in_var_stru.second_track, 0, sizeof(pv_in_var_stru.second_track) );
	memset( pv_in_var_stru.third_track, 0, sizeof(pv_in_var_stru.third_track) );
	memset( pv_in_var_stru.authorized_account, 0, sizeof(pv_in_var_stru.authorized_account) );
	memset( pv_in_var_stru.term_id, 0, sizeof(pv_in_var_stru.term_id) );
	memset( pv_in_var_stru.rsp_code, 0, sizeof(pv_in_var_stru.rsp_code) );
	memset( pv_in_var_stru.merchant_code, 0, sizeof(pv_in_var_stru.merchant_code) );
	memset( pv_in_var_stru.password, 0, sizeof(pv_in_var_stru.password) );
	memset( pv_in_var_stru.attached_amount, 0, sizeof(pv_in_var_stru.attached_amount) );
	memset( pv_in_var_stru.correct_code, 0, sizeof(pv_in_var_stru.correct_code) );
	memset( pv_in_var_stru.random, 0, sizeof(pv_in_var_stru.random) );
	memset( pv_in_var_stru.recv_party_code, 0, sizeof(pv_in_var_stru.recv_party_code) );
	memset( pv_in_var_stru.account_bit102, 0, sizeof(pv_in_var_stru.account_bit102) );
	memset( pv_in_var_stru.txn_declaration, 0, sizeof(pv_in_var_stru.txn_declaration) );
	memset( pv_in_var_stru.POS_txn_date, 0, sizeof(pv_in_var_stru.POS_txn_date) );
	memset( pv_in_var_stru.payment_type, 0, sizeof(pv_in_var_stru.payment_type) );
	memset( pv_in_var_stru.tel_num, 0, sizeof(pv_in_var_stru.tel_num) );
	memset( pv_in_var_stru.ori_stan, 0, sizeof(pv_in_var_stru.ori_stan) );
	memset( pv_in_var_stru.mac_key_sync_value, 0, sizeof(pv_in_var_stru.mac_key_sync_value) );
	memset( pv_in_var_stru.mac_key_value, 0, sizeof(pv_in_var_stru.mac_key_value) );
	memset( pv_in_var_stru.process_code, 0, sizeof(pv_in_var_stru.process_code) );
	memset( pv_in_var_stru.pri125, 0, sizeof(pv_in_var_stru.pri125) );
	memset( pv_in_var_stru.pay_date, 0, sizeof(pv_in_var_stru.pay_date) );

}

/*
int main()
{
	Ciso8583 test;
	char buffer[1024];
	char mv_hex_buf[2048];
	char mti_code[5];
	PG_txn_stru var_stru;

	test.mf_init();
	test.mf_init_var_stru(var_stru);
	memset(mti_code, 0, sizeof(mti_code));

	strcpy(buffer, "0200923a0001088100000000000015000001000000000100081214510412346114505720050812081306988620303030303832313937373938333232323232323200143630303130383835353838383535031234183534343131393931393030303035303438390001503532393030383130");
//	strcpy(buffer, "0400923a000108810010000000001500000100000000010008121451511234611451502005081208130698862030303030383231393737393933323232323232320026303030313030303038323139373739383038383535383838353500043730313306200001183534343131393931393030303035303438390001503532393030393130");
//	strcpy(buffer, "0100823a0001088100000000000011000001081214080000712515145820050421081306988620303030303832313937373837383832303031363300143030343630383835343432393337062000010001503338363635393739");
	test.mf_extra(mti_code, var_stru, buffer);
	strcpy(var_stru.rsp_code,"00");

//	strcpy(mv_send_stru.payment_type,"0100");
//	strcpy(mv_send_stru.tel_num,"31985697");

	test.mf_display_stru(var_stru);
	test.mf_const(mti_code, var_stru, mv_hex_buf);
	return 0;
}

*/