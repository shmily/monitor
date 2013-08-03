/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-12-06 16:05
#      Filename : main.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iniparser.h"
#include "param.h"

static struct _dev_parameter Dev_Param;
static int _DEV_ID_L;
static int _DEV_ID_H;

static int printf_bar(FILE *fp);
static void Dev_ID_Init(char *pdata);


int DevParam_Init(void)
{
	char 	   * pkey_val;
	dictionary * ini;

	memset((char *)&Dev_Param, 0, sizeof(struct _dev_parameter));
	
	strcpy(Dev_Param.apn,     	"CMNET");
	strcpy(Dev_Param.ip,     	"211.137.45.80");
	strcpy(Dev_Param.port,   	"11004");
	strcpy(Dev_Param.dev_id, 	"15140539442");
	strcpy(Dev_Param.ver,    	"12800030100");
	strcpy(Dev_Param.ven,    	"1067");
	strcpy(Dev_Param.group_id,  "cldw0000069");

	Dev_Param.ver_len 		= strlen(Dev_Param.ver);	
	Dev_Param.ven_len 		= strlen(Dev_Param.ven);	
	Dev_Param.group_id_len 	= strlen(Dev_Param.group_id);	

			
	ini = iniparser_load(_CONFIG_FILE);		// we load the param from the config file

	if(ini != NULL){	
		iniparser_dump_ini(ini, stdout);

		// load the param from the file one by one
		pkey_val = iniparser_getstring(ini, "device_param:apn", Dev_Param.apn); 		
		strcpy(Dev_Param.apn, pkey_val);
		
		pkey_val = iniparser_getstring(ini, "device_param:ip", Dev_Param.ip); 		
		strcpy(Dev_Param.ip, pkey_val);
		
		pkey_val = iniparser_getstring(ini, "device_param:port", Dev_Param.port); 		
		strcpy(Dev_Param.port, pkey_val);
		
		pkey_val = iniparser_getstring(ini, "device_param:dev_id", Dev_Param.dev_id); 		
		strcpy(Dev_Param.dev_id, pkey_val);
		
		pkey_val = iniparser_getstring(ini, "device_param:ver", Dev_Param.ver); 		
		strcpy(Dev_Param.ver, pkey_val);
		
		pkey_val = iniparser_getstring(ini, "device_param:ven", Dev_Param.ven); 		
		strcpy(Dev_Param.ven, pkey_val);
		
		pkey_val = iniparser_getstring(ini, "device_param:group_id", Dev_Param.group_id); 		
		strcpy(Dev_Param.group_id, pkey_val);
	} else {
		fprintf(stderr, "iniparser_load error...\n");
		fprintf(stderr, "Can NOT find \"%s\" file, use the defalut parameter.\n", _CONFIG_FILE);
	}
	
	iniparser_freedict(ini);

	// show the parameter	
	Dev_Param.ver_len 		= strlen(Dev_Param.ver);	
	Dev_Param.ven_len 		= strlen(Dev_Param.ven);	
	Dev_Param.group_id_len 	= strlen(Dev_Param.group_id);	
	
	fprintf(stderr, "The Device parameter :\n");
	
	fprintf(stderr, " >       apn = %s \n",     Dev_Param.apn);
	fprintf(stderr, " >        ip = %s \n",     Dev_Param.ip);
	fprintf(stderr, " >      port = %s \n",     Dev_Param.port);
	fprintf(stderr, " >    dev_id = %s \n",     Dev_Param.dev_id);
	fprintf(stderr, " >       ver = %s [%d]\n", Dev_Param.ver, 		Dev_Param.ver_len);
	fprintf(stderr, " >       ven = %s [%d]\n", Dev_Param.ven, 		Dev_Param.ven_len);
	fprintf(stderr, " >  group_id = %s [%d]\n", Dev_Param.group_id, Dev_Param.group_id_len);

	Dev_ID_Init(Dev_Param.dev_id);

	return 0;
}

int param_save(struct _dev_parameter *pDev_param)
{
	FILE  	   * fp;
	dictionary * ini;

	ini = iniparser_load(_CONFIG_FILE);		// we load the param from the config file
	
	if(ini != NULL){

		// save the param
		iniparser_set(ini, "device_param:apn",      pDev_param->apn);	
		iniparser_set(ini, "device_param:ip",       pDev_param->ip);	
		iniparser_set(ini, "device_param:port",     pDev_param->port);	
		iniparser_set(ini, "device_param:dev_id",   pDev_param->dev_id);	
		iniparser_set(ini, "device_param:ver",      pDev_param->ver);	
		iniparser_set(ini, "device_param:ven",      pDev_param->ven);	
		iniparser_set(ini, "device_param:group_id", pDev_param->group_id);	

		fp=fopen(_CONFIG_FILE, "w");
		printf_bar(fp);
		iniparser_dump_ini(ini, fp);
		fclose(fp);
	} else {
		fprintf(stderr, "iniparser_load error...\n");
		fprintf(stderr, "Can NOT find \"%s\" file, Can NOT save the param.\n", _CONFIG_FILE);
	}	

	iniparser_freedict(ini);

	return 0;
}

static int printf_bar(FILE *fp)
{
	fprintf(fp,
			"#=============================================================================\n"
			"#\n"
			"#        Author : shmily - shmily@mail.dlut.edu.cn\n"
			"#            QQ : 723103903\n"
			"# Last modified : 2012-12-15 11:32\n"
			"#      Filename : DrowsyDriving.cfg\n"
			"#   Description : not thing...\n"
			"#\n"
			"#=============================================================================\n"
			"\n"
			"#\n"
			"# This is the default param for the CMCC Vg3.0 protocol\n"
			"#\n"
			"\n"	
			"\n");
	return 0;
}

static void Dev_ID_Init(char *pdata)
{
	int i;
	int shift;

	shift = 0;
	for(i=0;i<3;i++){
		shift = (shift << 4) + ((*pdata) - '0');
		pdata++;	
	}
	_DEV_ID_H = (0xFFFFF000 | shift);
	
	shift = 0;
	for(i=0;i<8;i++){
		shift = (shift << 4) + ((*pdata) - '0');
		pdata++;	
	}
	_DEV_ID_L = ( shift);
} 

char * Param_IP(void)
{
	return Dev_Param.ip;
}

char * Param_Port(void)
{
	return Dev_Param.port;
}

int Param_APN(void)
{
	if(strcmp("CMNET", Dev_Param.apn)==0){
		return 0; 
	}

	return 1;
}

int Get_Dev_IDH(void)
{
	return _DEV_ID_H; 
}

int Get_Dev_IDL(void)
{
	return _DEV_ID_L; 
}

int Get_ver(char *p)
{
	memcpy(p, Dev_Param.ver, Dev_Param.ver_len);
	return Dev_Param.ver_len;
}

int Get_ven(char *p)
{
	memcpy(p, Dev_Param.ven, Dev_Param.ven_len);
	return Dev_Param.ven_len;
}

int Get_group_id(char *p)
{
	memcpy(p, Dev_Param.group_id, Dev_Param.group_id_len);
	return Dev_Param.group_id_len;
}
