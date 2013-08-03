/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-08 04:13
#      Filename : PU_StopMonitoring.c
#   Description : not thing...
#
=============================================================================*/

#include <memory.h>
#include "ReportCondition.h"

Rule_Info  DeleteID;

static TLP_Error PU_StopMonitoring_ACK( void );

// del the monitoring rule
// vg3.0 (page.96)
TLP_Error PU_StopMonitoring_Parser( Packet_Struct *pPacket )
{
    TLP_Error   res;
	char 		*pdata;

	DEBUG("%s : Stop monitoring Packet receive ...\n",__func__);
	DEBUG("%s : Start to parse Packet ...\n",__func__);

	pdata = (char *)(pPacket->Data);		                            // pointer to the app data
    
    PU_Parse_CommInfo(pdata, &DeleteID);

    res = PU_StopMonitoring_ACK();

	return res;
}

static TLP_Error PU_StopMonitoring_ACK( void )
{
	TLP_Error	res;
	PacketInfo	info;
	char *		buff;
	char *		pdata;
	
	buff = malloc(1024);

	pdata = PU_Enpacket_DefaultAPP(buff, &DeleteID);

    info.TransmitType = _TransmitControl(TypeA, Packet_OUT);
    info.SEQ_num      = DeleteID.SEQ_num;
	info.Option_data  = NULL;
	info.Option_len   = 0;
	info.App_data     = (uint8_t *)buff;
	info.App_len      = (int)(pdata - (char *)buff);

    // send packet
	res = TLP_SendPacket(&info);

	if(res!=ERROR_NONE){
		DEBUG("%s : Stop monitoring ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Stop monitoring ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}
