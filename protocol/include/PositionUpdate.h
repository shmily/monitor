/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-08 04:16
#      Filename : PositionUpdate.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _POS_UPDATE_RULE_H_
#define	_POS_UPDATE_RULE_H_

#include "TLP.h"
#include "myList.h"

#define _ID_LEN		256

#define	is_TimeChange( mask )	((mask & 0x0000FF00) == ('7'<<8))
#define	is_SendCount( mask )	((mask & 0x000000FF) == '1')

// date time struct
typedef struct _Time_DT_
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}Time_DT;

// time pointer struct
typedef struct _Time_D_
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}Time_D;

// date struct
typedef struct _Date_D_
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
}Date_D;

typedef struct _Rule_info_
{
    int     SEQ_num;
    char    cmd;
    int     action;
    char    id_len;
    char    ID[_ID_LEN];
}Rule_Info;

// rule effective struct
typedef struct _Rule_Effective_
{
    uint8_t  need_confirm;
	uint8_t  are_move;
	uint16_t cnt;
	uint16_t interval;
}
Rule_Effective;

union Emap{
    Time_DT     etime;
    int         cnt;
};

// rule map struct
typedef struct _Rule_map
{
    Time_DT     btime;
    int         tmap;
    union Emap  emap;
}Rule_map;

typedef struct _Rule_Struct_
{
	Rule_Info           info;
	Rule_Effective 		effective;
	int                 mask;
	Rule_map		    rulemap;
	struct list_head 	list;
}Rule_Struct;


// function


char *PU_Parse_CommInfo(const char *pdata, Rule_Info *pinfo);
char *PU_Parse_RuleEffective(const char *pdata, Rule_Effective *peff);
char *PU_Enpacket_DefaultAPP(char *pdata, Rule_Info *pinfo);

char *TimeDT_AKV_Parser(char *pdata_start, Time_DT *ptime);
char *TimeChange_AKV_Parser(char *pdata_start, int *period);
char *maxCnt_AKV_Parser(char *pdata_start, int *pcnt);
char *DistanceChange_AKV_Parser(char *pdata_start, int *pdistance);
char *RuleMap_AKV_Parser(char *pdata_start, int *pmask, Rule_map *prule);


TLP_Error PU_PositionTrace_Parser( Packet_Struct *pPacket );
TLP_Error PU_PositionTrace_ACK( Rule_Struct *prule );
TLP_Error PU_StopMonitoring_Parser( Packet_Struct *pPacket );
void ShowRules(struct list_head *phead);

TLP_Error RulePacket_Handle( Packet_Struct *pPacket );

void PositionUpdate_Rule_Del(Rule_Struct *pRule);
void Position_Rule_lock(void);
void Position_Rule_unlock(void);

int isNeed_Updata_MonitorPos(void);
int Get_Monitor_SEQ_num(void);

#endif
