/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-02-09 14:27
#      Filename : PositionMonitor.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "HeartBeat.h"
#include "PositionMonitor.h"
#include "TimeCompare.h"
#include "TLP.h"
#include "AsyncUpdate.h"
#include "GPS.h"
#include "misc.h"
#include "SequenceNumber.h"

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#ifdef _PM_LOWLEVLE__DEBUG__
	#define PM_Lowlevel_DEBUG(fmt,args...) fprintf(stderr,fmt, ## args)
#else
	#define PM_Lowlevel_DEBUG(fmt,args...)
#endif

extern struct list_head UpdateRuleList;
static Monitor_Info current_rule;

// if the last packet did not send over, Packet_Over_Flag is 0,
// we wait for untill it turn to be 1.
static int  Packet_Over_Flag = 1;

void Packet_Over_CallBack(int success);

static void Get_Active_Rule(struct list_head *pHead, Monitor_Info *pInfo);
static int  Check_Current_Rule(Monitor_Info *pInfo);
static int  Interval_Updata(Monitor_Info *pInfo);
static void Monitor_Packet_Update(void);

void PositionMonitor_init(void)
{
    Packet_Over_Flag = 1;
    memset(&current_rule, 0, sizeof(Monitor_Info));
}

void PositionMonitor( void )
{
	while(1){

        // wait for the last transmit finish.
        while(Packet_Over_Flag == 0){
            sleep(5);
        }

        // check the rule and update the position.
		if(current_rule.active == 0x00){		                    // no active rule, try to find one.
	
			Get_Active_Rule(&UpdateRuleList, &current_rule);

		}else{                                                      // if we have a active rule, update it.
			
			Check_Current_Rule(&current_rule);	
		}

		sleep(2);				                                    // every 2 sec
	}
}


// for position update rule, every rule is start by time, so we can
// test the begin time, if current time is over the begin time, we
// find the active rule.
static void Get_Active_Rule(struct list_head *pHead, Monitor_Info *pInfo)
{
	struct list_head 	*plist;
	Rule_Struct 		*pRule;
	
	memset(pInfo, 0, sizeof(Monitor_Info));

    Position_Rule_lock();
	list_for_each(plist, pHead){
		
		pRule = list_entry(plist, Rule_Struct, list);

		if( Is_BeyondTime( (Time_DT *)&(pRule->rulemap.btime) ) == 1){
			
			break;
		}
	}
    Position_Rule_unlock();

	if(plist!=pHead){	// we get the rule

		DEBUG("%s : > we get the rule...\n", __func__);
		pInfo->active = 1;
		pInfo->pRule = pRule;
	}
}

static int Check_Current_Rule(Monitor_Info *pInfo)
{
	int need_update;
    int rule_del = 0;

	// check the rule
    // if the rule is active or not.
	if( is_SendCount(pInfo->pRule->mask) ){
        PM_Lowlevel_DEBUG("%s : > this rule end with count.\n", __func__);
		if( pInfo->SendCount > pInfo->pRule->rulemap.emap.cnt ){
            
            PM_Lowlevel_DEBUG("%s : > send count : %d, emap : %d", __func__, pInfo->SendCount, pInfo->pRule->rulemap.emap.cnt);
            // we need to del the rule from the list
            rule_del = 1;    
		}
	} else {
        PM_Lowlevel_DEBUG("%s : > this rule end with time.\n", __func__);
		if( Is_BeyondTime( (Time_DT *)&(pInfo->pRule->rulemap.emap.etime) ) == 1 ){	
            // we need to del the rule from the list
            rule_del = 1;    
		}
	}

    if(rule_del == 1){      // del the rule
		
        DEBUG("%s : > Dell the rule.\n", __func__);
        Position_Rule_lock();
        PositionUpdate_Rule_Del(pInfo->pRule);
        Position_Rule_unlock();

	    memset(pInfo, 0, sizeof(Monitor_Info));

    } else {                // the rule is still active, update it.

		PM_Lowlevel_DEBUG("%s : > We need to update the position.\n", __func__);
	    need_update = Interval_Updata(pInfo); 
	    if(need_update == 1){
		    Monitor_Packet_Update();
	    }
    }

	return 0;
}


static int Interval_Updata(Monitor_Info *pInfo)
{
	int		need_send;
	time_t	current_time;
	int		mile;
	int 	interval;
	int		lastime;
	int		lastmile;

	need_send = 0;
	interval  = pInfo->pRule->rulemap.tmap;

	if( is_TimeChange(pInfo->pRule->mask) ){

		PM_Lowlevel_DEBUG("%s : > Time change update.\n", __func__);
		time(&current_time);
		lastime  = pInfo->lastTime; 

		if( (current_time - lastime) > interval ) {
			need_send = 1;
			pInfo->lastTime = current_time;
		}
	} else {
		
		PM_Lowlevel_DEBUG("%s : > Mileage change update.\n", __func__);
		mile     = Get_Current_mileage();
		lastmile = pInfo->last_mile;

		if( (mile - lastmile) > interval  ){
			need_send = 1;
			pInfo->last_mile = mile;
		} 
	}

	return need_send;
}

static void Monitor_Packet_Update(void)
{
	Update_elememt  elememt;
    
	memset(&elememt, 0, sizeof(Update_elememt));
	
	strcpy(elememt.Tid, "PRST");
	Position_Init(&elememt.location);
	elememt.action    = 0;
	elememt.speed     = 30;
	elememt.direction = 90;
	elememt.mileage   = 1000;
	elememt.info.TransmitType = _TransmitControl(TypeB, Packet_OUT);
	elememt.info.Option_data  = NULL;
	elememt.info.Option_len   = 0;
	//elememt.info.SEQ_num      = Get_Monitor_SEQ_num();
	elememt.info.SEQ_num      = Get_SEQ();

    elememt.Update_callback   = Packet_Over_CallBack;

    // fill with real gps value
    element_init_real(&elememt);

    Packet_Over_Flag = 0;
	AsyncUpdate_signal(&elememt);
}

void Packet_Over_CallBack(int success)
{
    current_rule.SendCount += success;
    Packet_Over_Flag = 1;
}
