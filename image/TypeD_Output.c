/*
===============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-02 22:03
#      Filename : TypeD_Output.c
#   Description : not thing...
#
===============================================================================
*/

#include <stdint.h>
#include <math.h>
#include "GSM-error.h"
#include "TLP.h"
#include "DtypeTransmit.h"

#include "AsyncUpdate.h"
#include "SequenceNumber.h"

#include <semaphore.h>
#include <assert.h>
#include <time.h>
#include <errno.h>


// send update packet request fo server.
//
TLP_Error TypeD_Update_Request(Image_Struct *pImage)
{
    TLP_Error   res;
    PacketInfo  Info;
    char        opt_buff[64];
    char        *pOpt;

    int         size;

    pOpt = (char *)opt_buff;

    // copy the md5(16 byte)
    memcpy(pOpt, pImage->MD5, 16);
    pOpt += 16;

    // image size
    size = pImage->size;
    *(pOpt++) = (char)(size>>24);
    *(pOpt++) = (char)(size>>16);
    *(pOpt++) = (char)(size>>8);
    *(pOpt++) = (char)size;

    // packet size
    *(pOpt++) = (char)(_DT_SIZE >> 8);
    *(pOpt++) = (char)(_DT_SIZE);

    // file name
    memcpy(pOpt, pImage->name, 6);
    pOpt += 6;

    // pip name
    *(pOpt++) = 0x01;

    // image type
    *(pOpt++) = pImage->type;
    
    Info.TransmitType = _DT_UPDATE;
    Info.Option_data  = (uint8_t *)opt_buff;
    Info.SEQ_num      = HTONL(Get_SEQ());
    Info.Option_len   = (int)(pOpt - (char *)opt_buff);
    Info.App_data     = NULL;
    Info.App_len      = 0;

 
    GSM_mutex_lock();
    res = TLP_SendPacket(&Info);
    GSM_mutex_unlock();

    if(res!=ERROR_NONE){
        DEBUG("%s : Dtype update Packet request send ...Error!\n",__func__);
    } else {
        DEBUG("%s : Dtype update Packet request send ...OK\n",__func__);
    }

    return res;
}



// send packet fo server.
//
TLP_Error TypeD_Send_Packet(Dtype_Packet_t *pPacket)
{
    TLP_Error   res;
    PacketInfo  Info;
    char        app_buff[_DT_SIZE];
    char        opt_buff[64];
    char        *pOpt;
    char        *pApp;

    int         num;

    pOpt = (char *)opt_buff;
    pApp = (char *)app_buff;

    // ----- option date ----- //

    // copy the md5(16 byte)
    memcpy(pOpt, pPacket->pImage->MD5, 16);
    pOpt += 16;

    // need ack
    *(pOpt++) = pPacket->need_ack;

    // packet num.
    num = pPacket->num;
    *(pOpt++) = (char)(num >> 8);
    *(pOpt++) = (char)(num);

    // ----- app date ----- //
    memcpy(pApp, (char *)pPacket->pdata, pPacket->actual_len);
 
    Info.TransmitType = _DT_PACKET;
    Info.Option_data  = (uint8_t *)opt_buff;
    Info.SEQ_num      = HTONL(Get_SEQ());
    Info.Option_len   = (int)(pOpt - (char *)opt_buff);
    Info.App_data     = (uint8_t *)pApp;
    Info.App_len      = pPacket->actual_len;
 
    GSM_mutex_lock();
    res = TLP_SendPacket(&Info);
    GSM_mutex_unlock();

    if(res!=ERROR_NONE){
        DEBUG("%s : Image Packet send ...Error!\n",__func__);
    } else {
        DEBUG("%s : Image [Packet:%d] send ...OK\n",__func__, num);
    }

    return res;
}



// send check packet request to server.
//
TLP_Error TypeD_Check_Packet(Image_Struct *pImage, int current_num)
{
    TLP_Error   res;
    PacketInfo  Info;
    char        opt_buff[64];
    char        *pOpt;

    int         size;

    pOpt = (char *)opt_buff;

    // copy the md5(16 byte)
    memcpy(pOpt, pImage->MD5, 16);
    pOpt += 16;

    // image size
    size = pImage->size;
    *(pOpt++) = (char)(size>>24);
    *(pOpt++) = (char)(size>>16);
    *(pOpt++) = (char)(size>>8);
    *(pOpt++) = (char)size;

    // packet num
    *(pOpt++) = (char)(current_num >> 8);
    *(pOpt++) = (char)(current_num);

    
    Info.TransmitType = _DT_RSEND;
    Info.Option_data  = (uint8_t *)opt_buff;
    Info.SEQ_num      = HTONL(Get_SEQ());
    Info.Option_len   = (int)(pOpt - (char *)opt_buff);
    Info.App_data     = NULL;
    Info.App_len      = 0;
 
    GSM_mutex_lock();
    res = TLP_SendPacket(&Info);
    GSM_mutex_unlock();

    if(res!=ERROR_NONE){
        DEBUG("%s : Dtype check Packet request send ...Error!\n",__func__);
    } else {
        DEBUG("%s : Dtype check Packet request send ...OK\n",__func__);
    }

    return res;
}


// send disconnect request to server.
//
TLP_Error TypeD_Disconnect_Request(int error)
{
    TLP_Error   res;
    PacketInfo  Info;
    char        opt_buff[64];
    char        *pOpt;

    pOpt = (char *)opt_buff;

    // error code
    *(pOpt++) = (char)(error);
 
    Info.TransmitType = _DT_DISCONNECT;
    Info.Option_data  = (uint8_t *)opt_buff;
    Info.SEQ_num      = HTONL(Get_SEQ());
    Info.Option_len   = (int)(pOpt - (char *)opt_buff);
    Info.App_data     = NULL;
    Info.App_len      = 0;
 
    GSM_mutex_lock();
    res = TLP_SendPacket(&Info);
    GSM_mutex_unlock();

    if(res!=ERROR_NONE){
        DEBUG("%s : Dtype disconnect request send ...Error!\n",__func__);
    } else {
        DEBUG("%s : Dtype disconnect request send ...OK\n",__func__);
    }

    return res;
}

