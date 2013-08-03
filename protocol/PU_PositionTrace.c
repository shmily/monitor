/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-09 17:42
#      Filename : PU_PositionTrace.c
#   Description : not thing...
#
=============================================================================*/

#include <memory.h>
#include <pthread.h>
#include "ReportCondition.h"

LIST_HEAD( UpdateRuleList );
static pthread_mutex_t Rule_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int isNeed_Update = 0;

static void Condition_Save(Rule_Struct *prule_in, struct list_head *rule_list);
static void Rule_Printf(Rule_Struct *pRule);
static void KeyValue_Printf(int mask, Rule_map *prule);

void Position_Rule_lock(void)
{
    pthread_mutex_lock(&Rule_Mutex);
}

void Position_Rule_unlock(void)
{
    pthread_mutex_unlock(&Rule_Mutex);
}

// position monitoring rule
// vg3.0 (page.91)
TLP_Error PU_PositionTrace_Parser( Packet_Struct *pPacket )
{
    TLP_Error       res;
	char 		    *pdata;
    Rule_Struct     rule;

	DEBUG("%s : Position monitoring Packet receive ...\n",__func__);
	DEBUG("%s : Start to parse Packet ...\n",__func__);

    memset(&rule, 0, sizeof(Rule_Struct));
	pdata = (char *)(pPacket->Data);		            // pointer to the app data

    isNeed_Update = 0x01;

    pdata = PU_Parse_CommInfo(pdata, &rule.info);
    pdata = PU_Parse_RuleEffective(pdata, &rule.effective);

    pdata = RuleMap_AKV_Parser(pdata, &rule.mask, &rule.rulemap);

    KeyValue_Printf(rule.mask, &rule.rulemap);

    Condition_Save(&rule, &UpdateRuleList);

    ShowRules(&UpdateRuleList);

    res = PU_PositionTrace_ACK(&rule);

    return res;
}

TLP_Error PU_PositionTrace_ACK( Rule_Struct *prule )
{
	TLP_Error	res;
	PacketInfo	info;
	char *		buff;
	char *		pdata;
	
	buff = malloc(1024);

	pdata = PU_Enpacket_DefaultAPP(buff, &prule->info);

    info.TransmitType = _TransmitControl(TypeA, Packet_OUT);
	info.Option_data  = NULL;
    info.SEQ_num      = prule->info.SEQ_num;
	info.Option_len   = 0;
	info.App_data     = (uint8_t *)buff;
	info.App_len      = (int)(pdata - (char *)buff);

    // send packet
	res = TLP_SendPacket(&info);

	if(res!=ERROR_NONE){
		DEBUG("%s : position monitoring ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : position monitoring ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}

static void Condition_Save(Rule_Struct *prule_in, struct list_head *rule_list)
{
	Rule_Struct         *prule;

    prule = (Rule_Struct *)malloc(sizeof(Rule_Struct));	
    memcpy(prule, prule_in, sizeof(Rule_Struct));

    Position_Rule_lock();
	list_add(&prule->list, rule_list);
    Position_Rule_unlock();
}

void PositionUpdate_Rule_Del(Rule_Struct *pRule)
{
    struct list_head *plist;

    plist = &pRule->list;
    list_del(plist);

    free(pRule);
}

void ShowRules(struct list_head *phead)
{
	struct list_head *plist;
	Rule_Struct *pRule;
	
	list_for_each(plist, phead) {
		pRule = list_entry(plist, Rule_Struct, list);
		
		Rule_Printf(pRule);
		fprintf(stderr,"=========================\n");
	}
}

static void Rule_Printf(Rule_Struct *pRule)
{
	fprintf(stderr,"Rule ID       : %s\n",   pRule->info.ID);
	fprintf(stderr,"Rule AreMove  : %d\n",   pRule->effective.are_move);
	fprintf(stderr,"Rule Cnt      : %d\n",   pRule->effective.cnt);
	fprintf(stderr,"Rule Interval : %d s\n", pRule->effective.interval);	

	KeyValue_Printf(pRule->mask, &pRule->rulemap);
	
}

static void KeyValue_Printf(int mask, Rule_map *prule)
{

	fprintf(stderr,"--- BMP Rule ---\n");
	fprintf(stderr,"BMP Rule Key  : %d\n", ((mask >> 16)&0xFF)-0x30 );
    fprintf(stderr,"Value         : %02d-%02d-%02d\n",  prule->btime.year,
                                                        prule->btime.month,
                                                        prule->btime.day);

    fprintf(stderr,"                %02d:%02d:%02d\n",  prule->btime.hour,
                                                        prule->btime.minute,
                                                        prule->btime.second);

	fprintf(stderr,"--- TMP Rule ---\n");
	fprintf(stderr,"TMP Rule Key  : %d\n", ((mask >> 8)&0xFF)-0x30 );   
	fprintf(stderr,"Value         : %d\n", prule->tmap);

	fprintf(stderr,"--- EMP Rule ---\n");
	fprintf(stderr,"EMP Rule Key  : %d\n", ((mask)&0xFF)-0x30 );
    
    if( (mask&0xFF) == '6' ){
        fprintf(stderr,"Value         : %02d-%02d-%02d\n",  prule->emap.etime.year,
                                                            prule->emap.etime.month,
                                                            prule->emap.etime.day);

        fprintf(stderr,"                %02d:%02d:%02d\n",  prule->emap.etime.hour,
                                                            prule->emap.etime.minute,
                                                            prule->emap.etime.second);
    } else {
        fprintf(stderr,"Value         : %d\n", prule->emap.cnt);
    }
}



int isNeed_Updata_MonitorPos(void)
{
    int need;

    need = isNeed_Update;
    isNeed_Update = 0x00;

    return need;
}

int Get_Monitor_SEQ_num(void)
{
    return 0;
    //return prv_SEQ_num;
}
