/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-04 23:01
#      Filename : RC_Zone.c
#   Description : not thing...
#
=============================================================================*/

#include <assert.h>
#include <memory.h>
#include <pthread.h>
#include "ReportCondition.h"

LIST_HEAD( InZoneList  );		// in  zone report
LIST_HEAD( OutZoneList );	    // out zone report

static pthread_mutex_t InZone_Mutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t OutZone_Mutex = PTHREAD_MUTEX_INITIALIZER;

static void Condition_Save   (Zone_Condition * pcondition_in, struct list_head *condition_list);
static void Condition_Printf (Zone_Condition *pCondition);
static void PointPrintf      (Point_Struct *pPoint);
static void CirclePrintf     (Circle_Struct *pCircle);
static void RectanglePrintf  (Rectangle_Struct *pRectangle);
static void PolygonPrintf    (Polygon_Struct *pPolygon);

// InZone input packet process
// vg3.0 (page.127)
TLP_Error RC_InZone_Parser( Packet_Struct *pPacket )
{
    TLP_Error       res;
	char 		    *pdata;
	Zone_Condition	condition;

	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

	char shape;
	int	 index;

    Circle_Struct       circle;
    Rectangle_Struct    rect;
    Polygon_Struct      poly;
    void *              pshape_value;

	DEBUG("%s : In zone report condition Packet receive ...\n",__func__);
	DEBUG("%s : Start to parse Packet ...\n",__func__);

    memset(&condition, 0, sizeof(Zone_Condition));
    memset(&circle,    0, sizeof(Circle_Struct));
    memset(&rect,      0, sizeof(Rectangle_Struct));
    memset(&poly,      0, sizeof(Polygon_Struct));

	pdata = (char *)(pPacket->Data);		                                // pointer to the app data

	pdata = RC_Parse_CommInfo(pdata, &condition.Info);				        // parse the comm info

    // BE time parse ('T' AKV)
	pdata = BEtime_Parser(pdata, &(condition.TimeSet));

    // 'ix' AKV
    pdata = IndexAKV_Parser(pdata, &index);
	
    // 'rT' AKV
	pdata = SimpleAKV_Parser( pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x02 );
	assert( value_len == 0x01 );
	assert( *(pkey++) == 'r'  );
	assert( *(pkey++) == 'T'  );

	shape  = *(pvalue++);	

    // 'rS' AKV
    if(shape == _Circle){
        pdata = CircleAKV_Parser(pdata, &circle);
        pshape_value = &circle;
    } else if(shape == _Rectangle){ 
        pdata = RectangleAKV_Parser(pdata, &rect);
        pshape_value = &rect;
    }else{
        pdata = PolygonAKV_Parser(pdata, &poly);
        pshape_value = &poly;
    }
    condition.index        = index;
	condition.shape        = shape;
	condition.pShape_Value = pshape_value;

    InZone_Cond_lock();
    Condition_Save(&condition, &InZoneList);						// save to list
    InZone_Cond_unlock();
    ShowCondition_Zone(&InZoneList);

    res = RC_InZone_ACK(&condition);

	return res;
}

