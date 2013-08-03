/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-14 03:20
#      Filename : AsyncUpdate.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __ASYNC_UPDATE__
#define __ASYNC_UPDATE__

#include "TLP.h"
#include "PositionUpdate.h"
#include "ReportCondition.h"

typedef struct _Update_elememt
{
    int             action;         // rname, for report update
    char            Tid[32];        // CALL / PRST
    Point_Struct    location;
    int             speed;          // km/h
    int             direction;      // 0~359
    int             mileage;        // unit : km

    PacketInfo      info;

    int             ack_timeout;
    int             ack_tries;
    void (*Update_callback)(int);

}Update_elememt;

// comm
char *Async_Enpacket_DefaultAPP (char *pbuff, int action, char need_return);
char *Enpacket_PI_AKV           (char *pbuff, Point_Struct *point);
char *Enpacket_TID_AKV          (char *pbuff, const char *tid);
char *Enpacket_S_AKV            (char *pbuff, int speed);
char *Enpacket_D_AKV            (char *pbuff, int direction);
char *Enpacket_ME_AKV           (char *pbuff, int mileage);
char *Enpacket_RID_AKV          (char *pbuff, const char *tid);
char *Enpacket_Rname_AKV        (char *pbuff, int rname);

void Update_elememt_copy(Update_elememt *src);

void Position_Init(Point_Struct *point);

// about thread
void GSM_mutex_lock(void);
void GSM_mutex_unlock(void);
void AsyncUpdate( void );
void AsyncUpdate_signal( Update_elememt *elememt );

#endif
