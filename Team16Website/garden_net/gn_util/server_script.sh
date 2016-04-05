#!/bin/bash
# My first script

lsof -i > output.txt
grep -i "python3" output.txt > python_sockets.txt

awk '{print $2}' python_sockets.txt > temp.txt

sed '1d' temp.txt > pid.txt

value=`cat pid.txt`

echo "$value"

kill -SIGKILL $value

python3 /var/www/Team16Website/garden_net/gn_util/controller_interface.py

rm output.txt
rm python_sockets.txt
rm temp.txt
rm pid.txt 
