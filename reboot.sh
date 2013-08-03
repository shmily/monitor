#!/bin/sh

# check a process
# if the process is exist, return 0
# else, return 1

CheckProcess()
{
  # check the input param
  if [ "$1" = "" ];
  then
    return 1
  fi
 
  # $PROCESS_NUM return the num of then process
  PROCESS_NUM=`ps | grep "$1" | grep -v "grep" | wc -l`
  if [ $PROCESS_NUM -eq 1 ];
  then
    return 0
  else
    return 1
  fi
}


while [ 1 ] ; do
 CheckProcess "GSM-Test"
 CheckQQ_RET=$?
 if [ $CheckQQ_RET -eq 1 ];
 then
  killall -9 GSM-Test
  exec ./GSM-Test
 fi
 sleep 20
done

