/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:00
#      Filename : ReportCondition.h
#   Description : not thing...
#
=============================================================================*/

#ifndef		_REPORT_CONDITION_H_
#define		_REPORT_CONDITION_H_

#include "PositionUpdate.h"
#include "myList.h"
#include "TLP.h"

// the max zone support
#define		_Max_Zone		8

// the max id length
#define		_RID_LEN		32

// action define
#define		_Emergency		0x0001
#define		_SPEED		    0x0007
#define		_Fatigue		0x0014
#define		_OUT_ZONE		0x0023
#define		_IN_ZONE		0x0022
#define 	_TD_SPEED		0x0008
#define 	_CROSSTRACK	    0x0018

// define the zone shape
#define		_Circle 		0x01
#define		_Rectangle  	0x02
#define		_Polygon 		0x03

// define the ack return poniter
typedef struct _ACK_Data_
{
	uint8_t		*pStart;
	uint8_t		*pEnd;
	uint8_t		ACK;
	void		*pCondition;
}ACK_DataPointer_Struct;


// point && shape defind
typedef struct _Point_
{
	uint8_t		location_Status;
	uint32_t	Latitude;
	uint32_t	Longitude;
	uint16_t	Alitude;
}Point_Struct;


typedef struct _Circle_Struct_
{
	Point_Struct 	Center;
	uint32_t	 	Radius;
}Circle_Struct;

typedef struct _Rectangle_Struct_
{
	Point_Struct 	Top_left_corner;
	Point_Struct	Bottom_right_corner;
}Rectangle_Struct;

typedef struct _Polygon_Struct_
{
	uint8_t			Corner_count;
	Point_Struct 	Corner[8];
}Polygon_Struct;


// time defined
typedef struct _Time_slot_
{
	Time_D		Btime;
	Time_D		Etime;
}TimeSlot_Struct;

typedef struct _Time_Set_
{
	uint8_t			TimeSet_Count;
	TimeSlot_Struct	BE_Time[6];
}TimeSet_Struct;


// about report condition
typedef struct _Report_Info_
{
	uint8_t		RID[_RID_LEN];
	uint8_t		R_len;
	uint8_t		Need_Confirm;
	uint8_t		On_now;
	uint8_t		Priority;
	uint16_t	Cnt;
	uint16_t	Interval;
	uint16_t	Action;
    int         SEQ_num;
}Report_Info;

typedef struct _Fatigue_Condition_
{
    Report_Info         Info;
    int                 max_driving_time;
    int                 min_break_time;
}Fatigue_Condition;

typedef struct ZONE_Condition_
{
	Report_Info 		Info;
	uint32_t			index;
	uint8_t				shape;			// 0x01 : circle; 0x02 : rectangle;  0x03 : polygon
	void				*pShape_Value;
	TimeSet_Struct		TimeSet;
	struct list_head 	list;
}Zone_Condition;

typedef struct _SpeedAnomaly_Condition_
{
	Report_Info 		Info;
	uint8_t				type;			// 0x01 : above; 0x02 : under; 0x03 : across;
	uint8_t				speed;			// unit : km/h
	uint16_t			interval;
	struct list_head 	list;
}SpeedAnomaly_Condition;

typedef struct _SpeedAnomaly_Condition_T
{
	Report_Info 		Info;
	uint32_t			index;
	uint8_t				speed;			// km/h
	TimeSet_Struct		TimeSet;
	uint16_t			interval;
	struct list_head 	list;
}SpeedAnomaly_Condition_T;

typedef struct _Line_Struct
{
    int                 sum;
    Point_Struct        point[16];
}Line_Struct;

typedef struct _Crosstrack_Condition
{
    Report_Info         Info;
    int                 index;
    int                 error;
    TimeSet_Struct      TimeSet;
    Line_Struct         path;
    struct list_head    list;
}CrossTrack_Condition;

char *RC_Parse_CommInfo(const char *pdata, Report_Info *pinfo);
char *RC_Enpacket_DefaultAKV(char *pdata, Report_Info *pinfo);
char *RC_Enpacket_DefaultAPP(char *pdata, Report_Info *pinfo);

char *SimpleAKV_Parser( char *pdata_start, 
				        char *pAttr, 
						char *pK_len, 
						int  *pV_len, 
						char **pKey, 
						char **pValue );

char *TimeSetAKV_Parser(char *pdata_start, int index, TimeSlot_Struct *pTimeSlot);
char *BEtime_Parser(char *pdata_start, TimeSet_Struct *pTime);
char *IndexAKV_Parser(char *pdata_start, int *pindex);
char *PointAKV_Parser(char *pdata_start, const char *expect_key, Point_Struct *pPoint);
char *CircleAKV_Parser(char *pdata_start, Circle_Struct *pCircle);
char *PolygonAKV_Parser(char *pdata_start, Polygon_Struct *pPolygon);
char *RectangleAKV_Parser(char *pdata_start, Rectangle_Struct *pRect);
char *PathAKV_Parser(char *pdata_start, Line_Struct *pPath);


char      *TLP_Head_Parser(const char *pdata, void *p_save);
TLP_Error RC_Emergency_Parser( Packet_Struct *pPacket );
TLP_Error RC_Fatigue_Parser( Packet_Struct *pPacket );
TLP_Error RC_VelocityAnomaly_Parser( Packet_Struct *pPacket );
TLP_Error RC_VelocityAnomaly_Parser_T( Packet_Struct *pPacket );
TLP_Error RC_InZone_Parser( Packet_Struct *pPacket );
TLP_Error RC_InZone_ACK( Zone_Condition * pcondition );
TLP_Error RC_OutZone_Parser( Packet_Struct *pPacket );
TLP_Error RC_OutZone_ACK( Zone_Condition * pcondition );

TLP_Error RC_CrossTrack_Parser( Packet_Struct *pPacket );

TLP_Error ReportPacket_Handle( Packet_Struct *pPacket );

void ShowCondition_Zone      (struct list_head *phead);
void ShowConditons_Speed     (struct list_head *phead);
void ShowConditons_Speed_T   (struct list_head *phead);
void ShowConditons_CrossTrack(struct list_head *phead);
void BEtimePrintf(TimeSlot_Struct *pTimeSlot);

// 2013.02.21 add for report update
// Mutex for condition protect
void Speed_Cond_lock(void);
void Speed_Cond_unlock(void);

void InZone_Cond_lock(void);
void InZone_Cond_unlock(void);
void OutZone_Cond_lock(void);
void OutZone_Cond_unlock(void);

void CrossTrack_Cond_lock(void);
void CrossTrack_Cond_unlock(void);

void Get_Speed_Cond(SpeedAnomaly_Condition *pcond);

void Copy_Emergency_Info(Report_Info *pInfo);
int isNeed_Update_Emergency(void);
int Get_Emergency_SEQ_num(void);

#endif
