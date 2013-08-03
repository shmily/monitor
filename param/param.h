/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-12-06 15:50
#      Filename : config.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _CONDIF_H_
#define _CONDIF_H_

#define _CONFIG_FILE "./DrowsyDriving.cfg"

struct _dev_parameter{
	char apn[8];	
	char ip[16];
	char port[8];
	char dev_id[16];
    char ver[16];
	int  ver_len;
	char ven[16];
	int  ven_len;
	char group_id[16];
	int  group_id_len;
};

int DevParam_Init(void);
int param_save(struct _dev_parameter *pDev_param);

char * Param_IP(void);
char * Param_Port(void);
int    Param_APN(void);

int Get_Dev_IDH(void);
int Get_Dev_IDL(void);

int Get_ver(char *p);
int Get_ven(char *p);
int Get_group_id(char *p);
#endif
