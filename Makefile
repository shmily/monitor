INCLUDE_FLAGS = -I. \
				-I./buzzer\
				-I./uart \
				-I./GSM \
				-I./include \
				-I./protocol/include \
				-I./task \
				-I./param \
				-I./GPS/nmea \
				-I./network \
				-I./judge/include \
				-I./image/include

CFLAGS	      = $(INCLUDE_FLAGS) -Wall -g

ifeq ($(arch),x86)
	CC	  		  = gcc
	AR			  = ar
	iniparserlib  = -liniparser-x86
else 
	CC	  		  = arm-linux-gcc
	AR			  = arm-linux-ar
	STRIP		  = arm-linux-strip
	iniparserlib  = -liniparser
endif

LFLAGS 		  = -L./image -limage \
				-L./judge -ljudge \
				-L./protocol -lprotocol \
				-L./network -lnetwork \
				-L./GSM -lgsm \
				-L./math -lmath \
				-L./GPS -lgps \
				-L./uart -luart \
				-L./task -ltask \
				-L./param -lparam $(iniparserlib) \
				-L./md5 -lmd5 \
				-L./buzzer -lbuzzer \
				-lm \
				-lrt \
				-lpthread

SHELL 		 = /bin/bash
SUBDIRS 	 = buzzer uart GSM task GPS param network protocol judge image md5 math

TARGET = GSM-Test

export CC
export AR

SOUCE_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOUCE_FILES))

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS) libs
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(LFLAGS)

libs:
	@ for subdir in $(SUBDIRS); do \
        (cd $$subdir && $(MAKE)); \
	done

.PHONY:clean update debug strip tags

strip: $(TARGET)
	$(STRIP) $(TARGET)	

update:
	./Update.sh update $(TARGET)

debug:
	./Update.sh debug debug.txt

tags:
	ctags -R *

clean:
	$(RM) $(TARGET) $(OBJS)
	@ for subdir in $(SUBDIRS); do \
        (cd $$subdir && $(MAKE) clean); \
    done
