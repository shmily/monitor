/*
=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-20 18:26
#      Filename : EmergencyMonitor.h
#   Description : not thing...
#
=============================================================================
*/

#ifndef __EMERGENCY_MONITOR_H__
#define __EMERGENCY_MONITOR_H__

#include <time.h>

#include "ReportCondition.h"

// Emergency monitor info
typedef struct _Emergency_Cond_Info_
{
	int 	    active;
	int         SendCount;
    time_t      prv_send_time;
    Report_Info cond;
}Emergency_Cond_Info;


void EmergencyMonitor_init(void);
void EmergencyMonitor(void);

#endif
