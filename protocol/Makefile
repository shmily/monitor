INCLUDE_FLAGS = -I. \
				-I./include \
				-I../GSM \
				-I../uart \
				-I../include \
				-I../network \
				-I../task \
				-I../math \
				-I../GPS/nmea \
				-I../image/include \
				-I../param \
				-I../judge/include

CFLAGS	      = $(INCLUDE_FLAGS) -Wall -g

ifeq ($(arch),)
CC			  = arm-linux-gcc
AR			  = arm-linux-ar
endif

AFLAGS		  = -r

TARGET = libprotocol.a

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
