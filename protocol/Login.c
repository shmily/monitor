/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-12-20 15:54
#      Filename : Login.c
#   Description : not thing...
#
=============================================================================*/

#include <unistd.h>

#include <stdlib.h>
#include <memory.h>
#include "AsyncUpdate.h"
#include "param.h"
#include "TLP.h"
#include "Login.h"

// login process (vg3.0, page 181)
// some AKV not include 
TLP_Error Send_LoginPacket(void)
{
	TLP_Error		res;
	PacketInfo   	Info;
	char			buff[32];
	char			*pData;

	// app data
	pData = (char *)buff;
	
	*(pData++) = 0x41;		// 0x41   --> object request
	*(pData++) = 0x00;
	*(pData++) = 0x01;		// 0x0001 --> login request
	
	// --- tP AKV ---		// tP     --> device protocol type					
	// attr					// 0x04   --> we use the vg3.0 protocol
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x02;
	// key
	*(pData++) = 't';
	*(pData++) = 'P';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x01;
	// value
	*(pData++) = 0x04;

	Info.TransmitType = _TransmitControl(TypeA, Packet_OUT);	
	Info.Option_data  = NULL;
	Info.SEQ_num      = 0;
	Info.Option_len   = 0;
	Info.App_data     = (uint8_t *)buff;
	Info.App_len      = (int)(pData - (char *)buff);

	// send packet
	res = TLP_SendPacket(&Info);	
	
	if(res!=ERROR_NONE){
		DEBUG("%s : Login Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Login Packet send ...OK\n",__func__);
	}
	
	return res;
}


TLP_Error Check_LoginACK(void)
{
	char	*buff;
	char 	len;	
	char 	*p;
	char 	type;
	int	 	action;
	char 	ack;

	TLP_Error res;

	buff = malloc(1024);
			
	len = TLP_ReceivePacket( (char *)buff );

	if(len > 0){		
		p = (char *)buff + sizeof(TLP_Head_Struct);

		type = *(p++);
		action = *(p++);
		action = ((action&0x00FF)<<8)+(*(p++));
		ack = *(p++);
	
		if((type==0x41)&&(action==0x0001)){
			if(ack==0x01){
				res = ERROR_NONE;
			}else{
				res = ERROR_UNKNOW;
				DEBUG("%s : Login...ACK ERROR.\n",__func__);
			}
		} else {
			res = ERROR_UNKNOW;
			DEBUG("%s : This is not a Login Packet ACK.\n",__func__);
		}
	} else {
		res = ERROR_UNKNOW;
		DEBUG("%s : No data in the GSM buff.\n",__func__);
	}

	free(buff);
	return res;
}


int Try_Login(void)
{
	int timeout;

	timeout = 5;
	do{
		DEBUG("%s : Try to login....\n",__func__);
		Send_LoginPacket();
		sleep(8);
		if( Check_LoginACK()==ERROR_NONE ){	
			DEBUG("%s : Login...ACK OK.\n",__func__);
			break;
		} else {	
			DEBUG("%s : Login...ACK ERROR.\n",__func__);
			timeout--;
		}

		DEBUG("\n\n");
	
	}while( timeout>0 );
	
	if(timeout!=0) 	return 0;
	else			return -1;
}


TLP_Error Send_RegistPacket(void)
{
	TLP_Error		res;
	PacketInfo   	Info;
	char*			buff;
	char			*pData;
	int				len;

	buff = malloc(1024);

	// app data
	pData = (char *)buff;
	
	*(pData++) = 0x41;		// 0x41   --> object request
	*(pData++) = 0x00;
	*(pData++) = 0x00;		// 0x0001 --> regist request
	
	// --- ver AKV ---		// ver    --> device version num.
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x03;
	// key
	*(pData++) = 'v';
	*(pData++) = 'e';	
	*(pData++) = 'r';
	// V_len
	len = Get_ver(pData+2);
	*(pData++) = (char)(len >> 8);
	*(pData++) = (char)len;
	pData = pData + len;

	// --- ven AKV ---		// ven    --> manufacturer id num.
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x03;
	// key
	*(pData++) = 'v';
	*(pData++) = 'e';	
	*(pData++) = 'n';
	// V_len
	len = Get_ven(pData+2);
	*(pData++) = (char)(len >> 8);
	*(pData++) = (char)len;
	pData = pData + len;

	// --- group AKV ---	// ver    --> group id num.
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x05;
	// key
	*(pData++) = 'g';
	*(pData++) = 'r';	
	*(pData++) = 'o';
	*(pData++) = 'u';
	*(pData++) = 'p';
	// V_len
	len = Get_group_id(pData+2);
	*(pData++) = (char)(len >> 8);
	*(pData++) = (char)len;
	pData = pData + len;

	Info.TransmitType = _TransmitControl(TypeA, Packet_OUT);	
	Info.Option_data  = NULL;
	Info.SEQ_num      = 0;
	Info.Option_len   = 0;
	Info.App_data     = (uint8_t *)buff;
	Info.App_len      = (int)(pData - (char *)buff);

	// send packet
	res = TLP_SendPacket(&Info);	
	
	if(res!=ERROR_NONE){
		DEBUG("%s : Regist Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Regist Packet send ...OK\n",__func__);
	}

	free(buff);	
	return res;
}

TLP_Error Check_RegistACK(void)
{
	char	*buff;
	char 	len;	
	char 	*p;
	char 	type;
	int	 	action;
	char 	ack;

	TLP_Error res;

	buff = malloc(1024);
			
	len = TLP_ReceivePacket( (char *)buff );

	if(len > 0){	
		p = (char *)buff + sizeof(TLP_Head_Struct);
		
		type   = *(p++);
		action = *(p++);
		action = ((action&0x00FF)<<8)+(*(p++));
		ack    = *(p++);
	
		if((type==0x41)&&(action==0x0000)){
			if(ack==0x01){
				res = ERROR_NONE;
			}else{
				res = ERROR_UNKNOW;
				DEBUG("%s : Login...ACK ERROR.\n",__func__);
			}
		}else{
			res = ERROR_UNKNOW;
			DEBUG("%s : This is not a Login Packet ACK.\n",__func__);
		}
	}else{
		res = ERROR_UNKNOW;
		DEBUG("%s : No data in the GSM buff.\n",__func__);
	}

	free(buff);
	return res;
}

int Try_Regist(void)
{
	int timeout;

	timeout = 5;
	do{
		DEBUG("%s : Try to Regist....\n",__func__);
		Send_RegistPacket();
		sleep(8);
		if( Check_RegistACK()==ERROR_NONE ){	
			DEBUG("%s : Regist...ACK OK.\n",__func__);
			break;
		} else {	
			DEBUG("%s : Regist...ACK ERROR.\n",__func__);
			timeout--;
		}

		DEBUG("\n\n");
	
	}while( timeout>0 );
	
	if(timeout!=0) 	return 0;
	else			return -1;
}

