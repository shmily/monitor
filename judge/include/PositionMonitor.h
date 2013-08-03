/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-02-09 14:27
#      Filename : PositionMonitor.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __POSITION_MONITOR_H__
#define __POSITION_MONITOR_H__

#include <time.h>

// position monitor info
typedef struct _Monitor_info__
{
	int 			active;
	int     		SendCount;
	time_t			lastTime;
	int				last_mile;
	Rule_Struct 	*pRule;
}Monitor_Info;


void PositionMonitor_init(void);
void PositionMonitor(void);

#endif
