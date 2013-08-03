/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-13 18:03
#      Filename : CrossTrackMonitor.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __CROSSTRACK_MONITOR_H__
#define __CROSSTRACK_MONITOR_H__

#include <time.h>

// track monitor info
typedef struct _Track_Cond_info__
{
	int 			active;
	int     		SendCount;
	time_t			last_send_time;

	int 			Time_active;
	int 			time_set_index;

	CrossTrack_Condition *pcond;
}Track_Cond_Info;


void CrossTrackMonitor_init(void);
void CrossTrackMonitor(void);

#endif
