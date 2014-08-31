#!/bin/bash

N=0
for i in `seq 1 1000`
do
  T=`./main`
  ((N+=T))
done
((N/=1000))
echo $N
