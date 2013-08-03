INCLUDE_FLAGS = -I./include \
				-I../include \
				-I../uart \
				-I../GSM \
				-I../math \
				-I../GPS/nmea \
				-I../protocol/include \
				-I../network \
				-I../task
 
CFLAGS	      = $(INCLUDE_FLAGS) -Wall -g


ifeq ($(arch),)
CC			  = arm-linux-gcc
AR			  = arm-linux-ar
endif

AFLAGS		  = -r

TARGET = libjudge.a

SOUCE_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOUCE_FILES))

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(AR) $(AFLAGS) $(TARGET) $(OBJS)

.PHONY:clean
clean:
	$(RM) $(TARGET) $(OBJS)
