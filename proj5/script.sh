#!/bin/bash
#Jeremy Chan jsc126
#bash script that pings textfile with hostnames, outputs results to csv
#txt file columns: host, min, avg, max, mdev 
filename="$1"
while read -r line; do
	name="$line"
	stat=$(ping $name -c 10 | sed -n '15p' | awk '{ print $4 }')
	echo $name $stat | tr "/" " "
done < "$filename"
