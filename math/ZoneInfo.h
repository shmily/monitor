/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:41
#      Filename : ZoneInfo.h
#   Description : not thing...
#
=============================================================================*/

#ifndef		_ZONE_INFO_H_
#define		_ZONE_INFO_H_

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "earth_math.h"
#include "CircleJudge.h"
#include "RectJudge.h"
#include "PolyJudge.h"
#include "nmea.h"
#include "GPS.h"


int Is_InZone(char shape, void *pShape_Value, GpsInfo *pGps);

#endif
