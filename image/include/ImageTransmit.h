/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-09 18:00
#      Filename : ImageTransmit.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _Image_TRANSMIT_H_
#define _Image_TRANSMIT_H_

#include "DrowsyDetect.h"
#include "DtypeTransmit.h"
#include "FIFO.h"
#include "Image_lib.h"

void ImageTransmit_Init(void);
void ImageTransmit_Thread(void);

#endif

