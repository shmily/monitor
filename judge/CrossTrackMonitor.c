/*
=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-13 18:02
#      Filename : CrossTrackMonitor.c
#   Description : not thing...
#
=============================================================================
*/

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "HeartBeat.h"
#include "CrossTrackMonitor.h"
#include "TimeCompare.h"
#include "TLP.h"
#include "AsyncUpdate.h"
#include "GPS.h"
#include "ZoneInfo.h"
#include "misc.h"

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

extern struct list_head CrossTrackList;
static  Track_Cond_Info current_cond;

// if the last packet did not send over, Packet_Over_Flag is 0,
// we wait for untill it turn to be 1.
static int  Track_Packet_Over_Flag = 1;

       void Track_Packet_Over_CallBack (int success);
static void Get_Active_Cond            (struct list_head *pHead, const GpsInfo *pGPS, Track_Cond_Info *pInfo);
static int  Check_Current_Cond         (Track_Cond_Info *pInfo);
static int  Interval_Updata            (Track_Cond_Info *pInfo);
static void Monitor_Packet_Update      (void);
static int  Get_CrossTrack_Error       (const GpsInfo *pGPS, CrossTrack_Condition *pCond);

void CrossTrackMonitor_init(void)
{
    Track_Packet_Over_Flag = 1;
    memset(&current_cond, 0, sizeof(Track_Cond_Info));
}

void CrossTrackMonitor( void )
{
    GpsInfo gps_info;

	while(1){

        while(Track_Packet_Over_Flag == 0);

		if(current_cond.active == 0x00){		                    // no active rule, try to find one.

            Get_Current_GPS(&gps_info);    
			Get_Active_Cond(&CrossTrackList, &gps_info, &current_cond);

		}else{                                                      // if we have a active rule, update it.
			
			Check_Current_Cond(&current_cond);	
		}

		sleep(5);				                                    // every 2 sec
	}
}

static void Get_Active_Cond(struct list_head *pHead, const GpsInfo *pGPS, Track_Cond_Info *pInfo)
{
	struct list_head 		*plist;
	CrossTrack_Condition    *pCondition;
	Time_D					*pTime;
	
	int						mach = 0;
	int 					TimeSet_sum;
	int 					i;
	TimeSlot_Struct			*pTimeSlot;
    int                     track_err;
    int                     err;

	// first, we find a active time proid

	if(pInfo->Time_active == 1){ // we aleady have a condition, we check it
		
		pCondition = pInfo->pcond;
		pTime = (Time_D *)&((pCondition->TimeSet).BE_Time[(pInfo->time_set_index)].Etime);

		if( Is_BeyondTime_D( pTime ) == 1 ){  // out of the time

			DEBUG("%s : >>>> Out of the Etime...\n", __func__);
			
            pInfo->active         = 0;
			pInfo->Time_active    = 0;
			pInfo->SendCount      = 0;
            pInfo->last_send_time = 0;
			
            pCondition   = NULL;
			pInfo->pcond = NULL;
		}

	} else {	// else, we search for a active condition

        memset(pInfo, 0, sizeof(Track_Cond_Info));

        CrossTrack_Cond_lock();
		list_for_each(plist, pHead){

			pCondition = list_entry(plist, CrossTrack_Condition, list);

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
        CrossTrack_Cond_unlock();

		if(plist!=pHead){	// we get the rule

			DEBUG("%s : > we get the rule...\n", __func__);
			pInfo->Time_active   = 1;
			pInfo->time_set_index = i;
			pInfo->pcond          = pCondition;
		}
	}

	// second, if we are in the active time proid, detetive if in the zone
	if(pInfo->Time_active == 1){

		track_err = pCondition->error;
        err = Get_CrossTrack_Error(pGPS, pCondition);

		DEBUG("%s : > CrossTrack Error = %d\n", __func__, err);

        if( err > track_err )
            pInfo->active = 1;
	}
}

static int Check_Current_Cond(Track_Cond_Info *pInfo)
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


static int Interval_Updata(Track_Cond_Info *pInfo)
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

    elememt.Update_callback   = Track_Packet_Over_CallBack;

    // fill with real gps value
    element_init_real(&elememt);

    Track_Packet_Over_Flag = 0;
	AsyncUpdate_signal(&elememt);
}


void Track_Packet_Over_CallBack(int success)
{
    current_cond.SendCount += success;
    Track_Packet_Over_Flag = 1;
}


static int Get_CrossTrack_Error(const GpsInfo *pGPS, CrossTrack_Condition *pCond)
{
    int    i;
    double err;
    double max_err;
    double curr_lat, curr_lon;
    double start_lat, start_lon;
    double end_lat, end_lon;

    curr_lat = nmea_ndeg2degree(pGPS->lat);
    curr_lon = nmea_ndeg2degree(pGPS->lon);

    start_lat = earth_msec2deg(pCond->path.point[0].Latitude);
    start_lon = earth_msec2deg(pCond->path.point[0].Longitude);

    max_err = 0.0;
    for(i=0; i<(pCond->path.sum-1); i++){

        end_lat = earth_msec2deg(pCond->path.point[i+1].Latitude);
        end_lon = earth_msec2deg(pCond->path.point[i+1].Longitude);
        
        err = get_CrossTrack(start_lat, start_lon, end_lat, end_lon, curr_lat, curr_lon);

        if( err>max_err ) max_err = err;

        start_lat = end_lat;
        start_lon = end_lon;
		
        DEBUG("%s : > max_err = %f\n", __func__, max_err);
    }

    return (int)(max_err*1000);
}
