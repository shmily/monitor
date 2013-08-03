/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-12-25 14:52
#      Filename : InputFilter.c
#   Description : not thing...
#
=============================================================================*/

#include "memory.h"
#include "TLP.h"
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "DeviceControl.h"
#include "ImageTransmit.h"
#include "HeartBeat.h"

Packet_Struct	InputPacket;	// .length; .Data[1024];

static int      tog = 0;
const  char *   runing[2] = {"+", "x"};


extern void AsyncUpdate_ACK(int error);
void TypeA_Spliter(Packet_Struct *pPacket);

// packet devide
TLP_Error InputPacket_Process(void)
{
	TLP_Head_Struct *pTlpHead;
    char            *perror;
    int             error;
	uint32_t		len;
	TLP_Error		res;

    len = TLP_ReceivePacket( (char*)InputPacket.Data );
    
	if(len>0){		// receive a packet
		
		InputPacket.length = len;	
		pTlpHead = (TLP_Head_Struct *)( InputPacket.Data );
        perror = (char *)(InputPacket.Data + sizeof(TLP_Head_Struct));
        error = *(perror); 
		res = ERROR_NONE;

		switch (pTlpHead->type & 0x70)
		{
			case TypeA :
				DEBUG("%s : This is a Type A transmit packet...\n",__func__);
	            TypeA_Spliter(&InputPacket);			
                break;
	
			case TypeD :
				DEBUG("%s : This is a Type D transmit packet...\n",__func__);	
				TypeD_InputPacketHandle(&InputPacket);
                break;

			case TypeB :
			    DEBUG("%s : This is a Type B transmit packet...\n",__func__);	
				AsyncUpdate_ACK(error);
                break;
	
			default :
				DEBUG("%s : Unknow Input packet...\n",__func__);	
				res = ERROR_UNKNOW;
		}
		
	} else {		// receive faild

        tog++;
		fprintf(stderr, "\r");
		fprintf(stderr, "%s : %s", __func__, runing[tog%2]);
		res = ERROR_UNKNOW;
	}
	
	return res;
}


// divide
// 0x11 --> report condition

void TypeA_Spliter(Packet_Struct *pPacket)
{
	char comm_type;
	char *pdata;

	pdata = (char *)(pPacket->Data + sizeof(TLP_Head_Struct));
	comm_type = *pdata;

	if( comm_type == _TypeA_ReportCondition ){
		
        ReportPacket_Handle( pPacket );
	
    } else if( (comm_type == _TypeA_PositionRule_Set) || 
			   (comm_type == _TypeA_PositionRule_Del) ) {
        
        RulePacket_Handle(pPacket);

	} else if( comm_type == _TypeA_DeviceControl ){
        
        DeviceControl_Handle(pPacket);
    } else if( comm_type == _TypeA_HeartBeat ){
        
        HeartBeat_Handle(pPacket);
    }
}
