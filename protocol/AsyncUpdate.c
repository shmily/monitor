/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-14 03:19
#      Filename : AsyncUpdate.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "AsyncUpdate.h"

#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

// mutex for gsm protect
pthread_mutex_t GSM_Mutex = PTHREAD_MUTEX_INITIALIZER;

// mutex for async update protect
pthread_mutex_t Update_Mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  Update_Condition = PTHREAD_COND_INITIALIZER;

// mutex for ACK protect
pthread_mutex_t Btype_ACK_Mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  Btype_ACK_Condition = PTHREAD_COND_INITIALIZER;

static Update_elememt elememt;
static char Update_BUFF[1024]; 


static char *Enpacket_PositionUpdate(char *pbuff, Update_elememt *pelememt);

void AsyncUpdate_Thread_Init(void)
{

}


// 2013-02-20, add Retransmit
void AsyncUpdate( void )
{
    char            *pbuff;
    int             res;

    int             err;
    struct timespec timeout;

    int             ack_timeout;
    int             ack_tries;
    int             send_counter;
    void (*Update_callback)(int);

    while(1)
    {

        pthread_mutex_lock(&Update_Mutex);
        pthread_cond_wait(&Update_Condition, &Update_Mutex);    // wait for a signel to send update packet

        fprintf(stderr, "%s > Start to send position update packet...\n", __func__);
      
        // 1. we create the packet. 
        memset(Update_BUFF, 0, 1024);
        pbuff = Update_BUFF;

	    pbuff = Enpacket_PositionUpdate(pbuff, &elememt);

        elememt.info.App_data     = (uint8_t *)Update_BUFF;
	    elememt.info.App_len      = (int)(pbuff - (char *)Update_BUFF);

        send_counter = 0;   // prepare for retransmit
        ack_timeout  = 15;
        ack_tries    = 3;
        Update_callback = elememt.Update_callback;

        // 2. send the packet. 
__Send_Packet:
        GSM_mutex_lock();
	   
        send_counter++; 
        res = TLP_SendPacket(&elememt.info);
	    if(res!=ERROR_NONE){
		    DEBUG("%s : Position update packet send ...Error!\n",__func__);
	    } else {
		    DEBUG("%s : Position update packet send ...OK\n",__func__);
	    }

        GSM_mutex_unlock();
    
        // 3. wait for the server ack.
        timeout.tv_sec  = time(NULL) + ack_timeout;
        timeout.tv_nsec = 0; 

        // wait for the ack signal.
        pthread_mutex_lock(&Btype_ACK_Mutex);
		DEBUG("%s : Wait for Ack time out, %d\n",__func__, (int)time(NULL));
        err = pthread_cond_timedwait(&Btype_ACK_Condition, &Btype_ACK_Mutex, &timeout); 
        pthread_mutex_unlock(&Btype_ACK_Mutex);

        if(err == ETIMEDOUT){       // ack timeout
		    DEBUG("%s : Wait for Ack time out, %d\n",__func__, (int)time(NULL));
            if(send_counter > ack_tries)    Update_callback(0);
            else                            goto __Send_Packet; 
        } else {                    // server return the ack
		    DEBUG("%s : Ack return, %d\n",__func__, (int)time(NULL));
            Update_callback(1);
        }

        pthread_mutex_unlock(&Update_Mutex);
    }
}

void AsyncUpdate_signal(Update_elememt *elememt)
{
    pthread_mutex_lock(&Update_Mutex);

    Update_elememt_copy(elememt);
    pthread_cond_signal(&Update_Condition);
    fprintf(stderr, "%s > Request to send position update packet...\n", __func__);

    pthread_mutex_unlock(&Update_Mutex);
}

void GSM_mutex_lock(void)
{ 
    pthread_mutex_lock(&GSM_Mutex);
}

void GSM_mutex_unlock(void)
{ 
    pthread_mutex_unlock(&GSM_Mutex);
}

void Update_elememt_copy(Update_elememt *src)
{
    memcpy(&elememt, src, sizeof(Update_elememt));
}



static char *Enpacket_PositionUpdate(char *pbuff, Update_elememt *pelememt)
{

    if(pelememt->action==0x00){         // position update
        pbuff = Async_Enpacket_DefaultAPP(pbuff, 0x0001, 0);
        pbuff = Enpacket_TID_AKV(pbuff, pelememt->Tid);
    }else{   // report update
        pbuff = Async_Enpacket_DefaultAPP(pbuff, 0x0003, 0);
        pbuff = Enpacket_Rname_AKV(pbuff, pelememt->action);

        if(pelememt->Tid[0] != '\0')
            pbuff = Enpacket_RID_AKV(pbuff, pelememt->Tid);
    }

    pbuff = Enpacket_PI_AKV(pbuff, &pelememt->location);
    pbuff = Enpacket_S_AKV(pbuff, pelememt->speed);
    pbuff = Enpacket_D_AKV(pbuff, pelememt->direction);
    pbuff = Enpacket_ME_AKV(pbuff, pelememt->mileage);

    return pbuff;
}


void AsyncUpdate_ACK(int error)
{
    if(error == 0x00){
        pthread_mutex_lock(&Btype_ACK_Mutex);
        pthread_cond_signal(&Btype_ACK_Condition);
        pthread_mutex_unlock(&Btype_ACK_Mutex); 
    } else { 
		DEBUG("%s : Btype update error, error code : 0x%X\n",__func__, error);
    }
}

