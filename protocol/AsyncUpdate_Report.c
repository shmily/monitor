/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-01-17 21:52
#      Filename : AsyncUpdate_Report.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "PositionUpdate.h"
#include "ReportCondition.h"
#include "AsyncUpdate.h"
#include "earth_math.h"


char *Enpacket_RID_AKV(char *pbuff, const char *tid)
{
    int tid_len;

    tid_len = strlen(tid);

    *(pbuff++) = 0x00;      // attr
    *(pbuff++) = 0x03;      // key length
    *(pbuff++) = 'r';       // key
    *(pbuff++) = 'i';
    *(pbuff++) = 'd';
    *(pbuff++) = (char)(tid_len >> 8);
    *(pbuff++) = (char)(tid_len);
    
    memcpy(pbuff, tid, tid_len);

    return (pbuff + tid_len);
}

char *Enpacket_Rname_AKV(char *pbuff, int rname)
{
    *(pbuff++) = 0x00;      // attr
    *(pbuff++) = 0x05;      // key length
    *(pbuff++) = 'r';       // key
    *(pbuff++) = 'n';
    *(pbuff++) = 'a';
    *(pbuff++) = 'm';
    *(pbuff++) = 'e';
    *(pbuff++) = 0x00;
    *(pbuff++) = 0x02;
    *(pbuff++) = (char)(rname >> 8);
    *(pbuff++) = (char)(rname);

    return pbuff;
}
