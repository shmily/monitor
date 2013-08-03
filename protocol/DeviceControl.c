/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-14 22:04
#      Filename : DeviceControl.c
#   Description : not thing...
#
=============================================================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "ReportCondition.h"
#include "TLP.h"
#include "AsyncUpdate.h"
#include "misc.h"

static int  isNeed_Update   = 0;    // if we got a roll call, update it to 0x01;
static int  prv_SEQ_num     = 0;


static TLP_Error DC_RollCall_Parser( Packet_Struct *pPacket );
static TLP_Error DC_RollCall_ACK( void );

TLP_Error DeviceControl_Handle( Packet_Struct *pPacket )
{
	char    *pdata;
    int     type;
    int     res;

	pdata = (char *)(pPacket->Data + sizeof(TLP_Head_Struct));		// pointer to the app data

    type = ((int)*(pdata+1))&0x000000FF;
    type = (type << 8) | (((int)*(pdata+2))&0x000000FF);

    if(type == 0x0002){
        res = DC_RollCall_Parser(pPacket);
        res = DC_RollCall_ACK();
    }

    return res;
}

static TLP_Error DC_RollCall_Parser( Packet_Struct *pPacket )
{
	char 		    *pdata;
	TLP_Head_Struct *pTlpHead;
    int             seq;

	DEBUG("%s : Roll call Packet receive ...\n",__func__);
	DEBUG("%s : Start to parse Packet ...\n",__func__);

    pTlpHead = (TLP_Head_Struct *)pPacket->Data;
	pdata = (char *)(pPacket->Data + sizeof(TLP_Head_Struct));		// pointer to the app data

    seq = pTlpHead->SEQ_num;
    prv_SEQ_num = HTONL(seq);

    isNeed_Update = 0x01;

	return ERROR_NONE;
}

static TLP_Error DC_RollCall_ACK( void )
{
	TLP_Error	res;
	PacketInfo	info;
	char *		buff;
	char *		pdata;
	
	buff = malloc(1024);
    pdata = buff;

    (*pdata++) = 0x01;
    (*pdata++) = 0x00;
    (*pdata++) = 0x02;
    (*pdata++) = 0x01;

	info.TransmitType = _TransmitControl(TypeA, Packet_OUT);
	info.Option_data  = NULL;
    info.SEQ_num      = prv_SEQ_num;
	info.Option_len   = 0;
	info.App_data     = (uint8_t *)buff;
	info.App_len      = (int)(pdata - (char *)buff);

    // send packet
	res = TLP_SendPacket(&info);

	if(res!=ERROR_NONE){
		DEBUG("%s : Roll call ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Roll call ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}

int isNeed_Updata_Position(void)
{
    int need;

    need = isNeed_Update;
    isNeed_Update = 0x00;

    return need;
}

int Get_RollCall_SEQ_num(void)
{
    return prv_SEQ_num;
}



// add at 2013.03.04

static int LoginACK = 0;
static int LoginOk  = 0;
void call_position_callback(int error)
{
    LoginOk  = error;
    LoginACK = 1;
}

int Position_update_after_Call(void)
{
    Update_elememt  elememt;

    memset(&elememt, 0, sizeof(Update_elememt));

    strcpy(elememt.Tid, "CALL");
    Position_Init(&elememt.location);
    elememt.action    = 0;
    elememt.speed     = 30;
    elememt.direction = 90;
    elememt.mileage   = 1000;
	elememt.info.TransmitType = _TransmitControl(TypeB, Packet_OUT);
	elememt.info.Option_data  = NULL;
	elememt.info.Option_len   = 0;
    elememt.info.SEQ_num      = Get_RollCall_SEQ_num();
	elememt.Update_callback   = call_position_callback;

    // fill with real gps value
    element_init_real(&elememt);

    AsyncUpdate_signal(&elememt);

    /*
    // wait for ok
    while(LoginACK == 0);

    if(LoginOk == 1){
        printf("%s : Position update ok.\n", __func__);
        ret = 0;
    } else {
        printf("%s : Position update ERROR!\n", __func__);
        ret = -1;
    }*/

    return 0;
}
