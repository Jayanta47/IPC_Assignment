#!/bin/bash
n_pass=10
output_file="output4.txt"

if [[ -d "./passengers" ]]; then
    rm -rf "./passengers"
fi 

mkdir "./passengers"

for i in $(seq 1 $n_pass); do
    grep -i "passenger $i " $output_file > "./passengers/($i)passenger.txt" 
done