#!/bin/bash

sudo echo "Password" 

if [ $( id -u ) -eq 0 ]; then
echo "Running as SuperUser , going for full performance"

governor="performance"
for CPUFREQ in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
do
[ -f $CPUFREQ ] || continue
echo -n $governor > $CPUFREQ
done

  sudo nice -n -20 ionice -c 1 -n 0  time ./MatrixTypeConversion KIx30.txt KData30b.txt Out.txt&
  exit 0

fi

 
echo "Running as normal User"
time ./MatrixTypeConversion KIx30.txt KData30b.txt Out.txt& 

exit 0
