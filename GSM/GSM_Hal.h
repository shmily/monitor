/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:36
#      Filename : GSM_Hal.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __GSM_HAL__
#define __GSM_HAL__

#include "GSM-error.h"

#ifdef __GSM_DEBUG__
	#define GSM_DEBUG(fmt,args...) fprintf(stderr,fmt, ## args)
#else
	#define GSM_DEBUG(fmt,args...)
#endif

GSM_Error GSM_Reset				(void);
GSM_Error GSM_Config			(void);
GSM_Error GSM_GetCSQ			(int *CSQ);
GSM_Error GSM_NetworkTest		(void);
GSM_Error GSM_GprsSetup			(void);
GSM_Error GSM_TcpipSetup		(int apn);
GSM_Error GSM_SetupUDPLink		(char *dest_ip, char *dest_port, char *local_port);
GSM_Error GSM_SendPacket		(char *pBuff);
GSM_Error GSM_GetPacketInfo		(char *pUnread_sum, char *pSum);
GSM_Error GSM_ReceivePacket		(int *pLink_num, int *pData_index, int *pData_len, char *pBuff);
GSM_Error GSM_CloseConnection	(void);
GSM_Error GSM_NetworkConfirm	(void);

GSM_Error NetWork_Connection_Config(char *p_IP, char *p_Port, int apn);

#endif
