/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-08 04:06
#      Filename : PositionUpdate.c
#   Description : not thing...
#
=============================================================================*/

#include <assert.h>
#include <memory.h>
#include "ReportCondition.h"
#include "PositionUpdate.h"
#include "TLP.h"


TLP_Error RulePacket_Handle( Packet_Struct *pPacket )
{
    char            cmd;
    char            *p;
	TLP_Error		res;
	
    p = (char *)(pPacket->Data + sizeof(TLP_Head_Struct));
	cmd = *p;

    if( cmd == _TypeA_PositionRule_Del ){
        res = PU_StopMonitoring_Parser( pPacket );
    } else if( cmd == _TypeA_PositionRule_Set ) {
        res = PU_PositionTrace_Parser( pPacket );
    } else {
		res = ERROR_NONE;
    }

    return res;
}

char *PU_Parse_CommInfo(const char *pdata, Rule_Info *pinfo)
{
    char    cmd;
	int 	action;
	char	id_len;

	memset(pinfo, 0, sizeof(Rule_Info));
	memset(pinfo->ID, '\0', _ID_LEN);

    pdata = TLP_Head_Parser(pdata, &pinfo->SEQ_num);    // save the SEQ num

    cmd = *(pdata++);

	action = ((int)*(pdata++))&0x000000FF;
	action = (action << 8) | (((int)*(pdata++))&0x000000FF);

    id_len = *(pdata++);

	memcpy(pinfo->ID, pdata, id_len);		// save RID

    pinfo->cmd    = cmd;
    pinfo->action = action;
    pinfo->id_len = id_len;

#ifdef __DEBUG__
	DEBUG("%s :          cmd : 0x%02X\n", __func__, cmd);
	DEBUG("%s :       id_len : %d\n",     __func__, id_len);
	DEBUG("%s :           ID : %s\n",     __func__, pinfo->ID);
#endif

    pdata += id_len;

	return (char *)pdata;
}

char *PU_Parse_RuleEffective(const char *pdata, Rule_Effective *peff)
{
    char    need_confirm;
	char 	are_move;
	int	    cnt;
    int     interval;
    int     tmp;

    need_confirm = *(pdata++);
    are_move     = *(pdata++);
   
    tmp = 0;
    tmp = ((int)*(pdata++))&0x000000FF;
    tmp = (tmp<<8) | (((int)*(pdata++))&0x000000FF);
    cnt = tmp;

    tmp = 0;
    tmp = ((int)*(pdata++))&0x000000FF;
    tmp = (tmp<<8) | (((int)*(pdata++))&0x000000FF);
    interval = tmp;
    
    peff->need_confirm = need_confirm;
    peff->are_move     = are_move;
    peff->cnt          = cnt;
    peff->interval     = interval;

#ifdef __DEBUG__
	DEBUG("%s : need_confirm : %d\n", __func__, need_confirm);
	DEBUG("%s :     are_move : %d\n", __func__, are_move);
	DEBUG("%s :          cnt : %d\n", __func__, cnt);
	DEBUG("%s :     interval : %d\n", __func__, interval);
#endif

	return (char *)pdata;
}

char *PU_Enpacket_DefaultAPP(char *pdata, Rule_Info *pinfo)
{
	char *pdata_iner;
    int  tmp;
	
	pdata_iner = pdata;

	*(pdata_iner++) = pinfo->cmd;		            // cmd

	tmp = pinfo->action;
	*(pdata_iner++) = (char )(tmp >> 8);			// ACTION
	*(pdata_iner++) = (char )(tmp);

	*(pdata_iner++) = pinfo->id_len;				// id_len

	memcpy(pdata_iner, pinfo->ID, pinfo->id_len);	// ID
	pdata_iner += pinfo->id_len;

	*(pdata_iner++) = 0x01;							// ACK, 0x01 -> success!

	return pdata_iner;
}

char *RuleMap_AKV_Parser(char *pdata_start, int *pmask, Rule_map *prule)
{
    int     type;
    int     mask;

    // b map
    type = *(pdata_start + 2);
    assert( type == '6' );

    mask = 0;
    mask = (type<<16);
    pdata_start = TimeDT_AKV_Parser(pdata_start, &prule->btime);
    
    // t map
    type = *(pdata_start + 2);
    assert( (type=='7')||(type == '9') );
 
    mask = mask | (type<<8);
    if(type == '7'){
        pdata_start = TimeChange_AKV_Parser(pdata_start, &prule->tmap);
    } else { 
        pdata_start = DistanceChange_AKV_Parser(pdata_start, &prule->tmap);
    }

    // e map
    type = *(pdata_start + 2);
    assert( (type=='6')||(type == '1') );

    if( type == '6' ){
        pdata_start = TimeDT_AKV_Parser(pdata_start, &prule->emap.etime);
    }else{
        pdata_start = maxCnt_AKV_Parser(pdata_start, &prule->emap.cnt); 
    }

    mask = mask | type;

    *pmask = mask;

    return pdata_start;
}

char *TimeDT_AKV_Parser(char *pdata_start, Time_DT *ptime)
{
    // for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

    // 'rS' AKV
    pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 1    );
	assert( value_len == 6    );
	assert( *(pkey++) == '6'  );

    ptime->year   = *(pvalue++);
    ptime->month  = *(pvalue++);
    ptime->day    = *(pvalue++);
    ptime->hour   = *(pvalue++);
    ptime->minute = *(pvalue++);
    ptime->second = *(pvalue++);

    return pdata_start;
}

char *TimeChange_AKV_Parser(char *pdata_start, int *period)
{
    // for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;
    int  tmp;

    // 'rS' AKV
    pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 1    );
	assert( value_len == 2    );
	assert( *(pkey++) == '7'  );
    
    tmp = 0;
    tmp = ((int)*(pvalue++))&0x000000FF;
    tmp = (tmp<<8) | (((int)*(pvalue++))&0x000000FF);

    *period = tmp;

    return pdata_start;
}

char *maxCnt_AKV_Parser(char *pdata_start, int *pcnt)
{
    // for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;
    int  tmp;

    // 'rS' AKV
    pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 1    );
	assert( value_len == 4    );
	assert( *(pkey++) == '1'  );
    
    tmp = 0;
    tmp = (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) | (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) | (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) | (((int)*(pvalue++))&0x000000FF);

    *pcnt = tmp;

    return pdata_start;
}

char *DistanceChange_AKV_Parser(char *pdata_start, int *pdistance)
{
    // for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;
    int  tmp;

    // 'rS' AKV
    pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 1    );
	assert( value_len == 4    );
	assert( *(pkey++) == '9'  );
    
    tmp = 0;
    tmp = (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) | (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) | (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) | (((int)*(pvalue++))&0x000000FF);

    *pdistance = tmp;

    return pdata_start;
}
