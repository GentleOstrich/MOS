#!/bin/bash
mkdir gen
a=1
while [$a -le 10]
do
	touch ./gen/$a.txt
	a=$[$a+1]
done
mkdir mydir
chmod +rwx mydir
touch myfile
echo 2023 > myfile
mv moveme ./mydir
cp copyme ./mydir
mv ./mydir/copyme ./mydir/copied
cat ./readme
gcc bad.c 2> err.txt




