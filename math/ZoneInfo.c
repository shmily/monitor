/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:41
#      Filename : ZoneInfo.c
#   Description : not thing...
#
=============================================================================*/

#include "ZoneInfo.h"


int Is_InZone(char shape, void *pShape_Value, GpsInfo *pGps)
{
	int res;

	switch (shape){

		case 0x01 :
			res = IsPosIn_Circle((Circle_Struct *)pShape_Value, pGps);
			break;

		case 0x02 :
			res = IsPosIn_Rect((Rectangle_Struct *)pShape_Value, pGps);
			break;

		case 0x03 :
			res = IsPosIn_Poly((Polygon_Struct *)pShape_Value, pGps);
			break;

		default :
			DEBUG("%s : > unknow shape ...\n", __func__);
			res = 0;
			break;
	}

	return res;
}
