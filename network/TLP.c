/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-12-19 19:50
#      Filename : TLP.c
#   Description : not thing...
#
=============================================================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "TLP.h"
#include "Timestamp.h"
#include "SequenceNumber.h"
#include "param.h"
#include "GSM_Hal.h"
#include "UDP_Lib.h"
#include "crc16.h"

static Packet_Struct    Packet;
static int              _Link_Active = 0;   // we use this to count Link Active between every two heartbeat,
                                            // if _Link_Active is equ 0, we need to send a heartbeat packet,
                                            // else, we set it to zero and continue.

static void Repack_TLP_Head(char *pbuff, PacketInfo *pInfo);

TLP_Error TLP_SendPacket(PacketInfo *pInfo)
{
	GSM_Error		res;
	int 		    crc_resault;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;
	
	PacketHead = (TLP_Head_Struct *)&(Packet.Data[0]);
	pData      = (uint8_t *)&(Packet.Data[0]) + sizeof(TLP_Head_Struct);
	
	// initial the tlp head
	Repack_TLP_Head( (char *)Packet.Data, pInfo );

	// add option head data
	if( pInfo->Option_len > 0 ){
		memcpy(pData, pInfo->Option_data, pInfo->Option_len);
		pData += pInfo->Option_len; 
	}

	// add app data
	if( pInfo->App_len > 0 ){
		memcpy(pData, pInfo->App_data, pInfo->App_len);
		pData += pInfo->App_len; 
	}

	// prepare to send packet
	Packet.length = pData - ((uint8_t *)&(Packet.Data[0]));
	
	// get the crc16
	pData       = (uint8_t *)&(Packet.Data[0]) + 2;	
	crc_resault = Caculate(pData, (Packet.length-2));
	DEBUG("crc_resault = 0x%08X\n",crc_resault);
    crc_resault = crc_resault&0xFFFF; 
	PacketHead->crc16 = HTONS(crc_resault);

#ifdef __DEBUG__
		int i;
		
		fprintf(stderr, "data length = %d\n",Packet.length);
		fprintf(stderr, "[Send Packet]\n");
		fprintf(stderr, "(*)TLP Head : ");

		for(i=0; i<24; i++){	
			fprintf(stderr, "0x%02X ", Packet.Data[i]);
		}
		fprintf(stderr, "\n");
		
		fprintf(stderr, "[*]APP data : ");
		
		for(i=24; i<Packet.length; i++){	
			fprintf(stderr, "0x%02X ", Packet.Data[i]);
			if( (((i-23)%16)==0) && (i!=24) ){ 
				fprintf(stderr, "\n");
				fprintf(stderr, "              ");
			}
		}
		fprintf(stderr, "\n\n");
#endif

	// send packet
	res = UDP_SendPacket((char *)&(Packet.Data[0]), Packet.length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : ACK ...Error!\n",__func__);
		return ERROR_SEND;
	} else {
        _Link_Active++;
		DEBUG("%s : ACK ...OK\n",__func__);
		return ERROR_NONE;
	}
}

int is_Link_Active(void)
{
    int tmp;

    tmp = (_Link_Active == 0);
    _Link_Active = 0;

    return tmp; 
}

static void Repack_TLP_Head(char *pbuff, PacketInfo *pInfo)
{
    int      seq;
	uint32_t timestamp;
	TLP_Head_Struct	*PacketHead;

	memset(pbuff, 0, _MAX_PACKET_LEN);			

	PacketHead = (TLP_Head_Struct *)(pbuff);
	
	// no need to change
	PacketHead->version = _VERSION;
	PacketHead->length  = sizeof(TLP_Head_Struct) + pInfo->Option_len;
	PacketHead->type    = pInfo->TransmitType;			// device to server
	PacketHead->medium  = _TransmitType;				// UDP
	PacketHead->encrypt = _Encrypt;						// encrypt none
	PacketHead->reserve = _Reserve;			
	
	PacketHead->DEV_IDH = HTONL( Get_Dev_IDH() );
	PacketHead->DEV_IDL = HTONL( Get_Dev_IDL() );
	
	//need to change
	GetTimestamp(&timestamp);
	PacketHead->timestamp = HTONL(timestamp);			// need to fix


    seq = (pInfo->SEQ_num==0)?(Get_SEQ()):(pInfo->SEQ_num);
	PacketHead->SEQ_num = HTONL( seq );
}


int TLP_ReceivePacket(char *pbuff)
{
	// about packet info
	char	unread_sum;
	char	packet_sum;

	int		link_num;
	int		data_index;
	int		data_len = 0;

	TLP_Head_Struct *pTlpHead;
	int		crc_i;

	GSM_GetPacketInfo(&unread_sum, &packet_sum);

	if(unread_sum>0){
		memset(pbuff, '\0', 1024);
		UDP_ReceivePacket(&link_num, &data_index, &data_len, pbuff);

#ifdef __DEBUG__
		fprintf(stderr, _GREEN_);
		fprintf(stderr, "Receive Packet ...\n");
		fprintf(stderr, "link num    = %d\n",link_num);
		fprintf(stderr, "data index  = %d\n",data_index);
		fprintf(stderr, "data length = %d\n",data_len);

		int i;
		
		fprintf(stderr, "[Receive Packet]\n");
		fprintf(stderr, "(*)TLP Head : ");

		for(i=0; i<24; i++){	
			fprintf(stderr, "0x%02X ", pbuff[i]);
		}
		fprintf(stderr, "\n");
		
		fprintf(stderr, "(*)APP data : ");
		
		for(i=24; i<data_len; i++){	
			fprintf(stderr, "0x%02X ", pbuff[i]);
			if( (((i-23)%16)==0)&&(i!=24) ){ 
				fprintf(stderr, "\n");
				fprintf(stderr, "              ");
			}
		}
		fprintf(stderr, "\n");
		fprintf(stderr, _NONE_);
#else
		//fprintf(stderr, _GREEN_);
		fprintf(stderr, "%s%s%s", _GREEN_, "\nReceive Packet ...\n", _NONE_);
		//fprintf(stderr, _NONE_);
#endif
	
		// check the crc16	
		pTlpHead = (TLP_Head_Struct *)pbuff;
    	
		crc_i = Caculate((char *)(pbuff+2),data_len-2)&0xFFFF;
		crc_i = HTONS(crc_i);
		crc_i = (crc_i & 0x0000FFFF);

    	if(crc_i != pTlpHead->crc16){
        	DEBUG("%s : CRC ERROR!\n",__func__);
			data_len = 0;
		} else {
        	DEBUG("%s : CRC OK!\n",__func__);
		}
	}
		
	return data_len;
}
