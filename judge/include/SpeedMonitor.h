/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-02-21 14:00
#      Filename : SpeedMonitor.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __SPEED_MONITOR_H__
#define __SPEED_MONITOR_H__

#include <time.h>

#include "ReportCondition.h"

// speed monitor info
typedef struct _Speed_Cond_Info_
{
	int 			        active;
	int     		        SendCount;
	int 			        last_time;
	int				        last_speed;
    time_t                  prv_send_time;
    SpeedAnomaly_Condition  cond;
}Speed_Cond_Info;


void SpeedMonitor_init(void);
void SpeedMonitor(void);

#endif
