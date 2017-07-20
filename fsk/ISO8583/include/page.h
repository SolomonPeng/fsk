#ifndef  PAGE_H
#define  PAGE_H

#include<string.h>
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
//#include<unistd.h>

/////////////////////////     constant  //////////////////////////////

#define		CB_PARTY_NUM	25
#define		DB_SERVER_NAME	"tope"
#define		DB_USER_ACC	"pe"
#define		DB_USER_PWD	"pe123"

#define MTI_INPUT_FILE	"mti.txt"

#define NUM_CB_PARTY	30

#define NUM_MTI_PAGES	30
#define NUM_BITS_128	128
#define NUM_BITS_128	128
#define NUM_BITS	128

#define NUM_MECH_CODE  200

#define MAX_MAC_LENGTH 300
#define MAX_KEY_LENGTH 16

#define PG_DEF_PATH "C:\\Project\\pkg8583_test\\Release\\"	//lxd


#define MAX_LOG_PATH_LENGTH       100
#define MAX_LOG_FILE_NAME_LENGTH  100

#define CONFIG_FILENAME	          "config.txt"

///////////////////////////  constant end  ///////////////////////////





#define B0_LEN  (64+1)
#define B1_LEN  (64+1)
#define B2_LEN  (19+1+10+1)	/*add*//*error*/
#define B3_LEN  (6+1)
#define B4_LEN  (12+1)
#define B5_LEN  (12+1)
#define B6_LEN  (12+1)
#define B7_LEN  (10+1)
#define B8_LEN  (8+1)
#define B9_LEN  (8+1)
#define B10_LEN (8+1)
#define B11_LEN (6+1)
#define B12_LEN (6+1)
#define B13_LEN (8+1)
#define B14_LEN (4+1)
#define B15_LEN (4+1)
#define B16_LEN (4+1)
#define B17_LEN (4+1)
#define B18_LEN (4+1)
#define B19_LEN (3+1)
#define B20_LEN (3+1)
#define B21_LEN (3+1)
#define B22_LEN (3+1)
#define B23_LEN (3+1)
#define B24_LEN (4+1)
#define B25_LEN (2+1)
#define B26_LEN (2+1)
#define B27_LEN (1+1)
#define B28_LEN (8+1)
#define B29_LEN (8+1)
#define B30_LEN (8+1)
#define B31_LEN (8+1)
#define B32_LEN (11+1)
#define B33_LEN (11+1)
#define B34_LEN (28+1)
#define B35_LEN (37*2+1+1+10)
#define B36_LEN (104*2+1+10)
#define B37_LEN (12*2+1)
#define B38_LEN (6*2+1)
#define B39_LEN (2*2+1)
#define B40_LEN (3+1)
#define B41_LEN (8*2+1)
#define B42_LEN (15*2+1)
#define B43_LEN (40+1)
#define B44_LEN (19+1)			/*check*/
#define B45_LEN (76+1)
#define B46_LEN (999+1)
#define B47_LEN (999+1)
#define B48_LEN (36*2+1+4)		/*check*/
#define B49_LEN (3+1)
#define B50_LEN (3+1)
#define B51_LEN (3+1)
#define B52_LEN (64+1)
#define B53_LEN (16+1)
#define B54_LEN (13*2+1)
#define B55_LEN (999+1)
#define B56_LEN (999+1)
#define B57_LEN (999+1)
#define B58_LEN (999+1)
#define B59_LEN (999+1)
#define B60_LEN (6*2+4+1+1)		/*check*/
#define B61_LEN (16*2+1+4)		/*check*/
#define B62_LEN (64*2+1+4)		/*check*/ /*problem*/
#define B63_LEN (999*2+1+4)		/*check*/ /*problem*/
#define B64_LEN (64+1)
#define B65_LEN (1+1)
#define B66_LEN (1+1)
#define B67_LEN (2+1)
#define B68_LEN (3+1+1)
#define B69_LEN (3+1+1)
#define B70_LEN (3+1+1)
#define B71_LEN (4+1)
#define B72_LEN (4+1)
#define B73_LEN (6+1)
#define B74_LEN (10+1)
#define B75_LEN (10+1)
#define B76_LEN (10+1)
#define B77_LEN (10+1)
#define B78_LEN (10+1)
#define B79_LEN (10+1)
#define B80_LEN (10+1)
#define B81_LEN (10+1)
#define B82_LEN (12+1)
#define B83_LEN (12+1)
#define B84_LEN (12+1)
#define B85_LEN (12+1)
#define B86_LEN (16+1)
#define B87_LEN (16+1)
#define B88_LEN (16+1)
#define B89_LEN (16+1)
#define B90_LEN (42+1)
#define B91_LEN (1*2+1+1)
#define B92_LEN (2*2+1)
#define B93_LEN (5*2+1+1)
#define B94_LEN (7*2+1+1)
#define B95_LEN (42*2+1)
#define B96_LEN (64+1)
#define B97_LEN (97+1+1)
#define B98_LEN (25*2+1+1)
#define B99_LEN (11+1+1)
#define B100_LEN (99+1+2+1)
#define B101_LEN (17*2+2+1+1)
#define B102_LEN (30+2+1+1)
#define B103_LEN (28*2+2+1)
#define B104_LEN (100*2+4+1)
#define B105_LEN (999*2+4+1+1)
#define B106_LEN (999*2+4+1+1)
#define B107_LEN (999*2+1+4+1)
#define B108_LEN (999*2+4+1+1)
#define B109_LEN (999*2+4+1+1)
#define B110_LEN (999*2+4+1+1)
#define B111_LEN (999*2+4+1+1)
#define B112_LEN (999*2+4+1+1)
#define B113_LEN (999*2+4+1+1)
#define B114_LEN (999*2+4+1+1)
#define B115_LEN (999*2+4+1+1)
#define B116_LEN (999*2+4+1+1)
#define B117_LEN (999*2+4+1+1)
#define B118_LEN (999*2+4+1+1)
#define B119_LEN (999*2+4+1+1)
#define B120_LEN (999*2+4+1+1)
#define B121_LEN (999*2+4+1+1)
#define B122_LEN (999*2+4+1+1)
#define B123_LEN (999*2+4+1+1)
#define B124_LEN (999*2+4+1+1)
#define B125_LEN (9999*2+4+1+1)
#define B126_LEN (999*2+4+1+1)
#define B127_LEN (999*2+4+1+1)
#define B128_LEN (64+1)


