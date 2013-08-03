
#ifndef		_GPS_H_
#define		_GPS_H_

#include "nmea.h"

typedef	struct _GpsInfo
{
	char	status;		// A = active; V = void
	double	lat;
	char	ns;			// N, S
	double	lon;		
	char	ew;			// E, W
	double	speed;		// km/h
	double	direction;
}GpsInfo;


void GPS_Init(void);
void GPS_Parse_thread(void);

void GPS_Read(GpsInfo *pGPS);
void GPS_InfoPrintf(GpsInfo *pGPS);
void GPS_Close(void);


int  Get_Current_Speed(void);
void Get_Current_GPS(void *p);
int  Get_Current_mileage(void);

void GPS_Infomation(void);

#endif
