/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:06
#      Filename : DtypeTransmit.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _BTYPE_TRANSMIT_H_
#define _BTYPE_TRANSMIT_H_

#include <stdio.h>
#include "TLP.h"

#define		_DT_SIZE			512

enum DT_ErrorCode_t
{
    ERR_UPDATE_ACC,
    ERR_UPDATE_DENY,
    ERR_PACKET_ACK,
    ERR_DISCON_ERROR,
    ERR_DISCON_OK,

    ERR_INIT
};

enum DT_Status_t
{
    S_UPDATE_REQ,
    S_UPDATE_ACK,
    S_PACKET_SEND,
    S_PACKET_ACK,
    S_CHECK_PACKET,
    S_DISCON_REQ,
    S_DISCON_ACK,
    S_CLEAN,

    S_ERROR,
    S_INIT
};

// Control typde define
#define		_DT_UPDATE			0xB0
#define		_DT_UPDATE_ACK		0x30
#define		_DT_PACKET			0xB1
#define		_DT_PACKET_ACK		0x31
#define		_DT_RSEND			0xB2
#define		_DT_RSEND_ACK		0x32
#define		_DT_RCONNECT		0xB3
#define		_DT_RCONNECT_ACK	0x33
#define		_DT_DISCONNECT		0xB4
#define		_DT_DISCONNECT_ACK	0x34

typedef struct _Retry_Info
{
	char 	error_code;
	char	packet_num;
	int		packetIndex[16];
}Retry_Info;

typedef struct _Image
{
	FILE 	*pFile;
	int		size;
	int		Total_Packet;
	char	MD5[16];
	char	name[8];
	char	type;
}Image_Struct;

typedef struct _DT_Packet
{
    Image_Struct *pImage;
	char	need_ack;
	int		num;

    int		next_num;
    int     Packet_sum;
    int     PacketIndex[16];
	int		lastPacket;
	int     actual_len;
	void	*pdata;
}Dtype_Packet_t;


// output founctions
TLP_Error TypeD_Update_Request     (Image_Struct *pImage);
TLP_Error TypeD_Send_Packet        (Dtype_Packet_t *pPacket);
TLP_Error TypeD_Check_Packet       (Image_Struct *pImage, int current_num);
TLP_Error TypeD_Disconnect_Request (int error);

// input founctions
void TypeD_Transmit_Init(void);
void TypeD_InputPacketHandle(Packet_Struct *pInput);

#endif



