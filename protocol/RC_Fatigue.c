/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-07 03:56
#      Filename : RC_Fatigue.c
#   Description : not thing...
#
=============================================================================*/

#include <assert.h>
#include <memory.h>
#include "ReportCondition.h"

Fatigue_Condition FatigueCondition;

TLP_Error RC_Fatigue_ACK( void );

// fatigue report input packet process
// vg3.0 (page.118)
TLP_Error RC_Fatigue_Parser( Packet_Struct *pPacket )
{
    TLP_Error   res;
	char 		*pdata;
	Report_Info Info;
    int         driving_time;
    int         break_time;
    
    char        attr;
    char        key_len;
    int         value_len;
    char        *pkey;
    char        *pvalue;

	DEBUG("%s : Fatigue Packet receive ...\n",__func__);
	DEBUG("%s : Start to parse Packet ...\n",__func__);

    memset(&Info, 0, sizeof(Report_Info));
	pdata = (char *)(pPacket->Data);		                        // pointer to the app data

	pdata = RC_Parse_CommInfo(pdata, &Info);						// parse the comm info
    
    // 'dT' AKV
    pdata = SimpleAKV_Parser(pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
    assert( attr      == 0x00 );
    assert( key_len   == 0x02 );
    assert( value_len == 0x02 );
    assert( *(pkey++) == 'r'  );
    assert( *(pkey++) == 'T'  );

    driving_time = 0;
    driving_time = (((int)*(pvalue++))&0x000000FF);
    driving_time = (driving_time<<8) + (((int)*(pvalue++))&0x000000FF);

    // 'rT' AKV
    pdata = SimpleAKV_Parser(pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
    assert( attr      == 0x00 );
    assert( key_len   == 0x02 );
    assert( value_len == 0x02 );
    assert( *(pkey++) == 'd'  );
    assert( *(pkey++) == 'T'  );

    break_time = 0;
    break_time = (((int)*(pvalue++))&0x000000FF);
    break_time = (break_time<<8) + (((int)*(pvalue++))&0x000000FF);

    FatigueCondition.max_driving_time = driving_time;
    FatigueCondition.min_break_time   = break_time;
	memcpy(&FatigueCondition.Info, &Info, sizeof(Report_Info));

    res = RC_Fatigue_ACK();

	return res;
}

TLP_Error RC_Fatigue_ACK( void )
{
	TLP_Error	res;
	PacketInfo	info;
	char *		buff;
	char *		pdata;
	
	buff = malloc(1024);

	pdata = RC_Enpacket_DefaultAPP(buff, &FatigueCondition.Info);

	if( FatigueCondition.Info.Need_Confirm == 0x01 ){
		pdata = RC_Enpacket_DefaultAKV(pdata, &FatigueCondition.Info);	
	}

	info.TransmitType = _TransmitControl(TypeA, Packet_OUT);
    info.SEQ_num      = FatigueCondition.Info.SEQ_num;
	info.Option_data  = NULL;
	info.Option_len   = 0;
	info.App_data     = (uint8_t *)buff;
	info.App_len      = (int)(pdata - (char *)buff);

    // send packet
	res = TLP_SendPacket(&info);

	if(res!=ERROR_NONE){
		DEBUG("%s : Fatigue ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Fatigue ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}