/*
 * memory allocated to hold incomming message
 */
typedef struct
{
   char b0[B0_LEN];
   char b1[B1_LEN]; 
   char b2[B2_LEN];	/*add*//*error*/
   char b3[B3_LEN];
   char b4[B4_LEN];
   char b5[B5_LEN];
   char b6[B6_LEN];
   char b7[B7_LEN];
   char b8[B8_LEN];
   char b9[B9_LEN];
   char b10[B10_LEN];
   char b11[B11_LEN];
   char b12[B12_LEN];
   char b13[B13_LEN];
   char b14[B14_LEN];
   char b15[B15_LEN];
   char b16[B16_LEN];
   char b17[B17_LEN];
   char b18[B18_LEN];
   char b19[B19_LEN];
   char b20[B20_LEN];
   char b21[B21_LEN];
   char b22[B22_LEN];
   char b23[B23_LEN];
   char b24[B24_LEN];
   char b25[B25_LEN];
   char b26[B26_LEN];
   char b27[B27_LEN];
   char b28[B28_LEN];
   char b29[B29_LEN];
   char b30[B30_LEN];
   char b31[B31_LEN];
   char b32[B32_LEN];
   char b33[B33_LEN];
   char b34[B34_LEN];
   char b35[B35_LEN];
   char b36[B36_LEN];
   char b37[B37_LEN];
   char b38[B38_LEN];
   char b39[B39_LEN];
   char b40[B40_LEN];
   char b41[B41_LEN];
   char b42[B42_LEN];
   char b43[B43_LEN];
   char b44[B44_LEN];	/*check*/
   char b45[B45_LEN];
   char b46[B46_LEN];
   char b47[B47_LEN];
   char b48[B48_LEN];	/*check*/
   char b49[B49_LEN];
   char b50[B50_LEN];
   char b51[B51_LEN];
   char b52[B52_LEN];
   char b53[B53_LEN];
   char b54[B54_LEN];
   char b55[B55_LEN];
   char b56[B56_LEN];
   char b57[B57_LEN];
   char b58[B58_LEN];
   char b59[B59_LEN];
   char b60[B60_LEN];	/*check*/
   char b61[B61_LEN];  	/*check*/
   char b62[B62_LEN];	/*check*/ /*problem*/
   char b63[B63_LEN];	/*check*/ /*problem*/
   char b64[B64_LEN];
   char b65[B65_LEN];
   char b66[B66_LEN];
   char b67[B67_LEN];
   char b68[B68_LEN];
   char b69[B69_LEN];
   char b70[B70_LEN];
   char b71[B71_LEN];
   char b72[B72_LEN];
   char b73[B73_LEN];
   char b74[B74_LEN];
   char b75[B75_LEN];
   char b76[B76_LEN];
   char b77[B77_LEN];
   char b78[B78_LEN];
   char b79[B79_LEN];
   char b80[B80_LEN];
   char b81[B81_LEN];
   char b82[B82_LEN];
   char b83[B83_LEN];
   char b84[B84_LEN];
   char b85[B85_LEN];
   char b86[B86_LEN];
   char b87[B87_LEN];
   char b88[B88_LEN];
   char b89[B89_LEN];
   char b90[B90_LEN];
   char b91[B91_LEN];
   char b92[B92_LEN];
   char b93[B93_LEN];
   char b94[B94_LEN];
   char b95[B95_LEN];
   char b96[B96_LEN];
   char b97[B97_LEN];
   char b98[B98_LEN];
   char b99[B99_LEN];
   char b100[B100_LEN];
   char b101[B101_LEN];
   char b102[B102_LEN];
   char b103[B103_LEN];
   char b104[B104_LEN];
   char b105[B105_LEN];
   char b106[B106_LEN];
   char b107[B107_LEN];
   char b108[B108_LEN];
   char b109[B109_LEN];
   char b110[B110_LEN];
   char b111[B111_LEN];
   char b112[B112_LEN];
   char b113[B113_LEN];
   char b114[B114_LEN];
   char b115[B115_LEN];
   char b116[B116_LEN];
   char b117[B117_LEN];
   char b118[B118_LEN];
   char b119[B119_LEN];
   char b120[B120_LEN];
   char b121[B121_LEN];
   char b122[B122_LEN];
   char b123[B123_LEN];
   char b124[B124_LEN];
   char b125[B125_LEN];
   char b126[B126_LEN];
   char b127[B127_LEN];
   char b128[B128_LEN];
} BIT_TABLE;



