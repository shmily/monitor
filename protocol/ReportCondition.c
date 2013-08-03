/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-12-29 09:35
#      Filename : ReportCondition.c
#   Description : not thing...
#
=============================================================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "ReportCondition.h"
#include "TLP.h"

TLP_Error ReportPacket_Handle( Packet_Struct *pPacket )
{
    int             action;
    char            *p;
	TLP_Error		res;
	
    p = (char *)(pPacket->Data + sizeof(TLP_Head_Struct));
    p++;
	action = *p++;
    action = (action << 8) + *p++;
    
    fprintf(stderr, "%s : action = 0x%04X\n",__func__, action);

    if( action == _Emergency ){

        // Emergency Report input packet handle
        res = RC_Emergency_Parser( pPacket );
    
    } else if( action == _SPEED ){

        // speed Report input packet handle
        res = RC_VelocityAnomaly_Parser( pPacket );
    
    } else if( action == _TD_SPEED ){

        // speed Report input packet handle
        res = RC_VelocityAnomaly_Parser_T( pPacket );
    
    } else if( action == _IN_ZONE ){
    
        // In zone Report input packet handle
        res = RC_InZone_Parser( pPacket );
    
    } else if( action == _OUT_ZONE ){
        
        // out zone Report input packet handle
        res = RC_OutZone_Parser( pPacket );
    
    } else if( action == _CROSSTRACK ){
        
        // cross track 
        res = RC_CrossTrack_Parser( pPacket );

    } else if( action == _Fatigue ){
        
        // Fatigue Report input packet handle
        res = RC_Fatigue_Parser( pPacket );
    
    } else {

        fprintf(stderr, "%s : unknow packet!\n",__func__);
		res = ERROR_NONE;
    }

    return res;
}

char *TLP_Head_Parser(const char *pdata, void *p_save)
{
    TLP_Head_Struct *pTlpHead;
    int             seq;
    int             *p_seq;

    p_seq = (int *)p_save;
    pTlpHead = (TLP_Head_Struct *)pdata;
    seq = pTlpHead->SEQ_num;
    seq = HTONL(seq);
    *(p_seq)  = seq;

    pdata = pdata + sizeof(TLP_Head_Struct);

    return (char *)pdata;
}

char *RC_Parse_CommInfo(const char *pdata, Report_Info *pinfo)
{
	int 	action;
	char	r_len;
	char	confirm;
	char	on_now;
	char	priority;
	int		cnt;
	int		interval;

	memset(pinfo, 0, sizeof(Report_Info));
	memset(pinfo->RID, '\0', _RID_LEN);

    pdata = TLP_Head_Parser(pdata, &pinfo->SEQ_num);    // save the SEQ num

    pdata++;                                            // skip the command

	action = (((int)*(pdata++))&0x000000FF);
	action = (action << 8) | (((int)*(pdata++))&0x000000FF);

	r_len = *(pdata++);
   	assert(r_len <= _RID_LEN);     			// we assert that the id length < _RID_LEN	
	memcpy(pinfo->RID, pdata, r_len);		// save RID
	pinfo->R_len = r_len;					// save R_len
	
	pdata = pdata + r_len;
	confirm  = *(pdata++);
	on_now   = *(pdata++);
   	priority = *(pdata++);
	
	cnt = (((int)*(pdata++))&0x000000FF);
	cnt = (cnt << 8) | (((int)*(pdata++))&0x000000FF);

	interval = (((int)*(pdata++))&0x000000FF);
	interval = (interval << 8) | (((int)*(pdata++))&0x000000FF);

	pinfo->Need_Confirm = confirm;
	pinfo->On_now       = on_now;
	pinfo->Priority     = priority;
	pinfo->Cnt          = cnt;
	pinfo->Interval     = interval;
	pinfo->Action       = action;

#ifdef __DEBUG__
	DEBUG("%s :        R_len : %d\n", __func__, r_len);
	DEBUG("%s :          RID : %s\n", __func__, pinfo->RID);
	DEBUG("%s : Need_Confirm : %d\n", __func__, confirm);
	DEBUG("%s :       On_now : %d\n", __func__, on_now);
	DEBUG("%s :     Priority : %d\n", __func__, priority);
	DEBUG("%s :          Cnt : %d\n", __func__, cnt);
	DEBUG("%s :     Interval : %d\n", __func__, interval);
#endif

	return (char *)pdata;
}

char *RC_Enpacket_DefaultAPP(char *pdata, Report_Info *pinfo)
{
	char *pdata_iner;
	int	 tmp;
	
	pdata_iner = pdata;

	*(pdata_iner++) = _TypeA_ReportCondition;		// 0x11

	tmp = pinfo->Action;
	*(pdata_iner++) = (char )(tmp >> 8);			// ACTION
	*(pdata_iner++) = (char )(tmp);

	*(pdata_iner++) = pinfo->R_len;					// R_len

	memcpy(pdata_iner, pinfo->RID, pinfo->R_len);	// RID
	pdata_iner += pinfo->R_len;

	*(pdata_iner++) = 0x01;							// ACK, 0x01 -> success!

	return pdata_iner;
}

