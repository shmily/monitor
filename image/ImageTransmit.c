/*
===============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-02 02:52
#      Filename : ImageTransmit.c
#   Description : not thing...
#
===============================================================================
*/

#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include "UDP_Lib.h"
#include "GSM_Hal.h"
#include "GSM-error.h"
#include "TLP.h"
#include "DrowsyDetect.h"
#include "DtypeTransmit.h"
#include "Image_lib.h"

#include <memory.h>
#include <semaphore.h>
#include <assert.h>
#include <time.h>
#include <errno.h>

Image_Struct    Image_Info;
Dtype_Packet_t  OutPut_Packet;

static int      Image_Sending = 0;

extern enum DT_ErrorCode_t  ErrorCode;
extern enum DT_Status_t     CurrentStatus;

extern sem_t sem_update_ack;
extern sem_t sem_packet_ack;
extern sem_t sem_discon_ack;

static void ImageTransmit_loop(void);
static int send_image_packet(void);

void ImageTransmit_Init(void)
{
    memset(&Image_Info, 0, sizeof(Image_Struct));
    memset(&OutPut_Packet, 0, sizeof(Dtype_Packet_t));

    Image_Sending = 0;
    OutPut_Packet.pImage = &Image_Info;
}

// 
//
void ImageTransmit_Thread(void)
{
    DrowsyImage_Info *pInput;

    while(1){
        if( Image_Sending == 0 ){   // no running image, check every 5 sec.
   
            pInput = (DrowsyImage_Info *)DrowsyImage_Check();
            if( pInput != NULL ){
          //      if( GetImage_Info(pInput->image_path, pInput->image_protocol_name)==0 ){
          //          Image_Sending = 1;
          //          continue;
          //      }
            } else {
                sleep(5);
            }
        } else {                    // Image is be sending.

            ImageTransmit_loop();
        }
    }
}


static void ImageTransmit_loop(void)
{
    int ret;
    struct timespec timeout;

    switch(CurrentStatus){

        case S_INIT :
            ret = TypeD_Update_Request(&Image_Info);
            if(ret == ERROR_NONE){
                DEBUG("%s : Update request OK!\n", __func__);
                CurrentStatus = S_UPDATE_ACK;
            } else { 
                DEBUG("%s : *** Update request ERROR!.\n", __func__);
                sleep(5);
            }
            break;

        case S_UPDATE_ACK :
            timeout.tv_sec  = time(NULL) + 10;
            timeout.tv_nsec = 0;

            ret = sem_timedwait(&sem_update_ack, &timeout);
            if(ret == 0){
                if(ErrorCode == ERR_UPDATE_ACC){
                    DEBUG("%s : The server is ready for image input.\n", __func__);
                    CurrentStatus = S_PACKET_SEND;
                } else { 
                    DEBUG("%s : *** The server is not ready for image input.\n", __func__);
                    sleep(5);
                    CurrentStatus = S_INIT;
                }
            } else { 
                DEBUG("%s : *** Wait for update ack time out!\n", __func__);
                sleep(5);
                CurrentStatus = S_INIT;
            }
            break;

        case S_PACKET_SEND :
            ret = send_image_packet();
            if(ret == ERROR_NONE){
                DEBUG("%s : Send 16 packets success!\n", __func__);
                CurrentStatus = S_PACKET_ACK;
            } else {
                DEBUG("%s : *** Update packet ERROR!.\n", __func__);
                CurrentStatus = S_ERROR;
            }
            break;
        
        case S_PACKET_ACK :
            timeout.tv_sec  = time(NULL) + 10;
            timeout.tv_nsec = 0;

            ret = sem_timedwait(&sem_packet_ack, &timeout);
            if(ret == 0){
                if(ErrorCode == ERR_PACKET_ACK){
                    DEBUG("%s : The Server return the packet ack.\n", __func__);

                    if( is_ImageTransmit_over(&OutPut_Packet)==1 ){
                        CurrentStatus = S_DISCON_REQ;
                    } else {
                        CurrentStatus = S_PACKET_SEND;
                    }
                } else { 
                    DEBUG("%s : *** The server return error.\n", __func__);
                    sleep(5);
                    CurrentStatus = S_CHECK_PACKET;
                }
            } else { 
                DEBUG("%s : *** Wait for packet ack time out!\n", __func__);
                sleep(5);
                CurrentStatus = S_CHECK_PACKET;
            }
            break;

        case S_CHECK_PACKET :
            ret = TypeD_Check_Packet(&Image_Info, (OutPut_Packet.next_num-1));
            if(ret == ERROR_NONE){
                DEBUG("%s : Check packet request send!\n", __func__);
                CurrentStatus = S_PACKET_ACK;
            } else {
                DEBUG("%s : *** Check packet request ERROR!.\n", __func__);
                CurrentStatus = S_ERROR;
            }
            break;

        case S_DISCON_REQ :
            ret = TypeD_Disconnect_Request(0);
            if(ret == ERROR_NONE){
                DEBUG("%s : Disconnect packet request send!\n", __func__);
                CurrentStatus = S_DISCON_ACK;
            } else {
                DEBUG("%s : *** Check packet request ERROR!.\n", __func__);
                CurrentStatus = S_ERROR;
            }
            break;

        case S_DISCON_ACK :

            /*
            timeout.tv_sec  = time(NULL) + 10;
            timeout.tv_nsec = 0;

            ret = sem_timedwait(&sem_discon_ack, &timeout);
            if(ret == 0){
                if(ErrorCode == ERR_DISCON_OK){
                    DEBUG("%s : The data link disconnect.\n", __func__);
                    CurrentStatus = S_CLEAN;
                } else { 
                    DEBUG("%s : *** The disconnect request is error.\n", __func__);
                    sleep(5);
                    CurrentStatus = S_DISCON_REQ;
                }
            } else { 
                DEBUG("%s : *** Wait for disconnect ack time out!\n", __func__);
                sleep(5);
                CurrentStatus = S_DISCON_REQ;
            }*/

            CurrentStatus = S_CLEAN;
            break;

        case S_CLEAN :
            DEBUG("%s : Clean ... ready for next transmit.\n", __func__);
            ImageTransmit_Init();
            CurrentStatus = S_INIT;
            break;

        case S_ERROR :
            DEBUG("%s : *** Transmit ERROR!!!\n", __func__);
            _Set_Network_ERR();
            break;

        default :
            CurrentStatus = S_CLEAN;
            break;
    }
}

static int send_image_packet(void)
{
    int     ret;
    int     i;
    char    buff[_DT_SIZE];
    int     len;

    gengrate_PacketIndex(&OutPut_Packet);

    for(i=0; i<OutPut_Packet.Packet_sum; i++){
        memset(buff, 0, _DT_SIZE);
        len = GetImagePacket(&Image_Info, OutPut_Packet.PacketIndex[i], buff);
        OutPut_Packet.pdata      = buff;
        OutPut_Packet.actual_len = len;
        OutPut_Packet.num        = OutPut_Packet.PacketIndex[i];
        ret = TypeD_Send_Packet(&OutPut_Packet);

        if(ret != ERROR_NONE) break;
    }

    return ret;
}
