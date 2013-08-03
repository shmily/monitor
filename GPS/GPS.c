#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

#include "nmea/GPS.h"
#include "uart.h"
#include "nmea/nmea.h"

#define _MULTI_THREADED

static nmeaGPRMC		vGPRMC;
static GSM_Device_Uart  GpsDevice;
static char			    Gps_str[512];
static int				len;
static GpsInfo          gps_tmp;
static int              Mileage;

static pthread_rwlock_t GPS_rwlock;

void Mileage_Integral(union sigval v);

void GPS_Init(void)
{
    struct sigevent     evp;
    struct itimerspec   ts;
    timer_t             timer;
    int                 ret;

	fprintf(stderr, "Initial GPS...\n");
	serial_initial("/dev/ttySAC2", &GpsDevice, 9600);

    Mileage = 0;

    // initial the rwlock.
    pthread_rwlock_init(&GPS_rwlock, NULL);

    // set a timer for mileage integral.
    memset(&evp, 0, sizeof(evp));
    evp.sigev_value.sival_ptr = &timer;
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = Mileage_Integral;

    ret = timer_create(CLOCK_REALTIME, &evp, &timer);
    if(ret){
        perror("timer_Create");
    }

    ts.it_interval.tv_sec  = 10;  
    ts.it_interval.tv_nsec = 0;  
    ts.it_value.tv_sec     = 10;  
    ts.it_value.tv_nsec    = 0;  
    
    ret = timer_settime(timer, TIMER_ABSTIME, &ts, NULL);  
    if( ret ){  
        perror("timer_settime");
    }
	
	gps_tmp.lat  	= 3853.1876;
	gps_tmp.lon   	= 12131.8528;
	gps_tmp.speed	= 32;
}


void GPS_Parse_thread(void)
{
    while(1){

		memset((char *)Gps_str, 0, 512);
		len = serial_read(&GpsDevice, Gps_str, 512);
		
		if(len>0){

			if(strstr(Gps_str,"$GPRMC")!=NULL){
				nmea_parse_GPRMC(Gps_str, len, &vGPRMC);

                pthread_rwlock_wrlock(&GPS_rwlock);
				gps_tmp.status    = vGPRMC.status;
				gps_tmp.lat       = (vGPRMC.status == 'A') ? vGPRMC.lat : gps_tmp.lat;
				gps_tmp.ns        = vGPRMC.ns;
				gps_tmp.lon       = (vGPRMC.status == 'A') ? vGPRMC.lon : gps_tmp.lon;
				gps_tmp.ew        = vGPRMC.ew;
				gps_tmp.speed     = (vGPRMC.status == 'A') ? vGPRMC.speed : gps_tmp.speed;
				gps_tmp.direction = vGPRMC.direction;
			    pthread_rwlock_unlock(&GPS_rwlock);
            }
		}

        sleep(2);
	}
}

// every 10 second.
void Mileage_Integral(union sigval v)
{
    int         speed;              // km/h
    int         tmp       = 0;      // units:m

	speed = Get_Current_Speed();   
    tmp = ((speed*100)/36);

    Mileage += tmp;

	//fprintf(stderr, "%s >> calculate the mileage : %d m\n", __func__, Mileage);
}

void GPS_InfoPrintf(GpsInfo *pGPS)
{
	fprintf(stderr, "\n\n========================\n");
	fprintf(stderr, "Status    : %s\n",     (pGPS->status=='A')?("Active"):("Inactive"));
	fprintf(stderr, "Latitude  : %5.4f\n",   pGPS->lat);
	fprintf(stderr, "Longitude : %5.4f\n",   pGPS->lon);
	fprintf(stderr, "Speed     : %f Km/h\n", pGPS->speed);
	fprintf(stderr, "direction : %f\n",      pGPS->direction);
	fprintf(stderr, "Mileage   : %d m\n",    Get_Current_mileage());
	fprintf(stderr, "========================\n\n");
}

void GPS_Infomation(void)
{
    pthread_rwlock_rdlock(&GPS_rwlock);

	fprintf(stderr, "{[%s]:",   (gps_tmp.status=='A')?("Active"):("Inactive"));
	fprintf(stderr, "N%5.4f, ", gps_tmp.lat);
	fprintf(stderr, "E%5.4f}",  gps_tmp.lon);
    
    pthread_rwlock_unlock(&GPS_rwlock);
}

void GPS_Close(void)
{
	serial_close(&GpsDevice);
}


// km/h
int Get_Current_Speed(void)
{
    int speed;

    pthread_rwlock_rdlock(&GPS_rwlock);
    speed = gps_tmp.speed * NMEA_TUD_KNOTS;
    pthread_rwlock_unlock(&GPS_rwlock);
    
    return speed; 
}

// return current GPS information
void Get_Current_GPS(void *p)
{
    pthread_rwlock_rdlock(&GPS_rwlock);
    memcpy(p, &gps_tmp, sizeof(GpsInfo));
    pthread_rwlock_unlock(&GPS_rwlock);
}

int  Get_Current_mileage(void)
{
    int tmp;

    pthread_rwlock_rdlock(&GPS_rwlock);
    tmp = Mileage;
    pthread_rwlock_unlock(&GPS_rwlock);

    return tmp;
}

