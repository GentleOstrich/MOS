#!/bin/bash
#First you can use grep (-n) to find the number of lines of string.
#Then you can use awk to separate the answer
touch $3
touch t
grep -n $2 $1 > t
awk -F: '{print $1 >  "t"}'  t
mv t $3