char *RC_Enpacket_DefaultAKV(char *pdata, Report_Info *pinfo)
{
	char *pdata_iner;
	int	 tmp;

	pdata_iner = pdata;

	// --- on AKV ---
	*(pdata_iner++) = 0x00;				// attr
	*(pdata_iner++) = 0x02;				// key length
	*(pdata_iner++) = 'o';				// key
	*(pdata_iner++) = 'N';
	*(pdata_iner++) = 0x00;				// value length
	*(pdata_iner++) = 0x01;
	*(pdata_iner++) = pinfo->On_now;	// value

	// --- p AKV ---
	*(pdata_iner++) = 0x00;				// attr
	*(pdata_iner++) = 0x01;				// key length
	*(pdata_iner++) = 'p';				// key
	*(pdata_iner++) = 0x00;				// value length
	*(pdata_iner++) = 0x01;
	*(pdata_iner++) = pinfo->Priority;	// value

	// --- ts AKV ---
	*(pdata_iner++) = 0x00;				// attr
	*(pdata_iner++) = 0x02;				// key length
	*(pdata_iner++) = 't';				// key
	*(pdata_iner++) = 's';
	*(pdata_iner++) = 0x00;				// value length
	*(pdata_iner++) = 0x02;

	tmp = pinfo->Cnt;
	*(pdata_iner++) = (uint8_t)((tmp>>8)&0x00FF);
	*(pdata_iner++) = (uint8_t)((tmp) & 0x00FF);

	// --- i AKV ---
	*(pdata_iner++) = 0x00;				// attr
	*(pdata_iner++) = 0x01;				// key length
	*(pdata_iner++) = 'i';				// key
	*(pdata_iner++) = 0x00;				// value length
	*(pdata_iner++) = 0x02;

	tmp = pinfo->Interval;
	*(pdata_iner++) = (uint8_t)((tmp>>8)&0x00FF);
	*(pdata_iner++) = (uint8_t)((tmp) & 0x00FF);

	return pdata_iner;
}


char *SimpleAKV_Parser(char *pdata_start, char *pAttr, char *pK_len, int *pV_len, char **pKey, char **pValue)
{
	char  	*pdata;
	int 	tmp;
	
	pdata = pdata_start;
	
	(*pAttr)  =  *(pdata++);
	(*pK_len) =  *(pdata++);
	
	(*pKey) = pdata;				// save the key pointer
	
	pdata = pdata + (*pK_len);		// point to the V_len
	tmp = ((int)*(pdata++))&0x000000FF;
	tmp = (tmp<<8) + (((int)*(pdata++))&0x000000FF);
	(*pV_len) = tmp;
	
	(*pValue) = pdata;				// save the Value pointer

	return (char *)(pdata+tmp);
}

char *BEtime_Parser(char *pdata_start, TimeSet_Struct *pTime)
{
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

	char *pdata_end;
	int	 len;
	int	 i;

	// get value address
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x11 );
	assert( key_len   == 0x01 );
	assert( *(pkey)   == 'T'  );

	len = 0;
	pdata_start = pvalue;
	for(i=0; i<6; i++){
		pdata_end = TimeSetAKV_Parser(pdata_start, i+1, &(pTime->BE_Time[i]));

		len += (pdata_end - pdata_start);
		pdata_start = pdata_end;

		if( len >= value_len ) break;	
	}

	pTime->TimeSet_Count = i+1;
	
	return pdata_end;
}


char *TimeSetAKV_Parser(char *pdata_start, int index, TimeSlot_Struct *pTimeSlot)
{
	// for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

	// get the value address	
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x10 );
	assert( key_len   == 0x01 );
	assert( *(pkey)   == (index + 0x30));	// '1','2','3'......

	// get e time
	pdata_start = pvalue;
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x01 );
	assert( *(pkey)   == 'e'  );
	pTimeSlot->Etime.hour   = *(pvalue++);
	pTimeSlot->Etime.minute = *(pvalue++);
	pTimeSlot->Etime.second = *(pvalue++);

	// get b time
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x01 );
	assert( *(pkey)   == 'b'  );
	pTimeSlot->Btime.hour   = *(pvalue++);
	pTimeSlot->Btime.minute = *(pvalue++);
	pTimeSlot->Btime.second = *(pvalue++);

	return pdata_start;
}

char *IndexAKV_Parser(char *pdata_start, int *pindex)
{
    // for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

    int  tmp;

	// parse the index	
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 0x02 );
	assert( *(pkey++) == 'i'  );
	assert( *(pkey++) == 'x'  );
    
    tmp = 0;
    tmp = (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) + (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) + (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) + (((int)*(pvalue++))&0x000000FF);

    *pindex = tmp;

    return pdata_start;
}

