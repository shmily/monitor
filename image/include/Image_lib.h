/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-03 03:51
#      Filename : Image_lib.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _Image_LIB_H_
#define _Image_LIB_H_

#include "DrowsyDetect.h"
#include "DtypeTransmit.h"

int GetImage_Info(const char *path, const char *name);
void gengrate_PacketIndex(Dtype_Packet_t *pPacket);
int GetImagePacket(Image_Struct *pImage, int index, char *pBuff);
int is_ImageTransmit_over(Dtype_Packet_t *pPacket);

#endif

