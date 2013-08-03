/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-12-29 16:26
#      Filename : RC_VelocityAnomaly.c
#   Description : not thing...
#          tips : -- 2013.02.21 
#                   do not need a list to save the condition,
#                   so, add the Speed_Cond to save the active condition.
#
=============================================================================*/

#include <assert.h>
#include <memory.h>
#include <pthread.h>
#include "ReportCondition.h"

LIST_HEAD( VelocityList );		// normal report
LIST_HEAD( VelocityList_T );	// time depend report

static pthread_mutex_t Cond_Mutex = PTHREAD_MUTEX_INITIALIZER;
static SpeedAnomaly_Condition Speed_Cond;

static void Condition_Save   (SpeedAnomaly_Condition   * pcondition_in);
static void Condition_Save_T (SpeedAnomaly_Condition_T * pcondition_in);

static TLP_Error RC_VelocityAnomaly_ACK  ( SpeedAnomaly_Condition   * pcondition );
static TLP_Error RC_VelocityAnomaly_ACK_T( SpeedAnomaly_Condition_T * pcondition );

static void Condition_Printf  (SpeedAnomaly_Condition   *pcondition);
static void Condition_Printf_T(SpeedAnomaly_Condition_T *pcondition);

// to protect the Speed_Cond.
void Speed_Cond_lock(void)
{
    pthread_mutex_lock(&Cond_Mutex);
}

void Speed_Cond_unlock(void)
{
    pthread_mutex_unlock(&Cond_Mutex);
}

// Velocity Anomaly input packet process
// vg3.0 (page.110)
TLP_Error RC_VelocityAnomaly_Parser( Packet_Struct *pPacket )
{
    TLP_Error               res;
	char 					*pdata;
	SpeedAnomaly_Condition	condition;

	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

	char type;
	char speed;
	int	 interval;

	DEBUG("%s : Velocity Anomaly Packet receive ...\n",__func__);
	DEBUG("%s : Start to parse Packet ...\n",__func__);
	
	pdata = (char *)(pPacket->Data);		                                // pointer to the app data

	pdata = RC_Parse_CommInfo(pdata, &condition.Info);				        // parse the comm info
																	
	// 't' AKV
	pdata = SimpleAKV_Parser( pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x01 );
	assert( value_len == 0x01 );
	assert( *(pkey)   == 't' );

	type  = *(pvalue);	
	
	// 's' AKV
	pdata = SimpleAKV_Parser( pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x01 );
	assert( value_len == 0x01 );
	assert( *(pkey)   == 's' );

	speed  = *(pvalue);

	// 'l' AKV
	pdata = SimpleAKV_Parser( pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x01 );
	assert( value_len == 0x02 );
	assert( *(pkey)   == 'l' );

	interval = (((int)*(pvalue++))&0x000000FF);
	interval = (interval << 0x08) | (((int)*(pvalue++))&0x000000FF);

	condition.type     = type;
	condition.speed    = speed;
	condition.interval = interval;

	Condition_Save(&condition);										// save to list

    ShowConditons_Speed( &VelocityList );

    res = RC_VelocityAnomaly_ACK(&condition);

	return res;
}

