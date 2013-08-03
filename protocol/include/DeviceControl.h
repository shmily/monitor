/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-14 22:01
#      Filename : DeviceControl.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __DEVICE_CONTROL_H__
#define	__DEVICE_CONTROL_H__

#include "TLP.h"
#include "AsyncUpdate.h"


TLP_Error DeviceControl_Handle( Packet_Struct *pPacket );
int isNeed_Updata_Position(void);
int Get_RollCall_SEQ_num(void);

int Position_update_after_Call(void);



#endif
