/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-02-21 14:45
#      Filename : SpeedMonitor.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "HeartBeat.h"
#include "SpeedMonitor.h"
#include "TimeCompare.h"
#include "TLP.h"
#include "AsyncUpdate.h"
#include "GPS.h"
#include "misc.h"

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

static Speed_Cond_Info current_cond;

// if the last packet did not send over, Packet_Over_Flag is 0,
// we wait for untill it turn to be 1.
static int  Packet_Over_Flag = 1;

       void Speed_Packet_Over_CallBack  (int success);
static void Get_Active_Cond             (Speed_Cond_Info *pInfo);
static int  Check_Current_Cond          (Speed_Cond_Info *pInfo);
static int  Interval_Updata             (Speed_Cond_Info *pInfo);
static void Monitor_Packet_Update       (void);

void SpeedMonitor_init(void)
{
    Packet_Over_Flag = 1;
    memset(&current_cond, 0, sizeof(Speed_Cond_Info));
}

void SpeedMonitor( void )
{
	while(1){

        while(Packet_Over_Flag == 0)
            sleep(2);

		if(current_cond.active == 0x00){		                    // no active rule, try to find one.
	
			Get_Active_Cond(&current_cond);

		}else{                                                      // if we have a active rule, update it.
			
			Check_Current_Cond(&current_cond);	
		}

		sleep(2);				                                    // every 2 sec
	}
}


// find if it is need to update the report.
// tips : compare the speed to find out if it is need to 
//        report.
static void Get_Active_Cond(Speed_Cond_Info *pInfo)
{
    int                     current_speed;
    int                     speed_err;
    SpeedAnomaly_Condition  cond;

    // use mutex to get the speed condition
    Get_Speed_Cond(&cond);                          

    if( (cond.Info.SEQ_num != pInfo->cond.Info.SEQ_num) || 
        (cond.Info.SEQ_num == 0)) {                                     // the cond is update by server
        
        pInfo->active        = 0;
        pInfo->SendCount     = 0;
        pInfo->last_time     = 0;
        pInfo->prv_send_time = 0;

        memcpy(&pInfo->cond, &cond, sizeof(SpeedAnomaly_Condition));
    } else {
        current_speed = Get_Current_Speed();
        speed_err = 0;

        if( pInfo->cond.type == 0x01 ){                                 // over special speed
            if(current_speed > pInfo->cond.speed){
                speed_err = 1;
            }
        } else if( pInfo->cond.type == 0x02 ){                          // belower special speed
            if(current_speed < pInfo->cond.speed){
                speed_err = 1;
            } 
        } else if( pInfo->cond.type == 0x03 ){                          // cross the special speed
            if( ((current_speed > pInfo->cond.speed)&&(pInfo->last_speed < pInfo->cond.speed)) || 
                ((current_speed < pInfo->cond.speed)&&(pInfo->last_speed > pInfo->cond.speed)) ){
                speed_err = 1;
            }
        }

        if(speed_err == 1){
            if(pInfo->cond.type == 0x03){
                pInfo->active    = 1;
                pInfo->SendCount = 0;
            } else {
                pInfo->last_time += 2;

                if(pInfo->last_time >= pInfo->cond.interval){
                    pInfo->active    = 1;
                    pInfo->SendCount = 0;
                }
            }
        } else {
            pInfo->last_time = 0;
        }

        pInfo->last_speed = current_speed;

    }
}

static int Check_Current_Cond(Speed_Cond_Info *pInfo)
{
	int need_update;
    int rule_del = 0;

	// check the rule
    // if the rule is active or not.
	if( pInfo->SendCount > pInfo->cond.Info.Cnt ){
        // we need to del the rule from the list
        rule_del = 1;    
	}

    if(rule_del == 1){      // del the rule

        pInfo->active    = 0;
        pInfo->SendCount = 0;

    } else {                // the rule is still active, update it.

	    need_update = Interval_Updata(pInfo); 
	    if(need_update == 1){
		    Monitor_Packet_Update();
	    }
    }

	return 0;
}


static int Interval_Updata(Speed_Cond_Info *pInfo)
{
	int		need_send;
	time_t	current_time;
	int 	interval;
	int		lastime;

	need_send = 0;
	interval  = pInfo->cond.interval;
	
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


    strcpy(elememt.Tid, (const char *)current_cond.cond.Info.RID);
    Position_Init(&elememt.location);
    elememt.action    = current_cond.cond.Info.Action;
    elememt.speed     = 30;
    elememt.direction = 90;
    elememt.mileage   = 1000;
    elememt.info.TransmitType = _TransmitControl(TypeB, Packet_OUT);
	elememt.info.Option_data  = NULL;
	elememt.info.Option_len   = 0;
    elememt.info.SEQ_num      = current_cond.cond.Info.SEQ_num;

    elememt.Update_callback   = Speed_Packet_Over_CallBack;

    // fill with real gps value
    element_init_real(&elememt);

    Packet_Over_Flag = 0;
	AsyncUpdate_signal(&elememt);
}

void Speed_Packet_Over_CallBack(int success)
{
    current_cond.SendCount += success;
    Packet_Over_Flag = 1;
}
