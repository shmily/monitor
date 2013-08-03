/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:49
#      Filename : Login.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _LOGIN_H_
#define	_LOGIN_H_

#include <stdio.h>
#include "TLP.h"

TLP_Error 	Send_LoginPacket		(void);
int 		Try_Login				(void);		// try to login server, 0 --> success; -1 --> faild
TLP_Error 	Send_RegistPacket		(void);
int 		Try_Regist				(void);
//APP_Error Wait_LoginACK			(void *pdata);
//GSM_Error Send_RegisterPacket	(void);
//APP_Error Wait_RegisterACK		(void *pdata);
#endif
