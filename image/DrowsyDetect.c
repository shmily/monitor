/*
=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2013-03-09 17:40
#      Filename : DrowsyDetect.c
#   Description : not thing...
#
=============================================================================
*/

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include "FIFO.h"
#include "DrowsyDetect.h"

// Fatigue flag
static int g_Fatigue_flag = 0;

// store the image fifo
static int 		    ImageFIFO_fd;
DrowsyImage_Info    Input_Image;

// open the fifo , so can get the image message
int Open_ImageFIFO(const char *path)
{

	/* create the fifo */
	if((mkfifo(path,O_CREAT|O_EXCL)<0)&&(errno!=EEXIST))
		fprintf(stderr, "cannot create fifoserver\n");
	fprintf(stderr, "Preparing for reading bytes...\n");

	ImageFIFO_fd = open(path,O_RDONLY|O_NONBLOCK,0);

	if( ImageFIFO_fd == -1 ){
		fprintf(stderr,"%s : > can't open the FIFO \"%s\"\n", __func__, path);
		return 0;
	} else {
		fprintf(stderr,"%s : > Open the FIFO \"%s\"\n", __func__, path);
		return 1;
	}
}

static int Check_Image(char *image_path)
{
	int  err = 0;	
	int  num_read;
	char fifo_buff[64];
	char *pstart;

	if(ImageFIFO_fd != -1){
		memset(fifo_buff, 0, 64);
		
		num_read = read(ImageFIFO_fd, fifo_buff, 64);
		
		if(num_read == -1){
			
			if(errno==EAGAIN){
				;
				//fprintf(stderr,"%s : > no data yet...\n", __func__);
			} else {
				fprintf(stderr,"%s : > unknow error...\n", __func__);
			}

		} else if(num_read>0) {
			
			pstart = strstr(fifo_buff, _IMAGE_PATH);
			if(pstart!=NULL){
				pstart = pstart + sizeof(_IMAGE_PATH);
				if(strstr(pstart, ".jpg")!=NULL){
					strcpy(image_path, fifo_buff);
					fprintf(stderr,"%s : > Get image \"%s\"\n", __func__, image_path);
					err = 1;
				}
			}
		}
	}

	return err;
}

static void Generate_Image_name(char *name)
{
	time_t 		current_time;	
	struct tm 	*Ref_Time;
	int			year_tmp;

	time(&current_time);
	Ref_Time = localtime(&current_time);
	
	year_tmp = (Ref_Time->tm_year);
	*(name++) = (year_tmp + 1900)%100;
	*(name++) = Ref_Time->tm_mon + 1;
	*(name++) = Ref_Time->tm_mday;
	*(name++) = Ref_Time->tm_hour;
	*(name++) = Ref_Time->tm_min;
	*(name++) = Ref_Time->tm_sec;
}

void *DrowsyImage_Check(void)
{
	int 	err;
	char 	*image_path;

    memset(&Input_Image, 0, sizeof(DrowsyImage_Info));

	image_path = Input_Image.image_path;

	err = Check_Image(image_path);

    if(err == 1){		// we got a new image

        g_Fatigue_flag = 1;
		Generate_Image_name(Input_Image.image_protocol_name);
        return (void *)&Input_Image;
	}

    return NULL;
}

int is_Fatigue(void)
{
    int ret;

    ret = g_Fatigue_flag;
    g_Fatigue_flag = 0;

    return ret;
}
