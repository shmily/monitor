/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-12-29 10:52
#      Filename : RC_Emergency.c
#   Description : not thing...
#
=============================================================================*/

#include <memory.h>
#include <pthread.h>
#include "ReportCondition.h"

static pthread_mutex_t Emer_Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t Emer_Copy  = PTHREAD_MUTEX_INITIALIZER;

Report_Info Emergency_Info;

static int isNeed_Update = 0;
static TLP_Error RC_Emergency_ACK( void );

// Emergency report input packet process
// vg3.0 (page.102)
TLP_Error RC_Emergency_Parser( Packet_Struct *pPacket )
{
    TLP_Error       res;
    char 		    *pdata;
	Report_Info     Info;

	DEBUG("%s : Emergency Packet receive ...\n",__func__);
	DEBUG("%s : Start to parse Packet ...\n",__func__);

    memset(&Info, 0, sizeof(Report_Info));
	pdata = (char *)(pPacket->Data);		                        // pointer to the app data

    isNeed_Update = 0x01;

	pdata = RC_Parse_CommInfo(pdata, &Info);						// parse the comm info

    pthread_mutex_lock(&Emer_Copy);
	memcpy(&Emergency_Info, &Info, sizeof(Report_Info));
    pthread_mutex_unlock(&Emer_Copy);

    res = RC_Emergency_ACK();


    pthread_mutex_lock(&Emer_Mutex);
    isNeed_Update = 1;
    pthread_mutex_unlock(&Emer_Mutex);

	return res;
}

TLP_Error RC_Emergency_ACK( void )
{
	TLP_Error	res;
	PacketInfo	info;
	char *		buff;
	char *		pdata;
	
	buff = malloc(1024);

	pdata = RC_Enpacket_DefaultAPP(buff, &Emergency_Info);

	if( Emergency_Info.Need_Confirm == 0x01 ){
		pdata = RC_Enpacket_DefaultAKV(pdata, &Emergency_Info);	
	}

	info.TransmitType = _TransmitControl(TypeA, Packet_OUT);
    info.SEQ_num      = Emergency_Info.SEQ_num;
    info.Option_data  = NULL;
	info.Option_len   = 0;
	info.App_data     = (uint8_t *)buff;
	info.App_len      = (int)(pdata - (char *)buff);

    // send packet
	res = TLP_SendPacket(&info);

	if(res!=ERROR_NONE){
		DEBUG("%s : Emergency ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Emergency ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}

int isNeed_Update_Emergency(void)
{
    int need;


    pthread_mutex_lock(&Emer_Mutex);
    need = isNeed_Update;
    isNeed_Update = 0x00;
    pthread_mutex_unlock(&Emer_Mutex);

    return need;
}

void Copy_Emergency_Info(Report_Info *pInfo)
{
    pthread_mutex_lock(&Emer_Copy);
    memcpy(pInfo, &Emergency_Info, sizeof(Report_Info));
    pthread_mutex_unlock(&Emer_Copy);
}


int Get_Emergency_SEQ_num(void)
{
    return Emergency_Info.SEQ_num;
}

