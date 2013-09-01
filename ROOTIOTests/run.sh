#!/bin/bash

#python sleeper.py

echo -n "time> begin "; date 

echo "=o= compiling read.C"
make --debug=b
echo "=============== done"

ls

python FDR.py > info.txt; 
cat info.txt

echo -n "time> finished all the tests "; date

