#!/bin/sh   

ACTION=$1

if [ $ACTION == "update" ]; then
    F="xxxx.ftp"   
    echo "open 192.168.2.149"    	 > $F   
    echo "user plg plg"     		>> $F   
    echo "bin"                      >> $F   
    echo "cd /home/plg/"            >> $F   
    echo "mput $2"                  >> $F   
    echo "bye"                      >> $F   
    ftp -i -in < $F   
    rm -rf $F
elif [ $ACTION == "debug" ]; then
    F="xxxx.ftp"   
    echo "open 192.168.2.149"    	 > $F   
    echo "user plg plg"     		>> $F   
    echo "bin"                      >> $F   
    echo "cd /home/plg/"            >> $F   
    echo "get $2"                   >> $F   
    echo "bye"                      >> $F   
    ftp -i -in < $F   
    rm -rf $F   
fi
