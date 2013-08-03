/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-13 17:28
#      Filename : ZoneMonitor.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "HeartBeat.h"
#include "ZoneMonitor.h"
#include "TimeCompare.h"
#include "TLP.h"
#include "AsyncUpdate.h"
#include "GPS.h"
#include "ZoneInfo.h"
#include "misc.h"

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

extern struct list_head InZoneList;
static   Zone_Cond_Info current_cond;

// if the last packet did not send over, Packet_Over_Flag is 0,
// we wait for untill it turn to be 1.
static int  Zone_Packet_Over_Flag = 1;

       void Zone_Packet_Over_CallBack (int success);
static void Get_Active_Cond           (struct list_head *pHead, const GpsInfo *pGPS, Zone_Cond_Info *pInfo, int inzone);
static int  Check_Current_Cond        (Zone_Cond_Info *pInfo);
static int  Interval_Updata           (Zone_Cond_Info *pInfo);
static void Monitor_Packet_Update     (void);

void InZoneMonitor_init(void)
{
    Zone_Packet_Over_Flag = 1;
    memset(&current_cond, 0, sizeof(Zone_Cond_Info));
}

void InZoneMonitor( void )
{
    GpsInfo gps_info;

	while(1){

        while(Zone_Packet_Over_Flag == 0);

		if(current_cond.active == 0x00){		                    // no active rule, try to find one.

            Get_Current_GPS(&gps_info);    
			Get_Active_Cond(&InZoneList, &gps_info, &current_cond, 1);

		}else{                                                      // if we have a active rule, update it.
			
			Check_Current_Cond(&current_cond);	
		}

		sleep(2);				                                    // every 2 sec
	}
}


// for position update rule, every rule is start by time, so we can
// test the begin time, if current time is over the begin time, we
// find the active rule.
// Tips: 
//       inzone : 1 --> in zone detect
//       inzone : 0 --> out zone detect
//
static void Get_Active_Cond(struct list_head *pHead, const GpsInfo *pGPS, Zone_Cond_Info *pInfo, int inzone)
{
	struct list_head 		*plist;
	Zone_Condition 	        *pCondition;
	Time_D					*pTime;
	
	int						mach = 0;
	int 					TimeSet_sum;
	int 					i;
	TimeSlot_Struct			*pTimeSlot;

	char					Zone_Shape;

	// first, we find a active time proid

	if(pInfo->Time_active == 1){				            // we aleady have a condition, we check it
		
		pCondition = pInfo->pcond;
		pTime = (Time_D *)&((pCondition->TimeSet).BE_Time[(pInfo->time_set_index)].Etime);

		if( Is_BeyondTime_D( pTime ) == 1 ){				// out of the time

			DEBUG("%s : >>>> Out of the Etime...\n", __func__);
			
            pInfo->active         = 0;
			pInfo->Time_active    = 0;
			pInfo->SendCount      = 0;
            pInfo->last_send_time = 0;
			
            pCondition   = NULL;
			pInfo->pcond = NULL;
		}

	} else {	// else, we search for a active condition

        memset(pInfo, 0, sizeof(Zone_Cond_Info));

        InZone_Cond_lock();
		list_for_each(plist, pHead){

			pCondition = list_entry(plist, Zone_Condition, list);

			TimeSet_sum = pCondition->TimeSet.TimeSet_Count;

			for(i=0; i<TimeSet_sum; i++){
			
				pTimeSlot = &(pCondition->TimeSet.BE_Time[i]);

				if( (Is_BeyondTime_D( &(pTimeSlot->Btime) ) == 1) && 
					(Is_BeyondTime_D( &(pTimeSlot->Etime) ) == 0) ) {
					
					mach = 1;
					break;
				}
			}

			if(mach == 1){
				break;
			}

		}
        InZone_Cond_unlock();


		if(plist!=pHead){	// we get the rule

			DEBUG("%s : > we get the rule...\n", __func__);
			pInfo->Time_active   = 1;
			pInfo->time_set_index = i;
			pInfo->pcond          = pCondition;
		}
	}

	// second, if we are in the active time proid, detetive if in the zone
	if(pInfo->Time_active == 1){

		Zone_Shape = pCondition->shape;

		if(Is_InZone(Zone_Shape, pCondition->pShape_Value, (GpsInfo *)pGPS) == 1){

			pInfo->active = ( (inzone==1) ? 1 : 0 );
		} else {

			pInfo->active = ( (inzone==1) ? 0 : 1 );
		}
	}
}

static int Check_Current_Cond(Zone_Cond_Info *pInfo)
{
	int need_update;
    int rule_del = 0;

	// check the rule
    // if the rule is active or not.
	if( pInfo->SendCount > pInfo->pcond->Info.Cnt ){
        // we need to del the rule from the list
        rule_del = 1;    
	}

    if(rule_del == 1){      // del the rule

        pInfo->SendCount = 0;
        pInfo->active = 0;

    } else {                // the rule is still active, update it.

	    need_update = Interval_Updata(pInfo); 
	    if(need_update == 1){
		    Monitor_Packet_Update();
	    }
    }

	return 0;
}


static int Interval_Updata(Zone_Cond_Info *pInfo)
{
	int		need_send;
	time_t	current_time;
	int 	interval;
	int		lastime;

	need_send = 0;
	interval  = pInfo->pcond->Info.Interval;
	
	time(&current_time);
	lastime  = pInfo->last_send_time; 


//	DEBUG("%s : > interval = %d\n", __func__, (int)interval);
//	DEBUG("%s : > current  = %d\n", __func__, (int)current_time);
//	DEBUG("%s : > last     = %d\n", __func__, (int)lastime);

	if( (current_time - lastime) > interval ) {
	    need_send = 1;
		pInfo->last_send_time = current_time;
	}

	return need_send;
}

static void Monitor_Packet_Update(void)
{
	Update_elememt  elememt;
    
	memset(&elememt, 0, sizeof(Update_elememt));

    strcpy(elememt.Tid, (const char *)current_cond.pcond->Info.RID);
    Position_Init(&elememt.location);
    elememt.action    = current_cond.pcond->Info.Action;
    elememt.speed     = 30;
    elememt.direction = 90;
    elememt.mileage   = 1000;
    elememt.info.TransmitType = _TransmitControl(TypeB, Packet_OUT);
	elememt.info.Option_data  = NULL;
	elememt.info.Option_len   = 0;
    elememt.info.SEQ_num      = current_cond.pcond->Info.SEQ_num;

    elememt.Update_callback   = Zone_Packet_Over_CallBack;

    // fill with real gps value
    element_init_real(&elememt);

    Zone_Packet_Over_Flag = 0;
	AsyncUpdate_signal(&elememt);
}


void Zone_Packet_Over_CallBack(int success)
{
    current_cond.SendCount += success;
    Zone_Packet_Over_Flag = 1;
}
