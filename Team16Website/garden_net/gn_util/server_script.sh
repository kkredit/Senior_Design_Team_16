#!/bin/bash

lsof -i > output.txt
grep -i "python3" output.txt > python_sockets.txt

awk '{print $2}' python_sockets.txt > temp.txt

sed '1d' temp.txt > pid.txt

if [[ -s pid.txt ]] ; then
value=`cat pid.txt`
kill -SIGKILL $value 
nohup python3 /var/www/Team16Website/garden_net/gn_util/controller_interface.py
else
nohup python3 /var/www/Team16Website/garden_net/gn_util/controller_interface.py 
fi

#rm output.txt
#rm python_sockets.txt
#rm temp.txt
#rm pid.txt 