static TLP_Error RC_VelocityAnomaly_ACK( SpeedAnomaly_Condition * pcondition )
{
	TLP_Error	res;
	PacketInfo	info;
	char *		buff;
	char *		pdata;
	
	buff = malloc(1024);

	pdata = RC_Enpacket_DefaultAPP(buff, &pcondition->Info);

	if( pcondition->Info.Need_Confirm == 0x01 ){
		pdata = RC_Enpacket_DefaultAKV(pdata, &pcondition->Info);	
	}

	info.TransmitType = _TransmitControl(TypeA, Packet_OUT);
    info.SEQ_num      = pcondition->Info.SEQ_num;
	info.Option_data  = NULL;
	info.Option_len   = 0;
	info.App_data     = (uint8_t *)buff;
	info.App_len      = (int)(pdata - (char *)buff);

    // send packet
	res = TLP_SendPacket(&info);

	if(res!=ERROR_NONE){
		DEBUG("%s : VelocityAnomaly ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : VelocityAnomaly ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}

//
//-----------------------------------------------------------------------------------------
//

// Time depend Velocity Anomaly input packet process
// vg3.0 (page.111)
TLP_Error RC_VelocityAnomaly_Parser_T( Packet_Struct *pPacket )
{
    TLP_Error                   res;
	char 						*pdata;
	SpeedAnomaly_Condition_T	condition;

	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

	char speed;
	int	 interval;

	DEBUG("%s : Time depend Velocity Anomaly Packet receive ...\n",__func__);
	DEBUG("%s : Start to parse Packet ...\n",__func__);
	
	pdata = (char *)(pPacket->Data);		                                // pointer to the app data
	
	pdata = RC_Parse_CommInfo(pdata, &condition.Info);				        // parse the comm info

	// BE time parse
	pdata = BEtime_Parser(pdata, &(condition.TimeSet));

	// 's' AKV
	pdata = SimpleAKV_Parser( pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x01 );
	assert( value_len == 0x02 );
	assert( *(pkey)   == 's' );

	speed = *(pvalue++);
    speed = (speed*10) + *pvalue;

    // 'ix' AKV
	pdata = SimpleAKV_Parser( pdata, &attr, &key_len, &value_len, &pkey, &pvalue);

	// 'l' AKV
	pdata = SimpleAKV_Parser( pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x01 );
	assert( value_len == 0x02 );
	assert( *(pkey)   == 'l' );

	interval = *(pvalue++);
	interval = (interval << 0x08) | *(pvalue);

	condition.index    = 0x00;
	condition.speed    = speed;
	condition.interval = interval;

	Condition_Save_T(&condition);										// save to list
    
    ShowConditons_Speed_T( &VelocityList_T );

    res = RC_VelocityAnomaly_ACK_T(&condition);

	return res;
}

static TLP_Error RC_VelocityAnomaly_ACK_T( SpeedAnomaly_Condition_T * pcondition )
{
	TLP_Error	res;
	PacketInfo	info;
	char *		buff;
	char *		pdata;
	
	buff = malloc(1024);

	pdata = RC_Enpacket_DefaultAPP(buff, &pcondition->Info);

	if( pcondition->Info.Need_Confirm == 0x01 ){
		pdata = RC_Enpacket_DefaultAKV(pdata, &pcondition->Info);	
	}

	info.TransmitType = _TransmitControl(TypeA, Packet_OUT);
    info.SEQ_num      = pcondition->Info.SEQ_num;
	info.Option_data  = NULL;
	info.Option_len   = 0;
	info.App_data     = (uint8_t *)buff;
	info.App_len      = (int)(pdata - (char *)buff);

    // send packet
	res = TLP_SendPacket(&info);

	if(res!=ERROR_NONE){
		DEBUG("%s : Time depend VelocityAnomaly ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Time depend VelocityAnomaly ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}

static void Condition_Save(SpeedAnomaly_Condition * pcondition_in)
{
/*
	struct list_head		*plist;	
	SpeedAnomaly_Condition	*pcondition;

	// check if we aleady have a condition in the list
	list_for_each(plist, &VelocityList){
		pcondition = list_entry(plist, SpeedAnomaly_Condition, list);

		if( strcmp((const char *)pcondition->Info.RID, (const char *)pcondition_in->Info.RID)==0 ){
			break;
		}
	}

	if( plist!=&VelocityList ){		// found
        
        // save the new value into list.
        memcpy(&pcondition->Info, &pcondition_in->Info, sizeof(Report_Info));
        pcondition->speed    = pcondition_in->speed; 
        pcondition->interval = pcondition_in->interval; 
	}else{
		pcondition = malloc(sizeof(SpeedAnomaly_Condition));	
		memcpy(pcondition, pcondition_in, sizeof(SpeedAnomaly_Condition));
		list_add(&pcondition->list, &VelocityList);
	}
*/
    Speed_Cond_lock();
    memcpy(&Speed_Cond, pcondition_in, sizeof(SpeedAnomaly_Condition));
    Speed_Cond_unlock();
}

void Get_Speed_Cond(SpeedAnomaly_Condition *pcond)
{
    Speed_Cond_lock();
    memcpy(pcond, &Speed_Cond, sizeof(SpeedAnomaly_Condition));
    Speed_Cond_unlock();
}

static void Condition_Save_T(SpeedAnomaly_Condition_T * pcondition_in)
{
	struct list_head		 *plist;	
	SpeedAnomaly_Condition_T *pcondition;

	// check if we aleady have a condition in the list
	list_for_each(plist, &VelocityList_T){
		pcondition = list_entry(plist, SpeedAnomaly_Condition_T, list);

		if( strcmp((const char *)pcondition->Info.RID, (const char *)pcondition_in->Info.RID)==0 ){
			break;
		}
	}

	if( plist!=&VelocityList_T ){		// found
        
        // save the new value into list.
        memcpy(&pcondition->Info, &pcondition_in->Info, sizeof(Report_Info));
        pcondition->index    = pcondition_in->index;
        pcondition->speed    = pcondition_in->speed; 
        pcondition->interval = pcondition_in->interval; 
        memcpy(&pcondition->TimeSet, &pcondition_in->TimeSet, sizeof(TimeSet_Struct));
		
	}else{
		pcondition = malloc(sizeof(SpeedAnomaly_Condition_T));	
		memcpy(pcondition, pcondition_in, sizeof(SpeedAnomaly_Condition_T));
		list_add(&pcondition->list, &VelocityList_T);
	}
}


void ShowConditons_Speed(struct list_head *phead)
{
	struct list_head        *plist;
	SpeedAnomaly_Condition  *pcondition;
	
    fprintf(stderr,"\n=========================\n");
	
    list_for_each(plist, phead) {
		pcondition = list_entry(plist, SpeedAnomaly_Condition, list);
		
		Condition_Printf(pcondition);
		fprintf(stderr,"=========================\n");
	}
}

static void Condition_Printf(SpeedAnomaly_Condition *pcondition)
{
	fprintf(stderr,"Report ID       : %s\n", pcondition->Info.RID);
	fprintf(stderr,"Report Interval : %d\n", pcondition->Info.Interval);
	fprintf(stderr,"Report Cnt      : %d\n", pcondition->Info.Cnt);
	fprintf(stderr,"Report Action   : %d\n", pcondition->Info.Action);	

    
	fprintf(stderr,"Speed type      : %d\n",      pcondition->type);	
	fprintf(stderr,"Speed speed     : %d km/h\n", pcondition->speed);	
	fprintf(stderr,"Speed last      : %d s\n",    pcondition->speed);	
}

void ShowConditons_Speed_T(struct list_head *phead)
{
	struct list_head            *plist;
	SpeedAnomaly_Condition_T    *pcondition;
	
    fprintf(stderr,"\n=========================\n");
	
    list_for_each(plist, phead) {
		pcondition = list_entry(plist, SpeedAnomaly_Condition_T, list);
		
		Condition_Printf_T(pcondition);
		fprintf(stderr,"=========================\n");
	}
}

static void Condition_Printf_T(SpeedAnomaly_Condition_T *pcondition)
{
    int i;

	fprintf(stderr,"Report ID       : %s\n", pcondition->Info.RID);
	fprintf(stderr,"Report Interval : %d\n", pcondition->Info.Interval);
	fprintf(stderr,"Report Cnt      : %d\n", pcondition->Info.Cnt);
	fprintf(stderr,"Report Action   : %d\n", pcondition->Info.Action);	

    
	fprintf(stderr,"Report index    : %d\n",      pcondition->index);	
	fprintf(stderr,"Speed speed     : %d km/h\n", pcondition->speed);	
	fprintf(stderr,"Speed last      : %d s\n",    pcondition->speed);	

    fprintf(stderr,"The time Set    :\n");
    for(i=0;i<pcondition->TimeSet.TimeSet_Count;i++){
        fprintf(stderr,"  *Time node %d :\n",i);
        BEtimePrintf(&(pcondition->TimeSet.BE_Time[i]));
    }

}

void BEtimePrintf(TimeSlot_Struct *pTimeSlot)
{
    fprintf(stderr,"    | Btime     : %02d:%02d:%02d\n", pTimeSlot->Btime.hour,pTimeSlot->Btime.minute,pTimeSlot->Btime.second);
    fprintf(stderr,"    | Etime     : %02d:%02d:%02d\n", pTimeSlot->Etime.hour,pTimeSlot->Etime.minute,pTimeSlot->Etime.second);
}

