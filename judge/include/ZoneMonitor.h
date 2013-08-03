/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-02-22 19:18
#      Filename : ZoneMonitor.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __ZONE_MONITOR_H__
#define __ZONE_MONITOR_H__

#include <time.h>

// Zone monitor info
typedef struct _Zone_Cond_info__
{
	int 			active;
	int     		SendCount;
	time_t			last_send_time;

	int 			Time_active;
	int 			time_set_index;

	Zone_Condition 	*pcond;
}Zone_Cond_Info;


void InZoneMonitor_init(void);
void InZoneMonitor(void);

void OutZoneMonitor_init(void);
void OutZoneMonitor(void);

#endif
