/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-16 00:36
#      Filename : HeartBeat.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "HeartBeat.h"

#include <sys/time.h>
#include <pthread.h>

extern pthread_mutex_t GSM_Mutex;
pthread_cond_t  HeartBeat_Send = PTHREAD_COND_INITIALIZER;
pthread_cond_t  HeartBeat_ACK  = PTHREAD_COND_INITIALIZER;

static int  Lose_packet_cnt = 0;
static char HeartBeat_BUFF[1024]; 

void HeartBeat( void )
{
    PacketInfo  info;
    char        *pbuff;
    int         res;
    
    struct timeval  current;
    struct timespec timeout;

    while(1)
    {
        pthread_mutex_lock(&GSM_Mutex);

        gettimeofday(&current, NULL);
        timeout.tv_sec  = current.tv_sec + 15;
        timeout.tv_nsec = current.tv_usec * 1000; 

        // wait for a signel to send heartbeat packet
        pthread_cond_timedwait(&HeartBeat_Send, &GSM_Mutex, &timeout); 

        // check if we need to send a heartbeat packet.
        if( is_Link_Active() ){

            fprintf(stderr, "%s%s%s", _YELLOW_, "\n>> Send heart beat packet...\n", _NONE_);
       
            memset(HeartBeat_BUFF, 0, 1024);
            pbuff = HeartBeat_BUFF;
        
            *(pbuff++) = 0x41;
            *(pbuff++) = 0x00;
            *(pbuff++) = 0x03;

	        info.TransmitType = _TransmitControl(TypeA, Packet_OUT);
	        info.Option_data  = NULL;
	        info.Option_len   = 0;
            info.App_data     = (uint8_t *)HeartBeat_BUFF;
	        info.App_len      = (int)(pbuff - (char *)HeartBeat_BUFF);

            // send packet
	        res = TLP_SendPacket(&info);
            Lose_packet_cnt++;

	        if(res!=ERROR_NONE){
		        DEBUG("%s : Heart beat packet send ...Error!\n",__func__);
	        } else {
		        DEBUG("%s : Heart beat packet send ...OK\n",__func__);
            }
        }

        pthread_mutex_unlock(&GSM_Mutex);
    }
}


TLP_Error HeartBeat_Handle( Packet_Struct *pPacket )
{
	char    *pdata;
    int     type;
    int     ret;

	pdata = (char *)(pPacket->Data + sizeof(TLP_Head_Struct));		// pointer to the app data

    type = ((int)*(pdata+1))&0x000000FF;
    type = (type << 8) | (((int)*(pdata+2))&0x000000FF);

    ret = *(pdata+3);
    if((type == 0x0003)&&(ret == 0x01)){
        Lose_packet_cnt = 0;
    }

    return ERROR_NONE;
}

int is_lose_connection(void)
{
    if(Lose_packet_cnt > 10){
        Lose_packet_cnt = 0;
        return 1;
    }else{
        return 0;
    }
}
