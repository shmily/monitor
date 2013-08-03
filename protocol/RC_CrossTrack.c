/*
=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-11 23:44
#      Filename : RC_CrossTrack.c
#   Description : not thing...
#          tips : -- 2013.02.21 
#
=============================================================================
*/

#include <assert.h>
#include <memory.h>
#include <pthread.h>
#include "ReportCondition.h"

LIST_HEAD( CrossTrackList );

static pthread_mutex_t          Cond_Mutex = PTHREAD_MUTEX_INITIALIZER;

static void Condition_Save              ( CrossTrack_Condition *pcondition_in );
static      TLP_Error RC_CrossTrack_ACK ( CrossTrack_Condition *pcondition );
static void Condition_Printf            ( CrossTrack_Condition *pcondition );
static void PointPrintf                 ( Point_Struct *pPoint );

// to protect the CrossTrack_Cond.
void CrossTrack_Cond_lock(void)
{
    pthread_mutex_lock(&Cond_Mutex);
}

void CrossTrack_Cond_unlock(void)
{
    pthread_mutex_unlock(&Cond_Mutex);
}

// input packet process
// vg3.0 (page.122)
TLP_Error RC_CrossTrack_Parser( Packet_Struct *pPacket )
{
    TLP_Error               res;
	char 				    *pdata;
	CrossTrack_Condition     condition;

	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

	int	 error;

	DEBUG("%s : CrossTrack Packet receive ...\n",__func__);
    DEBUG("%s : Start to parse Packet ...\n",__func__);
	
    memset(&condition, 0, sizeof(CrossTrack_Condition));

	pdata = (char *)(pPacket->Data);		                                // pointer to the app data

	pdata = RC_Parse_CommInfo(pdata, &condition.Info);				        // parse the comm info
	pdata = BEtime_Parser(pdata, &(condition.TimeSet));
    pdata = PathAKV_Parser(pdata, &(condition.path));
    pdata = IndexAKV_Parser(pdata, &(condition.index));

	// 'yD' AKV
	pdata = SimpleAKV_Parser( pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x02 );
	assert( value_len == 0x02 );
	assert( *(pkey++) == 'y' );
	assert( *(pkey++) == 'D' );
	
    error = (((int)*(pvalue++))&0x000000FF);
	error = (error << 0x08) | (((int)*(pvalue++))&0x000000FF);

	condition.error = error;

	Condition_Save(&condition);										// save to list

    ShowConditons_CrossTrack( &CrossTrackList );

    res = RC_CrossTrack_ACK(&condition);

	return res;
}

static TLP_Error RC_CrossTrack_ACK( CrossTrack_Condition * pcondition )
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
		DEBUG("%s : CrossTrack ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : CrossTrack ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}

static void Condition_Save(CrossTrack_Condition * pcondition_in)
{
	struct list_head		*plist;	
	CrossTrack_Condition	*pcondition;

	// check if we aleady have a condition in the list
	list_for_each(plist, &CrossTrackList){
		pcondition = list_entry(plist, CrossTrack_Condition, list);

		if( pcondition->index == pcondition_in->index ){
			break;
		}
	}

	if( plist!=&CrossTrackList ){		// found
        
        // save the new value into list.
        memcpy(&pcondition->Info, &pcondition_in->Info, sizeof(Report_Info));
        pcondition->index = pcondition_in->index;
        pcondition->error = pcondition_in->error;
        memcpy(&pcondition->TimeSet, &pcondition_in->TimeSet, sizeof(TimeSet_Struct));
        memcpy(&pcondition->path, &pcondition_in->path, sizeof(Line_Struct));
	}else{
		pcondition = malloc(sizeof(CrossTrack_Condition));	
		memcpy(pcondition, pcondition_in, sizeof(CrossTrack_Condition));
		list_add(&pcondition->list, &CrossTrackList);
	}
}

void ShowConditons_CrossTrack(struct list_head *phead)
{
	struct list_head        *plist;
	CrossTrack_Condition    *pcondition;
	
    fprintf(stderr,"\n=========================\n");
	
    list_for_each(plist, phead) {
		pcondition = list_entry(plist, CrossTrack_Condition, list);
		
		Condition_Printf(pcondition);
		fprintf(stderr,"=========================\n");
	}
}

static void Condition_Printf(CrossTrack_Condition *pcondition)
{
    int i;

	fprintf(stderr,"Report ID        : %s\n", pcondition->Info.RID);
	fprintf(stderr,"Report Interval  : %d\n", pcondition->Info.Interval);
	fprintf(stderr,"Report Cnt       : %d\n", pcondition->Info.Cnt);
	fprintf(stderr,"Report Action    : %d\n", pcondition->Info.Action);	

    
	fprintf(stderr,"Report index     : %d\n",   pcondition->index);	
	fprintf(stderr,"CrossTrack error : %d m\n", pcondition->error);	

    fprintf(stderr,"The time Set     :\n");
    for(i=0;i<pcondition->TimeSet.TimeSet_Count;i++){
        fprintf(stderr,"  *Time node %d :\n",i);
        BEtimePrintf(&(pcondition->TimeSet.BE_Time[i]));
    }

    fprintf(stderr, "Path ---------> :\n");
    for(i=0; i<pcondition->path.sum; i++){
        fprintf(stderr, "  *Point node %d :\n", i);
        PointPrintf(&pcondition->path.point[i]);
    }

}

static void PointPrintf(Point_Struct *pPoint)
{
	fprintf(stderr,"	| Latitude  : %d\n", pPoint->Latitude);
	fprintf(stderr,"	| Longitude : %d\n", pPoint->Longitude);
}
