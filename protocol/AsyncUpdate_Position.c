/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-14 03:21
#      Filename : AsyncUpdate_Position.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "AsyncUpdate.h"

#include "earth_math.h"

char *Async_Enpacket_DefaultAPP(char *pbuff, int action, char need_return)
{
	*(pbuff++) = 0x40;		            // 0x40

	*(pbuff++) = (char)(action>>8);	    // ACTION
	*(pbuff++) = (char)(action);

	*(pbuff++) = need_return;		    // need_return

	return pbuff;
}

/*
char *Enpacket_PositionUpdate(char *pbuff, Update_elememt *pelememt)
{
    char *p;
    int  len;

    const char debug[] = {
        0x40, 0x00, 0x01, 0x01, 0x00, 0x03, 0x54, 0x69, 0x64, 0x00, 0x04, 0x43, 0x41, 0x4C, 0x4C, 0x10,
        0x02, 0x70, 0x49, 0x00, 0x25, 0x00, 0x01, 0x74, 0x00, 0x01, 0x20, 0x00, 0x03, 0x6C, 0x61, 0x74,
        0x00, 0x04, 0x05, 0x42, 0x10, 0x84, 0x00, 0x03, 0x6C, 0x6F, 0x6E, 0x00, 0x04, 0x19, 0x57, 0xC6,
        0x98, 0x00, 0x03, 0x61, 0x6C, 0x74, 0x00, 0x02, 0x01, 0xF3, 0x00, 0x01, 0x73, 0x00, 0x02, 0x00,
        0x23, 0x00, 0x01, 0x64, 0x00, 0x02, 0x00, 0x03, 0x00, 0x02, 0x6D, 0x65, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x0C, 0x10, 0x05, 0x54, 0x53, 0x4D, 0x41, 0x50, 0x00, 0x12, 0x00, 0x01, 0x74, 0x00, 0x01,
        0x00, 0x00, 0x01, 0x61, 0x00, 0x01, 0x01, 0x00, 0x01, 0x67, 0x00, 0x01, 0x02};

    p = pbuff;
    len = sizeof(debug);
    fprintf(stderr, "%s > App date len : %d\n", __func__, len);

    memcpy(pbuff, debug, len);
    pbuff += len;

    pbuff = Async_Enpacket_DefaultAPP(pbuff);
    pbuff = Enpacket_TID_AKV(pbuff, pelememt->Tid);
    pbuff = Enpacket_PI_AKV(pbuff, &pelememt->location);
    pbuff = Enpacket_S_AKV(pbuff, pelememt->speed);
    pbuff = Enpacket_D_AKV(pbuff, pelememt->direction);
    pbuff = Enpacket_ME_AKV(pbuff, pelememt->mileage);

    return pbuff;
}*/

char *Enpacket_TID_AKV(char *pbuff, const char *tid)
{
    int tid_len;

    tid_len = strlen(tid);

    *(pbuff++) = 0x00;      // attr
    *(pbuff++) = 0x03;      // key length
    *(pbuff++) = 'T';       // key
    *(pbuff++) = 'i';
    *(pbuff++) = 'd';
    *(pbuff++) = (char)(tid_len >> 8);
    *(pbuff++) = (char)(tid_len);
    
    memcpy(pbuff, tid, tid_len);

    return (pbuff + tid_len);
}


char *Enpacket_S_AKV(char *pbuff, int speed)
{
    *(pbuff++) = 0x00;      // attr
    *(pbuff++) = 0x01;      // key length
    *(pbuff++) = 's';       // key
    *(pbuff++) = 0x00;
    *(pbuff++) = 0x02;
    *(pbuff++) = (char)(speed >> 8);
    *(pbuff++) = (char)(speed);

    return pbuff;
}

char *Enpacket_D_AKV(char *pbuff, int direction)
{
    *(pbuff++) = 0x00;      // attr
    *(pbuff++) = 0x01;      // key length
    *(pbuff++) = 'd';       // key
    *(pbuff++) = 0x00;
    *(pbuff++) = 0x02;
    *(pbuff++) = (char)(direction >> 8);
    *(pbuff++) = (char)(direction);

    return pbuff;
}

char *Enpacket_ME_AKV(char *pbuff, int mileage)
{
    *(pbuff++) = 0x00;      // attr
    *(pbuff++) = 0x02;      // key length
    *(pbuff++) = 'm';       // key
    *(pbuff++) = 'e';
    *(pbuff++) = 0x00;
    *(pbuff++) = 0x04;
    *(pbuff++) = (char)(mileage >> 24);
    *(pbuff++) = (char)(mileage >> 16);
    *(pbuff++) = (char)(mileage >> 8);
    *(pbuff++) = (char)(mileage);

    return pbuff;
}

char *Enpacket_PI_AKV(char *pbuff, Point_Struct *point)
{
    char t;
    int  lat;
    int  lon;
    int  alt;
    int  value_len;
    char *pvalue_len;

    t = point->location_Status;
    lat = point->Latitude;
    lon = point->Longitude;
    alt = point->Alitude;
    
    *(pbuff++) = 0x10;
    *(pbuff++) = 0x02;
    *(pbuff++) = 'p';
    *(pbuff++) = 'I';

    pvalue_len = pbuff;
    
    pbuff = pbuff + 2;

    // t AKV
    *(pbuff++) = 0x00;
    *(pbuff++) = 0x01;
    *(pbuff++) = 't';
    *(pbuff++) = 0x00;
    *(pbuff++) = 0x01;
    *(pbuff++) = t;

    // lat AKV
    *(pbuff++) = 0x00;      // attr
    *(pbuff++) = 0x03;      // key length
    *(pbuff++) = 'l';       // key
    *(pbuff++) = 'a';
    *(pbuff++) = 't';
    *(pbuff++) = 0x00;
    *(pbuff++) = 0x04;
    *(pbuff++) = (char)(lat >> 24);
    *(pbuff++) = (char)(lat >> 16);
    *(pbuff++) = (char)(lat >> 8);
    *(pbuff++) = (char)(lat);

    // lon AKV
    *(pbuff++) = 0x00;      // attr
    *(pbuff++) = 0x03;      // key length
    *(pbuff++) = 'l';       // key
    *(pbuff++) = 'o';
    *(pbuff++) = 'n';
    *(pbuff++) = 0x00;
    *(pbuff++) = 0x04;
    *(pbuff++) = (char)(lon >> 24);
    *(pbuff++) = (char)(lon >> 16);
    *(pbuff++) = (char)(lon >> 8);
    *(pbuff++) = (char)(lon);

   // alt AKV
    *(pbuff++) = 0x00;      // attr
    *(pbuff++) = 0x03;      // key length
    *(pbuff++) = 'a';       // key
    *(pbuff++) = 'l';
    *(pbuff++) = 't';
    *(pbuff++) = 0x00;
    *(pbuff++) = 0x02;
    *(pbuff++) = (char)(alt >> 8);
    *(pbuff++) = (char)(alt);

    value_len = pbuff - pvalue_len - 2;
    *(pvalue_len++) = (char)(value_len >> 8);
    *(pvalue_len++) = (char)(value_len);

    return pbuff;
}

void Position_Init(Point_Struct *point)
{
    static int i = 0;

    point->location_Status = 0x20;
    //point->Latitude        = 0x04DE650F;
    //point->Longitude       = 0x186A4853;
    point->Latitude        = Deg2MSEC( 38, 53,  8.28);
    point->Longitude       = Deg2MSEC(121, 31, 36.77);
    point->Alitude         = 20;

    i++;
}
