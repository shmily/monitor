/*
===============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-05 19:28
#      Filename : distance.c
#   Description : reference : http://www.movable-type.co.uk/scripts/latlong.html
#
===============================================================================
*/

#include <stdio.h>
#include <math.h>

#define PI                      3.1415926
#define EARTH_RADIUS            6378.137        //地球近似半径

double radian(double d);
double get_distance(double lat1, double lng1, double lat2, double lng2);

// 求弧度
double radian(double d)
{
    return d * PI / 180.0;   //角度1˚ = π / 180
}

//计算距离
double get_distance(double lat1, double lng1, double lat2, double lng2)
{
    double radLat1 = radian(lat1);
    double radLat2 = radian(lat2);
    double a = radLat1 - radLat2;
    double b = radian(lng1) - radian(lng2);
      
    double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2) )));
      
    dst = dst * EARTH_RADIUS; 
    
    dst= round(dst * 10000) / 10000;

    return dst;
}


double get_bearing(double lat1, double lng1, double lat2, double lng2)
{

    double x, y;
    double bearing;

    lat1 = radian(lat1);
    lng1 = radian(lng1);

    lat2 = radian(lat2);
    lng2 = radian(lng2);

    y = sin(lng2-lng1)*cos(lat2);
    x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(lng2-lng1);

    bearing = atan2(y, x);

    /* for deg output */
   /*
    bearing = (bearing * 180.0)/PI;
    if( bearing < 0.0 ) bearing += 360.0;
    */

    return bearing;
}

double get_CrossTrack(double lat1, double lng1, double lat2, double lng2, double lat3, double lng3)
{
    double d13;
    double a13, a12;
    double dxt;

    d13 = get_distance(lat1, lng1, lat3, lng3);
    a13 = get_bearing (lat1, lng1, lat3, lng3); 
    a12 = get_bearing (lat1, lng1, lat2, lng2); 

    /*
    printf("d13 = %0.3fkm\n", d13);  //dst = 9.281km
    printf("a13 = %0.3f\n",   a13);  //dst = 9.281km
    printf("a12 = %0.3f\n",   a12);  //dst = 9.281km
    */

    dxt = asin( sin(d13/EARTH_RADIUS) * sin(a13-a12) ) * EARTH_RADIUS;

    return dxt;
}

int test_main (int argc, const char * argv[])
{
    double lat1 = 39.90744;
    double lng1 = 116.41615;    //经度,纬度1
    double lat2 = 39.90744;
    double lng2 = 116.30746;    //经度,纬度2

    double lat3 = 39.90844;
    double lng3 = 116.41615;    //经度,纬度1

    // insert code here...

    double dst = get_distance(lat1, lng1, lat2, lng2);
    double ber = get_bearing(lat1, lng1, lat2, lng2);
    double dxt = get_CrossTrack(lat1, lng1, lat2, lng2, lat3, lng3);

    printf("distance = %0.3fkm\n", dst);  //dst = 9.281km
    
    printf("bearing  = %0.3f\n", ber);  //dst = 9.281km
    printf("dxt      = %0.3f\n", dxt);  //dst = 9.281km

    return 0;
}
