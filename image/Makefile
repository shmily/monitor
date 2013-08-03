INCLUDE_FLAGS = -I. \
				-I./include/ \
				-I../include \
				-I../uart \
				-I../GSM \
				-I../protocol/include \
				-I../network \
				-I../task \
				-I../md5
 
CFLAGS	      = $(INCLUDE_FLAGS) -Wall -g


ifeq ($(arch),)
CC			  = arm-linux-gcc
AR			  = arm-linux-ar
endif

AFLAGS		  = -r

TARGET = libimage.a

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