char *PointAKV_Parser(char *pdata_start, const char *expect_key, Point_Struct *pPoint)
{
    // for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

    int  lon;
    int  lat;
    char expect_key_len;

    expect_key_len = strlen(expect_key);

	// get the value address, and check if this akv is the expect
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x10 );
	assert( key_len   == expect_key_len );
	assert( strncmp(pkey, expect_key, expect_key_len) == 0 );
    
    // --- attention ---
    // the point akv from the seaver has only two sub akvs, 'lon' and 'lat';
    // but not include the 't' and 'alt' akv,
    // this founction only parse the 'lon' and 'lat'.

    // parse the 'lon' akv 
	pdata_start = pvalue;
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 3    );
	assert( strncmp(pkey, "lon", 3) == 0 );
	assert( value_len == 4    );

    lon = 0;
    lon = (((int)*(pvalue++))&0x000000FF);
    lon = (lon<<8) + (((int)*(pvalue++))&0x000000FF);
    lon = (lon<<8) + (((int)*(pvalue++))&0x000000FF);
    lon = (lon<<8) + (((int)*(pvalue++))&0x000000FF);

    // parse the 'lat' akv 
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 3    );
	assert( strncmp(pkey, "lat", 3) == 0 );
	assert( value_len == 4    );

    lat = 0;
    lat = (((int)*(pvalue++))&0x000000FF);
    lat = (lat<<8) + (((int)*(pvalue++))&0x000000FF);
    lat = (lat<<8) + (((int)*(pvalue++))&0x000000FF);
    lat = (lat<<8) + (((int)*(pvalue++))&0x000000FF);
    
    pPoint->Longitude = lon;
    pPoint->Latitude  = lat;

    return pdata_start;
}

char *CircleAKV_Parser(char *pdata_start, Circle_Struct *pCircle)
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
	assert( attr      == 0x11 );
	assert( key_len   == 2    );
	assert( *(pkey++) == 'r'  );
	assert( *(pkey++) == 'S'  );

    // get the value address
	pdata_start = pvalue;
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x10 );
	assert( key_len   == 1    );
	assert( *pkey     == '1'  );

    // get the center
	pdata_start = pvalue;
    pdata_start = PointAKV_Parser(pdata_start, "c", &(pCircle->Center));
    
    // get the radius
	pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x00 );
	assert( key_len   == 1    );
	assert( *pkey     == 'r'  ); 
	assert( value_len == 4  ); 

    tmp = 0;
    tmp = (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) + (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) + (((int)*(pvalue++))&0x000000FF);
    tmp = (tmp<<8) + (((int)*(pvalue++))&0x000000FF);
    pCircle->Radius = tmp;

    return pdata_start;
}

char *PolygonAKV_Parser(char *pdata_start, Polygon_Struct *pPolygon)
{

    const char *key_array[8] = {"1","2","3","4","5","6","7","8"};

    // for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;
    
    char *pdata_end;
    int  i;
    int  value_count;

    // 'rS' AKV
    pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x11 );
	assert( key_len   == 2    );
	assert( *(pkey++) == 'r'  );
	assert( *(pkey++) == 'S'  );

	pdata_start = pvalue;
    value_count = 0;
    for(i=0; i<8; i++){
        pdata_end = PointAKV_Parser(pdata_start, key_array[i], &(pPolygon->Corner[i]));
        value_count += (pdata_end - pdata_start);
        pdata_start = pdata_end;

        if( value_count>=value_len ) break;
    }

    pPolygon->Corner_count = i + 1;

    return pdata_start;
}

char *RectangleAKV_Parser(char *pdata_start, Rectangle_Struct *pRect)
{
    // for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

    // 'rS' AKV
    pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x11 );
	assert( key_len   == 2    );
	assert( *(pkey++) == 'r'  );
	assert( *(pkey++) == 'S'  );

    // save the top_left corner
	pdata_start = pvalue;
    pdata_start = PointAKV_Parser(pdata_start, "1", &(pRect->Top_left_corner));
    
    // save the bottom_right corner
    pdata_start = PointAKV_Parser(pdata_start, "2", &(pRect->Bottom_right_corner));

    return pdata_start;
}

char *PathAKV_Parser(char *pdata_start, Line_Struct *pPath)
{
    const char *Pkey_array[16] = {"1","2","3","4","5","6","7","8","9",
                                "10","11","12","13","14","15","16"};

    // for AKV parser, no need to save
	char attr;
	char key_len;
	int  value_len;
	char *pkey;
	char *pvalue;

    char *pdata_end;
    int  i;
    int  value_count;

    // 'rS' AKV
    pdata_start = SimpleAKV_Parser(pdata_start, &attr, &key_len, &value_len, &pkey, &pvalue);
	assert( attr      == 0x12 );
	assert( key_len   == 3    );
	assert( *(pkey++) == 'r'  );
	assert( *(pkey++) == 'P'  );
	assert( *(pkey++) == 'L'  );

    pdata_start = pvalue;
    value_count = 0;
    for(i=0; i<16; i++){
        pdata_end = PointAKV_Parser(pdata_start, Pkey_array[i], &(pPath->point[i]));
        value_count += (pdata_end - pdata_start);
        pdata_start = pdata_end;

        if( value_count>=value_len ) break;
    }

    pPath->sum = i + 1;

    return pdata_start;
}