TLP_Error RC_InZone_ACK( Zone_Condition * pcondition )
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
		DEBUG("%s : In zone report condition ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : In zone report condition ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}

//--------------------------------------------------------------------------------------

// InZone input packet process
// vg3.0 (page.129)
TLP_Error RC_OutZone_Parser( Packet_Struct *pPacket )
{
    TLP_Error       res;
	char 		    *pdata;
	Zone_Condition	condition;

	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

	char shape;
	int	 index;

    Circle_Struct       circle;
    Rectangle_Struct    rect;
    Polygon_Struct      poly;
    void *              pshape_value;

    memset(&condition, 0, sizeof(Zone_Condition));
    memset(&circle,    0, sizeof(Circle_Struct));
    memset(&rect,      0, sizeof(Rectangle_Struct));
    memset(&poly,      0, sizeof(Polygon_Struct));
	
    DEBUG("%s : Out zone report condition Packet receive ...\n",__func__);
	DEBUG("%s : Start to parse Packet ...\n",__func__);
	
	pdata = (char *)(pPacket->Data);		                                // pointer to the app data
	
	pdata = RC_Parse_CommInfo(pdata, &condition.Info);				        // parse the comm info

    // BE time parse ('T' AKV)
	pdata = BEtime_Parser(pdata, &(condition.TimeSet));

    // 'ix' AKV
    pdata = IndexAKV_Parser(pdata, &index);
	
    // 'rT' AKV
	pdata = SimpleAKV_Parser( pdata, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x02 );
	assert( value_len == 0x01 );
	assert( *(pkey++) == 'r'  );
	assert( *(pkey++) == 'T'  );

	shape  = *(pvalue++);	

    // 'rS' AKV
    if(shape == _Circle){
        pdata = CircleAKV_Parser(pdata, &circle);
        pshape_value = &circle;
    } else if(shape == _Rectangle){ 
        pdata = RectangleAKV_Parser(pdata, &rect);
        pshape_value = &rect;
    }else{
        pdata = PolygonAKV_Parser(pdata, &poly);
        pshape_value = &poly;
    }
	 
    condition.index        = index;
	condition.shape        = shape;
	condition.pShape_Value = pshape_value;

    OutZone_Cond_lock();
	Condition_Save(&condition, &OutZoneList);						// save to list
    OutZone_Cond_unlock();
    ShowCondition_Zone(&OutZoneList);

    res = RC_OutZone_ACK(&condition);

	return res;
}

TLP_Error RC_OutZone_ACK( Zone_Condition * pcondition )
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
		DEBUG("%s : Out zone report condition ACK Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Out zone report condition ACK Packet send ...OK\n",__func__);
	}

	free(buff);

	return res;
}

//--------------------------------------------------------------------------------------


static void Condition_Save(Zone_Condition * pcondition_in, struct list_head *condition_list)
{
    const int _valueSize[] = {0, sizeof(Circle_Struct), sizeof(Rectangle_Struct), sizeof(Polygon_Struct)};
	struct list_head	*plist;
	Zone_Condition      *pcondition;
    char                *p;

	// check if we aleady have a condition in the list
	list_for_each(plist, condition_list){
		pcondition = list_entry(plist, Zone_Condition, list);

		if( pcondition->index == pcondition_in->index ){
			break;
		}
	}

	if( plist!=condition_list ) {  // found
        
        // 1st. free the memery.
        p = (char *)pcondition->pShape_Value;   
        free(p);

        // 2st. save the new value into list.
        memcpy(&pcondition->Info, &pcondition_in->Info, sizeof(Report_Info));
        pcondition->index = pcondition_in->index;
        pcondition->shape = pcondition_in->shape; 
        memcpy(&pcondition->TimeSet, &pcondition_in->TimeSet, sizeof(TimeSet_Struct));
        
        // 3st. malloc and save the shape vaule. 
        p = malloc( _valueSize[pcondition->shape] );
        assert( p!=NULL );
        memset(p, 0, _valueSize[pcondition->shape]);
        memcpy(p, (char *)pcondition_in->pShape_Value, _valueSize[pcondition->shape]); 
        pcondition->pShape_Value = p; 

	}else{
		pcondition = (Zone_Condition *)malloc(sizeof(Zone_Condition));	
	    memset(pcondition, 0, sizeof(Zone_Condition));	
        memcpy(pcondition, pcondition_in, sizeof(Zone_Condition));

        // malloc and save the shape vaule.
        p = malloc( _valueSize[pcondition->shape] );
        assert( p!=NULL );    
        memset(p, 0, _valueSize[pcondition->shape]);
        memcpy(p, (char *)pcondition_in->pShape_Value, _valueSize[pcondition->shape]);
        pcondition->pShape_Value = p; 
        
		list_add(&pcondition->list, condition_list);
	}
}

