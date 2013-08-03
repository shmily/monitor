/*
===============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-20 17:50
#      Filename : EmergencyMonitor.c
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
#include "EmergencyMonitor.h"
#include "TimeCompare.h"
#include "TLP.h"
#include "AsyncUpdate.h"
#include "GPS.h"
#include "misc.h"

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

static int              alarm_flag = 0;
static int              button_active = 0;
static fd_set           fdsr;
static struct timeval   tv;
static int              buttons_fd;


// if the last packet did not send over, Packet_Over_Flag is 0,
// we wait for untill it turn to be 1.
static int  Packet_Over_Flag = 1;
static Emergency_Cond_Info current_cond;

void        Emer_Packet_Over_CallBack(int success);
static void Get_Active_Cond(Emergency_Cond_Info *pInfo);
static int  Interval_Updata(Emergency_Cond_Info *pInfo);
static int  Check_Current_Cond(Emergency_Cond_Info *pInfo);
static void Monitor_Packet_Update(void);

static int CheckFor_Button(void);

void EmergencyMonitor_init(void)
{

    buttons_fd = open("/dev/buttons", 0);
	if (buttons_fd < 0) {
		perror("open device buttons");
	}

    memset(&current_cond, 0, sizeof(Emergency_Cond_Info));
    strcpy((char *)&current_cond.cond.RID, "r001");
    current_cond.cond.Interval = 30;
    current_cond.cond.Cnt      = 6;
    current_cond.cond.Action   = 0x0001;
}

void EmergencyMonitor(void)
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

static void Get_Active_Cond(Emergency_Cond_Info *pInfo)
{
    if(isNeed_Update_Emergency()==1){
        Copy_Emergency_Info(&pInfo->cond);
    }

    pInfo->SendCount     = 0;    
    pInfo->prv_send_time = 0;    

	if(CheckFor_Button()==1){	// we get the rule

		DEBUG("%s : > we need to update the Emetgency Report...\n", __func__);
		pInfo->active = 1;
	}
}

static int Check_Current_Cond(Emergency_Cond_Info *pInfo)
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


static int Interval_Updata(Emergency_Cond_Info *pInfo)
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
	
	strcpy(elememt.Tid, (const char *)current_cond.cond.RID);
	Position_Init(&elememt.location);
	elememt.action    = 0x0001;
	elememt.speed     = 30;
	elememt.direction = 90;
	elememt.mileage   = 1000;
	elememt.info.TransmitType = _TransmitControl(TypeB, Packet_OUT);
	elememt.info.Option_data  = NULL;
	elememt.info.Option_len   = 0;
	elememt.info.SEQ_num      = current_cond.cond.SEQ_num;

    elememt.Update_callback   = Emer_Packet_Over_CallBack;

    // fill with real gps value
    element_init_real(&elememt);

    Packet_Over_Flag = 0;
	AsyncUpdate_signal(&elememt);
}

void Emer_Packet_Over_CallBack(int success)
{
    current_cond.SendCount += success;
    Packet_Over_Flag = 1;
}


// wait for the button input
static int CheckFor_Button(void)
{
    int ret;
    char current_buttons[8];

    FD_ZERO(&fdsr);
    FD_SET(buttons_fd, &fdsr);

    // timeout setting
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    ret = select(buttons_fd+1, &fdsr, NULL, NULL, &tv);
    if( ret>0 ){
        
	    if (read(buttons_fd, current_buttons, sizeof current_buttons) != sizeof current_buttons) {
		    perror("read buttons:");
			exit(1);
		} 
            
        if(current_buttons[0]!='0'){
            button_active = 1;
        } else {
            button_active = 0;
            alarm_flag = 0;
        }
    } else if(ret == 0){
        if(button_active == 1){
            button_active = 0;
            alarm_flag = 1;
        }
    }

    return alarm_flag;
}

