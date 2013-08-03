/*
===============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-05-19 22:26
#      Filename : FatigueAlarm.c
#   Description : not thing...
#
===============================================================================
*/
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "HeartBeat.h"
#include "FatigueAlarm.h"
#include "TimeCompare.h"
#include "TLP.h"
#include "AsyncUpdate.h"
#include "GPS.h"
#include "misc.h"

#include "SequenceNumber.h"

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>


// if the last packet did not send over, Packet_Over_Flag is 0,
// we wait for untill it turn to be 1.
static int  Packet_Over_Flag = 1;
static Fatigue_Cond_Info current_cond;

extern int  is_Fatigue(void);

void        Fat_Packet_Over_CallBack(int success);
static void Get_Active_Cond(Fatigue_Cond_Info *pInfo);
static int  Interval_Updata(Fatigue_Cond_Info *pInfo);
static int  Check_Current_Cond(Fatigue_Cond_Info *pInfo);
static void Monitor_Packet_Update(void);

void FatigueMonitor_init(void)
{
    memset(&current_cond, 0, sizeof(Fatigue_Cond_Info));
    current_cond.cond.Interval = 30;
    current_cond.cond.Cnt      = 3;
    current_cond.cond.Action   = 0x0014;
}

void FatigueMonitor(void)
{
    while(1){
    
        // wait for the last transmit finish.
        while(Packet_Over_Flag == 0){
            sleep(2);
        }

        if(current_cond.active == 0x00){
            Get_Active_Cond(&current_cond);
        } else {
            Check_Current_Cond(&current_cond);
        }

        sleep(2);
    }
}

static void Get_Active_Cond(Fatigue_Cond_Info *pInfo)
{

    pInfo->SendCount     = 0;    
    pInfo->prv_send_time = 0;    

	if(is_Fatigue()==1){

		DEBUG("%s : > we need to update the Fatigue Report...\n", __func__);
		pInfo->active = 1;
	}
}

static int Check_Current_Cond(Fatigue_Cond_Info *pInfo)
{
	int need_update;
    int rule_del = 0;

	// check the rule
    // if the rule is active or not.
	if( pInfo->SendCount > pInfo->cond.Cnt ){        
        rule_del = 1;
	}

    if(rule_del == 1){      // del the rule
		
        DEBUG("%s : > Dell the rule.\n", __func__);

        pInfo->active        = 0;
        pInfo->SendCount     = 0;    
        pInfo->prv_send_time = 0;    

    } else {                // the rule is still active, update it.

	    need_update = Interval_Updata(pInfo); 
	    if(need_update == 1){
		    Monitor_Packet_Update();
	    }
    }

	return 0;
}


static int Interval_Updata(Fatigue_Cond_Info *pInfo)
{
	int		need_send;
	time_t	current_time;
	int 	interval;
	int		lastime;

	need_send = 0;
	interval  = pInfo->cond.Interval;

	time(&current_time);
	lastime  = pInfo->prv_send_time; 

	if( (current_time - lastime) > interval ) {
	    need_send = 1;
	    pInfo->prv_send_time = current_time;
	}

	return need_send;
}

static void Monitor_Packet_Update(void)
{
	Update_elememt  elememt;
    
	memset(&elememt, 0, sizeof(Update_elememt));

	Position_Init(&elememt.location);
	elememt.action    = 0x0014;
	elememt.speed     = 30;
	elememt.direction = 90;
	elememt.mileage   = 1000;
	elememt.info.TransmitType = _TransmitControl(TypeB, Packet_OUT);
	elememt.info.Option_data  = NULL;
	elememt.info.Option_len   = 0;
	elememt.info.SEQ_num      = Get_SEQ();

    elememt.Update_callback   = Fat_Packet_Over_CallBack;

    // fill with real gps value
    element_init_real(&elememt);

    Packet_Over_Flag = 0;
	AsyncUpdate_signal(&elememt);
}

void Fat_Packet_Over_CallBack(int success)
{
    current_cond.SendCount += success;
    Packet_Over_Flag = 1;
}
