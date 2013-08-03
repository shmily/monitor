/*
=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-04 19:23
#      Filename : main.c
#   Description : not thing...
#
=============================================================================
*/

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

#include "Task.h"
#include "param.h"
#include "SequenceNumber.h"
#include "uart.h"
#include "AtTransmit.h"
#include "GSM-error.h"
#include "GSM_Hal.h"
#include "UDP_Lib.h"

#include "Login.h"
#include "InputFilter.h"
#include "PositionUpdate.h"
#include "DeviceControl.h"

#include "HeartBeat.h"
#include "AsyncUpdate.h"

#include "PositionMonitor.h"
#include "SpeedMonitor.h"
#include "ZoneMonitor.h"
#include "CrossTrackMonitor.h"
#include "EmergencyMonitor.h"
#include "FatigueAlarm.h"

#include "ImageTransmit.h"

#include "GPS.h"

#include "buzzer.h"

#define	_EN_INITIAL_GSM_

// about GPRS
GSM_Device_Uart GsmDevice;
int				CSQ;
int				res;

// about GPS
pthread_t       GPS_Threadid;
GpsInfo         GPS_Msg;
GpsInfo         GPS_Debug;

// for test
pthread_t       Thread_ID[12];

int             PosUpdate_count = 0;
int             Emergency_count = 0;

extern Report_Info Emergency_Info;

int main(void)
{
    // initial image transmit
    Open_ImageFIFO(ImageFIFO);

    // uart initial
	fprintf(stderr, "This is a test for the Contex-A8...\n");
	serial_initial("/dev/ttySAC1", &GsmDevice, 9600);
	AtTransmitInit(&GsmDevice);
	usleep(10000);

    // buzzer init
//    buzzer_init();
//    buzzer_beep(100000);

	// GPS uart init
	GPS_Init(); 

	// load the connection param
	DevParam_Init();
	
	SEQ_Init();

#ifdef	_EN_INITIAL_GSM_
	// config the gprs network
	NetWork_Connection_Config(Param_IP(), Param_Port(), Param_APN());
#endif
	
	// get the CSQ	
	if(GSM_GetCSQ(&CSQ)!=ERR_NONE){
		fprintf(stderr, "Can't get the CSQ...\n");
	} else{
		fprintf(stderr, "CSQ = %d\n",CSQ);
	}

	sleep(5);

    // create the thread
    pthread_create(&Thread_ID[0], NULL, (void *)AsyncUpdate, NULL);
    pthread_create(&Thread_ID[1], NULL, (void *)HeartBeat,   NULL);

    PositionMonitor_init();
    pthread_create(&Thread_ID[2], NULL, (void *)PositionMonitor, NULL);

    SpeedMonitor_init();
    pthread_create(&Thread_ID[3], NULL, (void *)SpeedMonitor, NULL);

    InZoneMonitor_init();
    pthread_create(&Thread_ID[4], NULL, (void *)InZoneMonitor, NULL);
    
    OutZoneMonitor_init();
    pthread_create(&Thread_ID[5], NULL, (void *)OutZoneMonitor, NULL);

    CrossTrackMonitor_init();
    pthread_create(&Thread_ID[6], NULL, (void *)CrossTrackMonitor, NULL);

//    // initial image transmit
//    Open_ImageFIFO(ImageFIFO);
  
    TypeD_Transmit_Init();
    ImageTransmit_Init(); 
    pthread_create(&Thread_ID[7], NULL, (void *)ImageTransmit_Thread, NULL);

    // Emergency report
    EmergencyMonitor_init();
    pthread_create(&Thread_ID[8], NULL, (void *)EmergencyMonitor, NULL);

    // Fatigue report
    FatigueMonitor_init();
    pthread_create(&Thread_ID[9], NULL, (void *)FatigueMonitor, NULL);

    // GPS parse thread.
    pthread_create(&GPS_Threadid, NULL, (void *)GPS_Parse_thread, NULL);

    // try to login server
    while( Try_Login() != 0 );
//    buzzer_beep(100000);

	while(1){

        GSM_mutex_lock();
        InputPacket_Process();
        GPS_Infomation();
        GSM_mutex_unlock();

        if( isNeed_Updata_Position() ){
            Position_update_after_Call();
        }

        if(is_lose_connection() == 1){
            exit(-1);
        }

        sleep(1);
	}

	serial_close(&GsmDevice);
	
	return 0;
}