/* FIELDS DESC
 *	b_on_off_i, b_on_off_o
 *		  - set according to the run time message format
 *		  - b_on_off_i set as the incoming bit map
 *		  - b_on_off_o set as the outgoing bit map
 *
 * 	b_type, b_LLL, b_size, b_i, b_o
 * 		  - fields set according to mti.txt data file
 *		  - these fields denote the standardised message format
 *
 *	b_cont
 *		  - a pointer to the allocated memory location to hold 
 *		    the inbound or outbound bit content
 */ 
typedef struct {
   int b_on_off_i;
   int b_on_off_o;
   int b_type;
   int b_LLL;
   int b_size;
   char b_i;
   char b_o;
   char *b_cont;
}MTI_TABLE ;


/*
 * mti_tbl - a pointer to MTI_TABLE structure holding the data structure
 *	     description of a read in message
 */
typedef  struct {
   char party[10];
   char mti_code[10];
   char p_code[10];
   char nmic[10];
   char rtn_mti[10];
   MTI_TABLE *mti_tbl;
}MTI_PAGE ;       

typedef struct   //支付网关变动部分结构
{
	char biz_type[4+1];             /*交易类型*/
	char mod_code[2+1];             /*模块号, 网关可能分布为多个, 以连接不同业务单位或作负载平衡, 用模块号作标识.*/
	char to_gw_mti_code[4+1];       /*支撑平台发送到网关的交易命令代码*/
	char from_gw_mti_code[4+1];     /*网关发送到支撑平台的交易命令返回码*/
	char proc_code[6+1];            /*处理码, 作为to_gw_mti_code的补充命令，组成完整的交易命令.*/
	char att_msg[16];               /*附加字段, 作扩充用途. */

	char stan[12+1];		//bit 37		/*系统参考号*/
	char account[30+1];		//bit 2
	char process_code[6+1];		//bit 3
	char amount[12+1];		//bit 4
	char transfer_date_time[10+1];	
	char POS_pipeline_num[6+1];	//bit 11
	char POS_txn_time[6+1];		//bit 12
	char POS_txn_date[8+1];		//bit 13
	char settle_date[4+1];		//bit 15
	char txn_fee[9+1];		//bit 28
	char PE_party_code[6+1];	//bit 32
	char send_party_code[6+1];	//bit 33
	char second_track[39+1];	//bit 35
	char third_track[107+1];	//bit 36
	char authorized_account[6+1];	//bit 38
	char rsp_code[6+1];		//bit 39	/*返回码, 作为from_gw_mti_code的补充, 组成完整的返回信息. */
	char term_id[8+1];		//bit 41
	char merchant_code[15+1];     	// bit 42 

	char payment_type[4+1];		// bit 481
	char tel_num[20+1];		// bit 482
	char pay_date[10];		//bit483 查询交易
	char ori_stan[12+1];		// bit 483	冲正交易
	char mac_key_sync_value[4+1];	// bit 484
	char mac_key_value[16+1];	// bit 485
	
	char password[64+1];		//bit 52
	char correct_code[4+1];		//bit 60
	char attached_amount[13+1];	//bit 54
	char random[12+1];		//bit 63
	char recv_party_code[6+1];	//bit 100
	char account_bit102[30+1];	//bit 102
	char txn_declaration[13+1];	//bit 104
	char pri125[B125_LEN];	//bit 125
} PG_txn_stru;


typedef struct 
{
	char biz_type[20];
	char party_code[6+1];
	char mech_code[16];
}mech_table_stru;


typedef struct
{
	char party_code[6+1];
	char mac_key[16+1];
	char key_sync_const[4+1];
	char master_key[16+1];
}mac_table_stru;


typedef struct
{
	int	type;
	int	LLL;
	int	len;
}BIT_DEF;


#endif

