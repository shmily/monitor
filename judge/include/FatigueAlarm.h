/*
=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-05-19 22:17
#      Filename : FatigueAlarm.h
#   Description : not thing...
#
=============================================================================
*/

#ifndef __FATIGUE_MONITOR_H__
#define __FATIGUE_MONITOR_H__

#include <time.h>

#include "ReportCondition.h"

// monitor info
typedef struct _Fatigue_Cond_Info_
{
	int 	    active;
	int         SendCount;
    time_t      prv_send_time;
    Report_Info cond;
}Fatigue_Cond_Info;


void FatigueMonitor_init(void);
void FatigueMonitor(void);

#endif
