#!/bin/bash

echo -n "time> begin "; date 

python costMatrix.py > info.txt; 
cat info.txt

echo -n "time> finished all the tests "; date

