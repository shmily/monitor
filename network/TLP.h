/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:02
#      Filename : TLP.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __TLP_H__
#define __TLP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define 	__DEBUG__ 		1

#ifdef __DEBUG__
	#define DEBUG(fmt,args...) fprintf(stderr,fmt, ## args)
#else
	#define DEBUG(fmt,args...)
#endif

#define HTONS(n) ((((uint16_t)((n) & 0xff)) << 8) | (((n) & 0xff00) >> 8))

#define HTONL(n) ((((uint32_t)(n) & 0xff000000) >> 24) | \
                  (((uint32_t)(n) & 0x00ff0000) >> 8)  | \
                  (((uint32_t)(n) & 0x0000ff00) << 8)  | \
                  (((uint32_t)(n) & 0x000000ff) << 24))


#define _NONE_         "\033[m"
#define _RED_          "\033[0;32;31m"
#define _LIGHT_RED_    "\033[1;31m"
#define _GREEN_        "\033[0;32;32m"
#define _LIGHT_GREEN_  "\033[1;32m"
#define _BLUE_         "\033[0;32;34m"
#define _LIGHT_BLUE_   "\033[1;34m"
#define _DARY_GRAY_    "\033[1;30m"
#define _CYAN_         "\033[0;36m"
#define _LIGHT_CYAN_   "\033[1;36m"
#define _PURPLE_       "\033[0;35m"
#define _LIGHT_PURPLE_ "\033[1;35m"
#define _BROWN_        "\033[0;33m"
#define _YELLOW_       "\033[1;33m"
#define _LIGHT_GRAY_   "\033[0;37m"
#define _WHITE_        "\033[1;37m"

#define __REG       "\033[31m"
#define __GREEN     "\033[32m"
#define __YELLOW    "\033[33m"
#define __END       "\033[0m"


// Transmit Error defind
typedef enum{
	
	ERROR_NONE = 0,		// transmit ok
	ERROR_CRC,			// CRC error
	ERROR_SEND,
	ERROR_UNKNOW		// unknow error

}TLP_Error;

// packet head info
#define     _VERSION        0x00
#define     _TransmitCtrl   0x80
#define     _TransmitType   0x01
#define     _Encrypt        0x00
#define     _Reserve        0x00

// Transmit type def

#define		TypeA			0x00
#define		TypeB			0x10
#define		TypeC			0x20
#define		TypeD			0x30
#define		TypeE			0x40
#define		TypeX			0xFF

// TypeA command
#define		_TypeA_ReportCondition		0x11
#define 	_TypeA_PositionRule_Set		0x0D
#define 	_TypeA_PositionRule_Del		0x0E
#define 	_TypeA_DeviceControl		0x01
#define     _TypeA_HeartBeat            0x41

// transmit control 
#define 	Packet_IN		0x00
#define 	Packet_OUT		0x80

#define		_TransmitControl(type, dir)	\
			(type | dir)

#define		_MAX_PACKET_LEN		1024

// input packet
typedef struct _InputPacket_
{
	uint32_t length;
	uint8_t  Data[_MAX_PACKET_LEN];
}Packet_Struct;


// transport layer head
typedef struct _TLP_Head_
{
	uint16_t	crc16;
	uint8_t		version;		// 0x00
	uint8_t		length;
	
	uint8_t		type;			
	uint8_t		medium;			// UDP : 0x03
	uint8_t		encrypt;		// none : 0x00
	uint8_t		reserve;		// 0x00
	
	uint32_t	SEQ_num;
	uint32_t	DEV_IDH;
	uint32_t	DEV_IDL;
	uint32_t	timestamp;
}TLP_Head_Struct;

// Input Pack struct
typedef struct _InputPack_
{
	uint8_t  	TransmitType;
	uint32_t 	SEQ_num;
	uint8_t  	*Option_data;
	uint16_t 	Option_len;
	uint8_t  	*App_data;
	uint16_t 	App_len;
}PacketInfo;

TLP_Error 	TLP_SendPacket		(PacketInfo *pInfo);
int 		TLP_ReceivePacket	(char *pbuff);
int         is_Link_Active      (void);

#define _Set_Network_ERR()  

#endif
