/*
===============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-02 00:14
#      Filename : TypeD_Input.c
#   Description : not thing...
#
===============================================================================
*/

#include <stdint.h>
#include <math.h>
#include "UDP_Lib.h"
#include "GSM_Hal.h"
#include "GSM-error.h"
#include "TLP.h"
#include "DtypeTransmit.h"

#include <memory.h>
#include <semaphore.h>
#include <assert.h>
#include <time.h>
#include <errno.h>

Retry_Info		RetryPacket;

static void Printf_RtryPacket(Retry_Info *pInfo);
static void DT_UpdateReqACK(char *pData, char *pACK);
static void DT_PacketACK(char *pData, Retry_Info *pInfo);
static void DT_DisconnetcACK(char *pData, char *error_code);


// (2013.03.01 add by shmily)
enum DT_ErrorCode_t  ErrorCode;
enum DT_Status_t     CurrentStatus;

// sem_t for wait for the ack.
sem_t sem_update_ack;       
sem_t sem_packet_ack;       
sem_t sem_discon_ack;       

// init at the setup main.c
void TypeD_Transmit_Init(void)
{
    memset(&RetryPacket, 0, sizeof(Retry_Info));
    ErrorCode     = ERR_INIT;
    CurrentStatus = S_INIT;

    sem_init(&sem_update_ack, 0, 0);
    sem_init(&sem_packet_ack, 0, 0);
    sem_init(&sem_discon_ack, 0, 0);
}

// handle by ../protocol/InputFilter.c
void TypeD_InputPacketHandle(Packet_Struct *pInput)
{
	TLP_Head_Struct *pHead;
	char  			*pReceive;
	char			ACK_error;
	
	pHead = (TLP_Head_Struct *)(pInput->Data);
	pReceive = (char *)(pInput->Data + sizeof(TLP_Head_Struct));
	
	switch(pHead->type){

		case _DT_UPDATE_ACK:

			DEBUG("%s : > Connection REQ ACK...!\n",__func__);
            if( CurrentStatus == S_UPDATE_ACK ){
			    DT_UpdateReqACK(pReceive, &ACK_error);
			    if(ACK_error==0x00){
                    ErrorCode = ERR_UPDATE_ACC;
			    } else {
                    ErrorCode = ERR_UPDATE_DENY;
			    }
                
                sem_post(&sem_update_ack);
            }
			break;
		
		case _DT_PACKET_ACK:
		case _DT_RSEND_ACK:
		case _DT_RCONNECT_ACK:

			DEBUG("%s : > Packet updata ACK...!\n",__func__);
			if( CurrentStatus == S_PACKET_ACK ){    
                DT_PacketACK(pReceive, &RetryPacket);
                ErrorCode = ERR_PACKET_ACK;
                sem_post(&sem_packet_ack);
            }
           
            break;
		
		case  _DT_DISCONNECT_ACK:

			DEBUG("%s : > Disconnect REQ ACK...!\n",__func__);
			if( CurrentStatus == S_DISCON_ACK ){
                DT_DisconnetcACK(pReceive, &ACK_error);
                if(ACK_error == 0x00){
                    ErrorCode = ERR_DISCON_OK;
                } else { 
                    ErrorCode = ERR_DISCON_ERROR;
                }

                sem_post(&sem_discon_ack);
            }

			break;
			
		default:
			DEBUG("%s : > Unknow packet...!\n",__func__);
			break;
	}
}

static void DT_UpdateReqACK(char *pData, char *pACK)
{
	(*pACK) = *(pData+16);
}


static void DT_PacketACK(char *pData, Retry_Info *pInfo)
{
		int i;
		int tmp;

		pData = pData + 16 + 2; 	
		pInfo->error_code = *(pData++);
		pInfo->packet_num = *(pData++);
	
		if(pInfo->error_code == 0x01){
			pInfo->packet_num = 0;
		} else if(pInfo->error_code == 0x02){
			for(i=0; i<pInfo->packet_num; i++){
			
				tmp = *(pData++);
				tmp = (tmp<<8) + *(pData++);
				pInfo->packetIndex[i] = tmp;
			}
		} else if(pInfo->error_code == 0x03){
			pInfo->packet_num = 16;
		}

    Printf_RtryPacket(pInfo);
}


static void DT_DisconnetcACK(char *pData, char *error_code)
{	
	(*error_code) = *(pData);
}

static void Printf_RtryPacket(Retry_Info *pInfo)
{
	int i;
	
	fprintf(stderr, "\r\n>>\r\n");
	fprintf(stderr,"%s : > error_code    = 0x%02X\n", __func__, pInfo->error_code);
	fprintf(stderr,"%s : > packet_num    = 0x%02X\n", __func__, pInfo->packet_num);
	fprintf(stderr,"%s : > Resend Packet : ", __func__);
	for(i=0; i<pInfo->packet_num; i++){
		fprintf(stderr,"[%d] ", pInfo->packetIndex[i]);
	}
	fprintf(stderr, "\r\n");
}
