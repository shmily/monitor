/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-16 01:16
#      Filename : HeartBeat.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __HEART_BEAT__
#define __HEART_BEAT__

#include "TLP.h"
#include "PositionUpdate.h"
#include "ReportCondition.h"

void HeartBeat( void );
TLP_Error HeartBeat_Handle( Packet_Struct *pPacket );
int is_lose_connection(void);

#endif
