/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:42
#      Filename : PolyJudge.c
#   Description : not thing...
#
=============================================================================*/

#include "PolyJudge.h"

static EarthPos		P_CurPoint;
static EarthPos 	P_Corners[8];
static int			P_CornerNum = 0;

static void PosNormalize(Polygon_Struct *poly, GpsInfo *pGps)
{
	int num;
	int i;
	
	num = poly->Corner_count;
	P_CornerNum = num;
	
	for(i=0; i<num; i++){
		MSEC2Deg(poly->Corner[i].Latitude, poly->Corner[i].Longitude, &P_Corners[i]);
	}
	
	NDEG2Deg(pGps->lat, pGps->lon, &P_CurPoint);
}

int IsCrossLong180(int num, EarthPos *Corners)
{
	EarthPos *pCorners;
	double  prvLon;
	double  CurLon;
	int		i;
	
	if(num < 2){
		return -1;
	}
	
	pCorners = Corners;
	
	for(i=0; i<=num; i++){
		if(i==num){
			CurLon = pCorners->lon;
		}else{
			CurLon = (pCorners+i)->lon;
		}
		
		if(i!=0){
			if((CurLon*prvLon)<0.0){							// �����������ľ���ֵ�����෴��������϶���0�Ȼ���180�Ⱦ���
				if((fabs(prvLon) + fabs(CurLon)) >= 180.0){		// �������ֵ֮�ʹ���180����ô���ǿ�180�Ⱦ���(�����и����裺����侭�Ȳ�С��180)
					return 1;
				}
			}
		}
		
		prvLon = CurLon;
	}
	
	return 0;
}

int ZoneDetect(EarthPos *pCurPoint, int num, EarthPos *pCorners)
{
	int			i;
	int			Cross180;
	int			nCrossNum = 0;
	
	EarthPos 	CurPoint;
	
	double		CurLon;
	double 		CurLat;
	double  	NetLon;
	double		NetLat;
	double		cal_lat;
	

	CurPoint.lat = pCurPoint->lat;
	CurPoint.lon = pCurPoint->lon;
	
	Cross180 = IsCrossLong180(num, pCorners);

	if((Cross180 == 1)&&(CurPoint.lon < 0)){
		CurPoint.lon += 360;
	}

	for(i=0; i<num; i++){
		CurLon = (pCorners+i)->lon;
		CurLat = (pCorners+i)->lat;
		
		if(i != num-1){					// ȡ����һ���㣬���һ�߶�
			NetLon = (pCorners+i+1)->lon;
			NetLat = (pCorners+i+1)->lat;
		}else{							// ���һ�����һ������߶�
			NetLon = pCorners->lon;
			NetLat = pCorners->lat;
		}
		
		if(Cross180 == 1){				// �������180�Ⱦ��ߣ�������ֵ��Ҫת��Ϊ������ֵ  
			if(CurLon < 0){
				CurLon += 360;
			}
			
			if(NetLon < 0){
				NetLon += 360;
			}
		}
		
		if((fabs(CurLon - CurPoint.lon)<0.000001)&&
		   (fabs(CurLat - CurPoint.lat)<0.000001)){		// �����ǰλ�õ���Ƕ��㣬����Ϊ����������
			return 1;
		}
		
		if((fabs(NetLon - CurPoint.lon)<0.000001)&&
		   (fabs(NetLat - CurPoint.lat)<0.000001)){		// �����ǰλ�õ���Ƕ��㣬����Ϊ����������
			return 1;
		}
		
		//�����������һ�����γ����90~������γ�ȼ䣬������γ������  
		if(((CurLat < 90)&&(CurLat > CurPoint.lat))||
		   ((NetLat < 90)&&(NetLat > CurPoint.lat))){
			
			// �����ж��Ƿ�����γ������
			if(((CurPoint.lon < CurLon)&&(CurPoint.lon > NetLon))||
			   ((CurPoint.lon > CurLon)&&(CurPoint.lon < NetLon))){
				
				// ���㽻���γ��ֵ
				cal_lat = CurLat - (CurLat - NetLat)*(CurLon - CurPoint.lon)/(CurLon - NetLon);
				
				// ���γ��ֵ���Ǵ�����γ��ֵ���������ڶ���α����ϣ���Ϊ�ڶ������
				if(fabs(cal_lat - CurPoint.lat) < 0.000001){
					return 1;
				}
				
				//���γ��ֵ���ڴ���γ��ֵ�����������߶����ཻ�߶Σ�������1
				if(cal_lat > CurPoint.lat){
					nCrossNum++;
				}	
			}
		}	
	}

	if((nCrossNum%2) == 1){
		return 1;
	}
	
	return 0;

}


int IsPosIn_Poly(Polygon_Struct *poly, GpsInfo *pGps)
{
	
	PosNormalize(poly, pGps);
	return ZoneDetect(&P_CurPoint, P_CornerNum, P_Corners);

}
