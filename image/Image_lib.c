/*
===============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-02 23:42
#      Filename : Image_lib.c
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
#include "md5.h"

#include <semaphore.h>
#include <assert.h>
#include <time.h>
#include <errno.h>

extern Image_Struct     Image_Info;
extern Retry_Info       RetryPacket;

int GetImage_Info(const char *path, const char *name)
{	
	FILE *pFile;
	
	// for md5
	md5_state_t state;
	int			i;
	int			len;
	char	    buff[1024];
	
	pFile = fopen(path, "r");
	Image_Info.pFile = pFile;
	
	if(pFile == NULL){
		fprintf(stderr, "%s : Opend file \"%s\" faild...\n", __func__, path);
		return -1;
	}
	
	// get file szie
	fseek(pFile, SEEK_SET, SEEK_END);
	Image_Info.size = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

    // get total packet 
	Image_Info.Total_Packet = (Image_Info.size + (_DT_SIZE-1))/_DT_SIZE;
	
	// get file md5
	md5_init(&state);
	
	do{
		len = fread(buff, 1, 1024, pFile);
		md5_append(&state, (const md5_byte_t *)buff, len);
	}while(len==1024);
	
	md5_finish(&state, (md5_byte_t *)Image_Info.MD5);
	
	fprintf(stderr, "MD5 of \"%s\" : ", path);
	for(i=0;i<16;i++){
		fprintf(stderr, "%02X", Image_Info.MD5[i]);
	}	
	fprintf(stderr, "\n");
	
	fseek(pFile, 0, SEEK_SET);

	// type
	Image_Info.type = 0x07;
	
	// file name need to be add **
	Image_Info.name[0] = *(name++);
	Image_Info.name[1] = *(name++);;
	Image_Info.name[2] = *(name++);;
	Image_Info.name[3] = *(name++);;
	Image_Info.name[4] = *(name++);;
	Image_Info.name[5] = *(name++);;


	fprintf(stderr, "%s : image size   : %d Byte\n", __func__, Image_Info.size);
	fprintf(stderr, "%s : total Packet : %d\n", __func__, Image_Info.Total_Packet);
	
	return 0;
}


void gengrate_PacketIndex(Dtype_Packet_t *pPacket)
{
    int i;
    int next_PacketIndex;
    int last_packet_num;
    int packet_sum;

    next_PacketIndex = pPacket->next_num;
    if( (RetryPacket.error_code == 0x00) || (RetryPacket.error_code == 0x01) ){     // no resend packet.
        for(i=0; i<16; i++){
            pPacket->PacketIndex[i] = next_PacketIndex;
            next_PacketIndex++;
        }
    } else if(RetryPacket.error_code == 0x02) {
    
        // copy the resend packet index
        for(i=0; i<RetryPacket.packet_num; i++){
            pPacket->PacketIndex[i] = RetryPacket.packetIndex[i];
        }
        
        // get new packet index
        for(; i<16; i++){ 
            pPacket->PacketIndex[i] = next_PacketIndex;
            next_PacketIndex++;
        }
    }

    // if we reach the end of the file
    packet_sum = 16;
    last_packet_num = pPacket->pImage->Total_Packet;
    if(next_PacketIndex >= last_packet_num){
	    fprintf(stderr, "%s : ......... reach the end of the file.........\n", __func__);
        pPacket->lastPacket = 1;
        packet_sum = (16 - (next_PacketIndex - last_packet_num));
        next_PacketIndex = last_packet_num;
    }

    pPacket->Packet_sum = packet_sum;
    pPacket->next_num   = next_PacketIndex;
 
	fprintf(stderr, "%s : pPacket->Packet_sum : %d\n", __func__, pPacket->Packet_sum);
	fprintf(stderr, "%s : pPacket->next_num   : %d\n", __func__, pPacket->next_num);
}

int GetImagePacket(Image_Struct *pImage, int index, char *pBuff)
{
	FILE *pFile;
	int	 len;
				
	pFile = pImage->pFile;
	fseek(pFile, (index*_DT_SIZE), SEEK_SET);
	len = fread(pBuff, 1, _DT_SIZE, pFile);
				
	return len;
}

int is_ImageTransmit_over(Dtype_Packet_t *pPacket)
{
    if(pPacket->lastPacket == 1){
        if(RetryPacket.error_code == 0x01) return 1;
    }

    return 0;
}