// ===================================================================
void ShowCondition_Zone(struct list_head *phead)
{
	struct list_head *plist;
	Zone_Condition   *pCondition;
	
	fprintf(stderr,"\n=========================\n");
	
    list_for_each(plist, phead) {
		pCondition = list_entry(plist, Zone_Condition, list);	
		Condition_Printf(pCondition);
		fprintf(stderr,"=========================\n");
	}
}

static void Condition_Printf(Zone_Condition *pCondition)
{
	int i;
	
	fprintf(stderr,"Condition RID   : %s\n", pCondition->Info.RID);
	fprintf(stderr,"Need Confirm    : %d\n", pCondition->Info.Need_Confirm);
	fprintf(stderr,"On Now          : %d\n", pCondition->Info.On_now);
	fprintf(stderr,"Priority        : %d\n", pCondition->Info.Priority);
	fprintf(stderr,"Cnt             : %d\n", pCondition->Info.Cnt);
	fprintf(stderr,"Interval        : %d\n", pCondition->Info.Interval);
	fprintf(stderr,"Condition index : %d\n", pCondition->index);
	
	fprintf(stderr,"Zone Shape      : %02X (0x01 : circle; 0x02 : rectangle;  0x03 : polygon)\n", pCondition->shape);
	
	if(pCondition->shape == _Circle){
		CirclePrintf((Circle_Struct *)pCondition->pShape_Value);
	} else if(pCondition->shape == _Rectangle){
		RectanglePrintf((Rectangle_Struct *)pCondition->pShape_Value);
	} else if(pCondition->shape == _Polygon){
		PolygonPrintf((Polygon_Struct *)pCondition->pShape_Value);
	}
	
	fprintf(stderr,"The time Set:\n");
	for(i=0;i<pCondition->TimeSet.TimeSet_Count;i++){
		fprintf(stderr,"  *Time node %d :\n",i);
		BEtimePrintf(&(pCondition->TimeSet.BE_Time[i]));
	}
}

static void PointPrintf(Point_Struct *pPoint)
{
	fprintf(stderr,"	| Status    : %d\n", pPoint->location_Status);
	fprintf(stderr,"	| Latitude  : %d\n", pPoint->Latitude);
	fprintf(stderr,"	| Longitude : %d\n", pPoint->Longitude);
	fprintf(stderr,"	| Alitude   : %d\n", pPoint->Alitude);
}

static void CirclePrintf(Circle_Struct *pCircle)
{
	fprintf(stderr,"  this is a Circle\n");
	fprintf(stderr,"  *The Center :\n");
	PointPrintf(&(pCircle->Center));
	fprintf(stderr,"  *The Radius :\n");
	fprintf(stderr,"	| %d M\n", pCircle->Radius);
}

static void RectanglePrintf(Rectangle_Struct *pRectangle)
{
	fprintf(stderr,"  this is a Rectangle\n");
	fprintf(stderr,"  *The Top left corner :\n");
	PointPrintf(&(pRectangle->Top_left_corner));
	fprintf(stderr,"  *Bottom right corner :\n");
	PointPrintf(&(pRectangle->Bottom_right_corner));
}

static void PolygonPrintf(Polygon_Struct *pPolygon)
{
	int i;
	fprintf(stderr,"  this is a Polygon\n");
	
	for(i=0;i<pPolygon->Corner_count;i++){
		fprintf(stderr,"  *The corner %d :\n",i);
		PointPrintf(&(pPolygon->Corner[i]));
	}
}

// to protect the Cond.
void InZone_Cond_lock(void)
{
    pthread_mutex_lock(&InZone_Mutex);
}

void InZone_Cond_unlock(void)
{
    pthread_mutex_unlock(&InZone_Mutex);
}

void OutZone_Cond_lock(void)
{
    pthread_mutex_lock(&OutZone_Mutex);
}

void OutZone_Cond_unlock(void)
{
    pthread_mutex_unlock(&OutZone_Mutex);
}
 
