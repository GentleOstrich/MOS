#!/bin/bash
mkdir mydir
chmod +rwx mydir
touch myfile
echo 2023 > myfile
mv moveme ./mydir
cp copyme ./mydir
mv ./mydir/copyme ./mydir/copied
cat ./readme
gcc bad.c 2> err.txt




