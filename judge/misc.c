/*
===============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-22 21:15
#      Filename : misc.c
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
#include "TimeCompare.h"
#include "TLP.h"
#include "AsyncUpdate.h"
#include "GPS.h"
#include "earth_math.h"

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>


void element_init_real(Update_elememt  *pelememt)
{
    GpsInfo     gps;

    memset(&gps, 0, sizeof(GpsInfo));
    Get_Current_GPS(&gps);

    pelememt->speed     = Get_Current_Speed(); 
    pelememt->direction = gps.direction;
    pelememt->mileage   = Get_Current_mileage() / 1000;

    pelememt->location.location_Status = (gps.status=='A') ? 0x20:0x00;
    pelememt->location.Latitude        = NDEG2MSEC(gps.lat);  
    pelememt->location.Longitude       = NDEG2MSEC(gps.lon);
    pelememt->location.Alitude         = 20; 
}
