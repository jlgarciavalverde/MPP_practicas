#!/bin/bash

make sec

in_file="../input/igen/in.txt"
# in_file_distances="../input/input"
in_file_distances="../input/input_1000_400_200_100.txt"
out_file="../output/output.txt"

while read line ; do 

	if [[ $line != \#* ]]
	then

	n=$(echo $line | tr -s ' ' | cut -f1 -d ' ')
	m=$(echo $line | tr -s ' ' | cut -f2 -d ' ')
	gen=$(echo $line | tr -s ' ' | cut -f3 -d ' ')
	tam=$(echo $line | tr -s ' ' | cut -f4 -d ' ')
	m_rate=$(echo $line | tr -s ' ' | cut -f5 -d ' ')
	threshold_percent=$(echo $line | tr -s ' ' | cut -f6 -d ' ')
	window_size=$(echo $line | tr -s ' ' | cut -f7 -d ' ')
	
	echo -e
	echo -n "Executing with: "
	echo -e "N = "$n" M = "$m" N_GEN = "$gen" TAM_POB = "$tam" M_RATE = "$m_rate" THRESHOLD = "$threshold_percent" WINDOW = "$window_size
	./sec $n $m $gen $tam $m_rate $threshold_percent $window_size < "${in_file_distances}"
	fi
done < $in_file

